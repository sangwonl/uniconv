#ifndef UNICONV_CORE_OUTPUT_OUTPUT_H
#define UNICONV_CORE_OUTPUT_OUTPUT_H

#include <cstdint>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

namespace uniconv::core::output {

class IOutput {
public:
    virtual ~IOutput() = default;

    // Messages
    virtual void error(std::string_view msg) = 0;
    virtual void warning(std::string_view msg) = 0;
    virtual void info(std::string_view msg) = 0;
    virtual void success(std::string_view msg) = 0;
    virtual void debug(std::string_view msg) = 0;

    // Structured data (table in text, object in JSON)
    virtual void data(const nlohmann::json& data,
                      const std::string& text_format = "") = 0;

    // Help text (always plain, even in JSON mode)
    virtual void help(std::string_view text) = 0;

    // Pipeline stage progress
    virtual void stage_started(size_t current, size_t total, const std::string& target) = 0;
    virtual void stage_completed(size_t current, size_t total, const std::string& target,
                                 int64_t duration_ms, bool success, const std::string& error = "") = 0;

    // State
    virtual bool is_verbose() const = 0;
    virtual bool is_quiet() const = 0;
    virtual void flush() = 0;
};

} // namespace uniconv::core::output

#endif // UNICONV_CORE_OUTPUT_OUTPUT_H
