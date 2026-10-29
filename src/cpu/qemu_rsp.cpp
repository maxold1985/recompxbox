#include "recompxbox/qemu_rsp.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif
namespace recompxbox {
namespace {
#ifdef _WIN32
using Sock = SOCKET;
constexpr Sock bad = INVALID_SOCKET;
void shut(Sock s) { closesocket(s); }
#else
using Sock = int;
constexpr Sock bad = -1;
void shut(Sock s) { close(s); }
#endif
bool sendAll(Sock s, const std::string& b) {
    size_t p = 0;
    while (p < b.size()) {
        int n = send(s, b.data() + p, static_cast<int>(std::min<size_t>(4096, b.size()-p)), 0);
        if (n <= 0) return false;
        p += static_cast<size_t>(n);
    }
    return true;
}
bool byte(Sock s, char& c) { return recv(s, &c, 1, 0) == 1; }
bool rsp(Sock s, const std::string& cmd, std::string& answer) {
    unsigned sum = 0;
    for (unsigned char c : cmd) sum = (sum + c) & 255;
    char suffix[4];
    std::snprintf(suffix, sizeof(suffix), "#%02x", sum);
    if (!sendAll(s, "$" + cmd + suffix)) return false;
    char c;
    if (!byte(s,c) || c != '+') return false;
    do { if (!byte(s,c)) return false; } while (c != '$');
    answer.clear();
    while (true) {
        if (!byte(s,c)) return false;
        if (c == '#') break;
        if (answer.size() >= 8192) return false;
        answer.push_back(c);
    }
    char a,b;
    if (!byte(s,a) || !byte(s,b)) return false;
    unsigned check = 0;
    for (unsigned char x : answer) check = (check+x)&255;
    char expected[3];
    std::snprintf(expected,sizeof(expected),"%02x",check);
    if (a != expected[0] || b != expected[1]) {
        sendAll(s,"-");
        return false;
    }
    return sendAll(s,"+");
}
bool write(Sock s, uint64_t addr, const uint8_t* bytes, size_t len, size_t chunkSize, std::string& error) {
    constexpr char hex[]="0123456789abcdef";
    for (size_t off=0; off<len; off+=chunkSize) {
        size_t n=std::min(chunkSize,len-off);
        char label[64];
        std::snprintf(label,sizeof(label),"M%llx,%zx:",
                      static_cast<unsigned long long>(addr+off),n);
        std::string cmd(label);
        for (size_t i=0;i<n;++i) {
            auto v=bytes[off+i];
            cmd.push_back(hex[v>>4]);
            cmd.push_back(hex[v&15]);
        }
        std::string reply;
        if (!rsp(s,cmd,reply)) {
            error="GDB RSP transport/checksum failure at address "+std::to_string(addr+off);
            return false;
        }
        if (reply!="OK") {
            error="QEMU rejected memory write at "+std::to_string(addr+off)+
                  " (reply: "+reply+")";
            return false;
        }
        if ((off / chunkSize) % 32 == 0 || off + n == len) {
            std::cout << "  TCP progress: " << (off + n) << "/" << len
                      << " bytes\n" << std::flush;
        }
    }
    return true;
}
}
bool sendXbeToQemu(const XbeImage& image, const std::string& endpoint, std::string& error) {
    const auto sep=endpoint.rfind(':');
    if (sep==std::string::npos || sep==0 || sep+1==endpoint.size() ||
        image.memory.size()!=image.header.imageSize) {
        error="Invalid host:port or XBE image"; return false;
    }
    const auto host=endpoint.substr(0,sep), port=endpoint.substr(sep+1);
    for (char c:port) if (c<'0'||c>'9') {error="Invalid port";return false;}
    unsigned long number=0;
    try {number=std::stoul(port);} catch (...) {error="Invalid port";return false;}
    if (number==0||number>65535) {error="Invalid port";return false;}
#ifdef _WIN32
    WSADATA ws{};
    if (WSAStartup(MAKEWORD(2,2),&ws)!=0) {error="Winsock startup failed";return false;}
#endif
    std::cout << "QEMU TCP: resolving " << endpoint << "\n" << std::flush;
    addrinfo hints{};
    hints.ai_socktype=SOCK_STREAM;
    addrinfo* addresses=nullptr;
    Sock sock=bad;
    if (getaddrinfo(host.c_str(),port.c_str(),&hints,&addresses)==0) {
        for (auto* p=addresses;p;p=p->ai_next) {
            sock=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
            if (sock==bad) continue;
#ifdef _WIN32
            DWORD timeout=5000;
            setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
            setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
#else
            timeval timeout{5,0};
            setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
            setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
#endif
            std::cout << "QEMU TCP: connecting...\n" << std::flush;
            if (connect(sock,p->ai_addr,static_cast<int>(p->ai_addrlen))==0) break;
            shut(sock);
            sock=bad;
        }
        freeaddrinfo(addresses);
    }
    bool ok=sock!=bad;
    if (!ok) error="Cannot connect to QEMU TCP";
    std::string reply;
    if (ok) std::cout << "QEMU TCP: connected, negotiating GDB RSP...\n" << std::flush;
    if (ok && !(ok=rsp(sock,"qSupported:multiprocess+",reply)))
        error="QEMU GDB handshake failed";
    if (ok) std::cout << "QEMU RSP: " << reply << "\n" << std::flush;
    size_t chunkSize = 1024;
    if (ok) {
        // PacketSize is a hexadecimal GDB RSP packet payload limit.
        const auto marker = reply.find("PacketSize=");
        if (marker != std::string::npos) {
            const auto begin = marker + 11;
            const auto end = reply.find(';', begin);
            try {
                const auto limit = std::stoul(reply.substr(begin, end - begin), nullptr, 16);
                // M packet encodes each byte as two hex characters.
                // Reserve 64 characters for the address and length prefix.
                if (limit >= 256 && limit <= 1048576)
                    chunkSize = std::min<size_t>(16384, (limit - 64) / 2);
            } catch (...) {
                // Fall back to the conservative 1024-byte writes.
            }
        }
        std::cout << "QEMU TCP: negotiated write chunk = " << chunkSize
                  << " bytes\n" << std::flush;
    }
    if (ok) std::cout << "QEMU TCP: writing XBE header...\n" << std::flush;
    if (ok) ok=write(sock,image.header.baseAddress,image.memory.data(),
                     image.header.headerSize,chunkSize,error);
    if (ok) for (const auto& section:image.sections) {
        std::cout << "QEMU TCP: section VA=0x" << std::hex << section.virtualAddress
                  << std::dec << " size=" << section.virtualSize << "\n" << std::flush;
        const size_t off=section.virtualAddress-image.header.baseAddress;
        if (!(ok=write(sock,section.virtualAddress,image.memory.data()+off,
                       section.virtualSize,chunkSize,error))) break;
    }
    if (sock!=bad) shut(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return ok;
}
}
