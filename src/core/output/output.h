#ifndef UNICONV_CORE_OUTPUT_OUTPUT_H
#define UNICONV_CORE_OUTPUT_OUTPUT_H

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

    // Progress (future-ready, no-op for now)
    virtual void progress(std::string_view task, int percent) = 0;

    // State
    virtual bool is_verbose() const = 0;
    virtual bool is_quiet() const = 0;
    virtual void flush() = 0;
};

} // namespace uniconv::core::output

#endif // UNICONV_CORE_OUTPUT_OUTPUT_H
