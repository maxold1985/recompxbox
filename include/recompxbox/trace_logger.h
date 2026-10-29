#pragma once
#include <cstdint>
#include <fstream>
#include <string>

namespace recompxbox {
// JSON Lines trace format: one independently parseable object per event.
// Caller owns synchronization if multiple threads emit events.
class TraceLogger {
public:
    explicit TraceLogger(const std::string& path);
    bool good() const;
    void cpu(std::uint32_t eip, std::uint32_t eax, std::uint32_t esp);
    void mmio(const char* operation, std::uint32_t eip, std::uint32_t address,
              std::uint64_t value, unsigned width);
    void kernel(std::uint32_t eip, std::uint32_t ordinal, const char* name);
    void interrupt(std::uint32_t eip, std::uint32_t vector);
    void message(const char* subsystem, const char* message);
private:
    std::ofstream stream_;
    std::uint64_t sequence_ = 0;
    void start(const char* kind);
    void finish();
    static std::string escape(const char* value);
};
}
