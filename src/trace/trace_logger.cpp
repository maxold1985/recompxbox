#include "recompxbox/trace_logger.h"
#include <iomanip>
#include <sstream>

namespace recompxbox {
TraceLogger::TraceLogger(const std::string& path)
    : stream_(path, std::ios::out | std::ios::trunc) {}
bool TraceLogger::good() const { return stream_.good(); }
std::string TraceLogger::escape(const char* input) {
    std::string out;
    if (!input) return out;
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(input); *p; ++p) {
        const unsigned char c = *p;
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    out += "\\u00";
                    out += hex[c >> 4];
                    out += hex[c & 15];
                } else out += static_cast<char>(c);
        }
    }
    return out;
}
void TraceLogger::start(const char* kind) {
    stream_ << "{\"seq\":" << sequence_++ << ",\"type\":\"" << kind << "\"";
}
void TraceLogger::finish() { stream_ << "}\n"; stream_.flush(); }
void TraceLogger::cpu(std::uint32_t eip, std::uint32_t eax, std::uint32_t esp) {
    start("cpu");
    stream_ << ",\"eip\":" << eip << ",\"eax\":" << eax << ",\"esp\":" << esp;
    finish();
}
void TraceLogger::mmio(const char* operation, std::uint32_t eip,
                       std::uint32_t address, std::uint64_t value, unsigned width) {
    start("mmio");
    stream_ << ",\"op\":\"" << escape(operation) << "\""
            << ",\"eip\":" << eip << ",\"address\":" << address
            << ",\"value\":" << value << ",\"width\":" << width;
    finish();
}
void TraceLogger::kernel(std::uint32_t eip, std::uint32_t ordinal, const char* name) {
    start("kernel");
    stream_ << ",\"eip\":" << eip << ",\"ordinal\":" << ordinal
            << ",\"name\":\"" << escape(name) << "\"";
    finish();
}
void TraceLogger::interrupt(std::uint32_t eip, std::uint32_t vector) {
    start("interrupt");
    stream_ << ",\"eip\":" << eip << ",\"vector\":" << vector;
    finish();
}
void TraceLogger::message(const char* subsystem, const char* message) {
    start("message");
    stream_ << ",\"subsystem\":\"" << escape(subsystem)
            << "\",\"message\":\"" << escape(message) << "\"";
    finish();
}
}
