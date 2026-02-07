#ifndef UNICONV_CORE_OUTPUT_SPINNER_H
#define UNICONV_CORE_OUTPUT_SPINNER_H

#include <atomic>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

namespace uniconv::core::output {

class Spinner {
public:
    explicit Spinner(std::ostream& os) : os_(os) {}

    ~Spinner() { stop(); }

    Spinner(const Spinner&) = delete;
    Spinner& operator=(const Spinner&) = delete;

    void start(const std::string& message) {
        start([message]() { return message; });
    }

    void start(std::function<std::string()> message_fn) {
        stop();
        message_fn_ = std::move(message_fn);
        running_.store(true);
        thread_ = std::thread([this]() {
            static const char* frames[] = {
                "\xe2\xa0\x8b", "\xe2\xa0\x99", "\xe2\xa0\xb9", "\xe2\xa0\xb8",
                "\xe2\xa0\xbc", "\xe2\xa0\xb4", "\xe2\xa0\xa6", "\xe2\xa0\xa7",
                "\xe2\xa0\x87", "\xe2\xa0\x8f"
            };
            size_t i = 0;
            while (running_.load()) {
                os_ << "\r\033[K" << frames[i % 10] << " " << message_fn_() << std::flush;
                i++;
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
            }
        });
    }

    void stop() {
        if (running_.exchange(false)) {
            if (thread_.joinable()) {
                thread_.join();
            }
            // Clear the spinner line
            os_ << "\r\033[K" << std::flush;
        }
    }

private:
    std::ostream& os_;
    std::function<std::string()> message_fn_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};

} // namespace uniconv::core::output

#endif // UNICONV_CORE_OUTPUT_SPINNER_H
