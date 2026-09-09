// FILENAME: basicpp.hpp
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: C++17 Host Applications, Game Engines, Desktop & Embedded Applications
// Modern C++17 RAII Header Wrapper for BASIC++ v6.5.2.

#ifndef BASICPP_HPP
#define BASICPP_HPP

#include "basicpp.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <optional>

namespace bpp {

//
// ---- Exception Class ----
//

class Exception : public std::runtime_error {
public:
    explicit Exception(int error_code, const std::string &message)
        : std::runtime_error(message + " (Error " + std::to_string(error_code) + ")"),
          code_(error_code) {}

    int code() const noexcept { return code_; }

private:
    int code_;
};

//
// ---- Value Wrapper ----
//

class Value {
public:
    Value() : type_(BPP_VAL_NULL), number_(0.0) {}
    explicit Value(double num) : type_(BPP_VAL_NUMBER), number_(num) {}
    explicit Value(int num) : type_(BPP_VAL_INTEGER), number_(num) {}
    explicit Value(const std::string &str) : type_(BPP_VAL_STRING), str_(str), number_(0.0) {}
    explicit Value(const char *str) : type_(BPP_VAL_STRING), str_(str ? str : ""), number_(0.0) {}

    bool is_null() const noexcept { return type_ == BPP_VAL_NULL; }
    bool is_number() const noexcept { return type_ == BPP_VAL_NUMBER || type_ == BPP_VAL_INTEGER; }
    bool is_string() const noexcept { return type_ == BPP_VAL_STRING; }

    double as_number() const {
        if (is_number()) return number_;
        if (is_string()) {
            try { return std::stod(str_); } catch (...) { return 0.0; }
        }
        return 0.0;
    }

    const std::string &as_string() const noexcept { return str_; }

    BppValueType type() const noexcept { return type_; }

private:
    BppValueType type_;
    std::string str_;
    double number_;
};

//
// ---- Engine Class ----
//

class Engine {
public:
    using BusCallback = std::function<void(std::string_view topic, std::string_view payload)>;
    using LogCallback = std::function<void(BppLogLevel level, std::string_view tag, std::string_view msg, std::string_view timestamp)>;

    explicit Engine(size_t ram_mb = 64) {
        ctx_ = bpp_init(ram_mb * 1024 * 1024);
        if (!ctx_) {
            throw Exception(1, "Failed to initialize BASIC++ virtual machine context.");
        }
    }

    ~Engine() {
        if (ctx_) {
            bpp_shutdown(ctx_);
            ctx_ = nullptr;
        }
    }

    // Non-copyable, movable
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
    Engine(Engine &&other) noexcept : ctx_(other.ctx_) { other.ctx_ = nullptr; }
    Engine &operator=(Engine &&other) noexcept {
        if (this != &other) {
            if (ctx_) bpp_shutdown(ctx_);
            ctx_ = other.ctx_;
            other.ctx_ = nullptr;
        }
        return *this;
    }

    void reset() {
        if (ctx_) bpp_reset(ctx_);
    }

    std::string_view version() const noexcept {
        return bpp_version();
    }

    // Statement Execution
    void exec(std::string_view code) {
        std::string null_term(code);
        int res = bpp_exec_string(ctx_, null_term.c_str());
        if (res != 0) {
            throw Exception(res, "Execution error on: " + null_term);
        }
    }

    void run_file(const std::string &filepath) {
        int res = bpp_exec_file(ctx_, filepath.c_str());
        if (res != 0) {
            throw Exception(res, "Failed executing program file: " + filepath);
        }
    }

    // Expression Evaluation
    Value eval(std::string_view expr) {
        std::string null_term(expr);
        BppValue val = bpp_eval_expr(ctx_, null_term.c_str());
        try {
            if (val.type == BPP_VAL_NUMBER || val.type == BPP_VAL_INTEGER) {
                double n = val.as.number;
                bpp_value_free(ctx_, &val);
                return Value(n);
            } else if (val.type == BPP_VAL_STRING) {
                std::string s = val.as.string ? val.as.string : "";
                bpp_value_free(ctx_, &val);
                return Value(s);
            } else if (val.type == BPP_VAL_ERROR) {
                int code = val.as.error_code;
                bpp_value_free(ctx_, &val);
                throw Exception(code, "Error evaluating expression: " + null_term);
            }
            bpp_value_free(ctx_, &val);
            return Value();
        } catch (...) {
            bpp_value_free(ctx_, &val);
            throw;
        }
    }

    // Variable Access
    void set(const std::string &name, double val) {
        bpp_set_number(ctx_, name.c_str(), val);
    }

    void set(const std::string &name, const std::string &val) {
        bpp_set_string(ctx_, name.c_str(), val.c_str());
    }

    double get_number(const std::string &name) const {
        return bpp_get_number(ctx_, name.c_str());
    }

    std::string get_string(const std::string &name) const {
        const char *s = bpp_get_string(ctx_, name.c_str());
        return s ? std::string(s) : std::string();
    }

    // Zero-Copy Vector / Array Binding
    void bind_array(const std::string &name, std::vector<double> &vec) {
        int res = bpp_array_bind_view(ctx_, name.c_str(), 1, vec.data(), vec.size());
        if (res != 0) {
            throw Exception(res, "Failed to bind array view: " + name);
        }
    }

    // Message Bus Pub/Sub
    void publish(std::string_view topic, std::string_view payload) {
        std::string t(topic);
        bpp_bus_publish(ctx_, t.c_str(), payload.data(), payload.size());
    }

    void subscribe(const std::string &topic, BusCallback cb) {
        bus_callbacks_.push_back(std::move(cb));
        auto *cb_ptr = &bus_callbacks_.back();
        bpp_bus_subscribe(ctx_, topic.c_str(), [](const char *top, const char *pay, size_t len, void *udata) {
            auto *fn = static_cast<BusCallback *>(udata);
            if (fn && *fn) {
                (*fn)(std::string_view(top ? top : ""), std::string_view(pay ? pay : "", len));
            }
        }, cb_ptr);
    }

    // Logging Sink Redirection
    void set_log_sink(LogCallback cb, BppLogLevel min_level = BPP_LOG_INFO) {
        log_callback_ = std::move(cb);
        bpp_log_add_sink(ctx_, [](BppLogLevel level, const char *tag, const char *msg, const char *ts, void *udata) {
            auto *self = static_cast<Engine *>(udata);
            if (self && self->log_callback_) {
                self->log_callback_(level, tag ? tag : "", msg ? msg : "", ts ? ts : "");
            }
        }, static_cast<int>(min_level), this);
    }

    BppEngineContext *native_handle() const noexcept { return ctx_; }

private:
    BppEngineContext *ctx_{nullptr};
    std::vector<BusCallback> bus_callbacks_;
    LogCallback log_callback_;
};

} // namespace bpp

#endif // BASICPP_HPP
