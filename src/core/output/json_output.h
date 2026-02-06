#ifndef UNICONV_CORE_OUTPUT_JSON_OUTPUT_H
#define UNICONV_CORE_OUTPUT_JSON_OUTPUT_H

#include "output.h"

#include <iostream>

namespace uniconv::core::output {

class JsonOutput : public IOutput {
public:
    JsonOutput(std::ostream& out, std::ostream& err,
               bool verbose = false, bool quiet = false);

    void error(std::string_view msg) override;
    void warning(std::string_view msg) override;
    void info(std::string_view msg) override;
    void success(std::string_view msg) override;
    void debug(std::string_view msg) override;

    void data(const nlohmann::json& data,
              const std::string& text_format = "") override;

    void help(std::string_view text) override;

    void stage_started(size_t current, size_t total, const std::string& target) override;
    void stage_completed(size_t current, size_t total, const std::string& target,
                         int64_t duration_ms, bool success, const std::string& error = "") override;

    bool is_verbose() const override;
    bool is_quiet() const override;
    void flush() override;

private:
    void output_message(std::ostream& os, const std::string& type,
                        std::string_view msg);

    std::ostream& out_;
    std::ostream& err_;
    bool verbose_;
    bool quiet_;
};

} // namespace uniconv::core::output

#endif // UNICONV_CORE_OUTPUT_JSON_OUTPUT_H
