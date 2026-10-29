#include "recompxbox/trace_logger.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

int main() {
    const char* path = "trace_logger_test.jsonl";
    {
        recompxbox::TraceLogger log(path);
        assert(log.good());
        log.cpu(0x10000, 42, 0x20000);
        log.mmio("write", 0x10002, 0xfd000100, 1, 4);
        log.kernel(0x10003, 7, "Kernel\"Test");
        log.interrupt(0x10004, 3);
        log.message("test", "line\nbreak");
    }
    std::ifstream file(path);
    assert(file.good());
    const std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    assert(content.find("\"type\":\"cpu\"") != std::string::npos);
    assert(content.find("\"type\":\"mmio\"") != std::string::npos);
    assert(content.find("\"type\":\"kernel\"") != std::string::npos);
    assert(content.find("Kernel\\\"Test") != std::string::npos);
    assert(content.find("\"type\":\"interrupt\"") != std::string::npos);
    assert(content.find("line\\nbreak") != std::string::npos);
    file.close();
    std::remove(path);
}
