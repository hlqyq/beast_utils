// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/utils.h"
#include <iomanip>
#include <mutex>
#include <thread>
#include <cstring>

//////////////////////////////////////// misc ////////////////////////////////////////

const std::chrono::system_clock::time_point now(void) {
    return std::chrono::system_clock::now();
}

const std::string to_string_impl(const std::chrono::system_clock::time_point& time, const char* output_format) {
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    tm timeinfo;
# ifdef _WIN32
    localtime_s(&timeinfo, &tt);
# else
    localtime_r(&tt, &timeinfo);
# endif
    std::ostringstream os;
    os << std::put_time(&timeinfo, output_format);
    return os.str();
}

const std::string to_string(const std::chrono::system_clock::time_point& time) {
    return to_string_impl(time, "%Y-%m-%d %H:%M:%S");
}

const std::string to_time_string(const std::chrono::system_clock::time_point& time) {
    return to_string_impl(time, "%H:%M:%S");
}

const std::string to_date_string(const std::chrono::system_clock::time_point& time) {
    return to_string_impl(time, "%Y-%m-%d");
}

const std::string now_to_string(void) {
    return to_string(now());
}

const std::string now_to_time_string(void) {
    return to_time_string(now());
}

const std::string now_to_date_string(void) {
    return to_date_string(now());
}

//////////////////////////////////////// logging ////////////////////////////////////////

void default_console_output_handle(int severity_level, const char* message, uint32_t message_content_offset) {
    fprintf(stderr, "%s\n", message);
    fflush(stderr);
}

int& log_reporting_level(void) {
    return log_message::reporting_level();
}

void set_log_handle(log_handler_type log_handler, uintptr_t user_data, bool multi_threaded) {
    if (log_handler) {
        if (multi_threaded) {
            static std::mutex k_log_mutex;
            std::mutex* p_mutex = &k_log_mutex;
            log_message::output_handler() = [log_handler, user_data, p_mutex](int severity_level, const char* message, uint32_t content_offset) {
                std::unique_lock<std::mutex> lock(*p_mutex);
                log_handler(user_data, severity_level, message, content_offset);
            };

        } else {
            log_message::output_handler() = [log_handler, user_data](int severity_level, const char* message, uint32_t message_content_offset) {
                log_handler(user_data, severity_level, message, message_content_offset);
            };
        }
    } else {
        log_message::output_handler() = default_console_output_handle;
    }
}

log_message::log_message(const char* file_name, int line_number, int severity_level) : severity_level_(severity_level), message_offset_(0) {
    stream_ << '[' << now_to_time_string() << ' ';

    const char* const k_log_severity_names[LOG_NUM_SEVERITIES] = { "INFO", "WARNING", "ERROR", "ERROR REPORT", "FATAL ERROR" };
    if ((severity_level >= 0) && (severity_level < LOG_NUM_SEVERITIES)) {
        stream_ << k_log_severity_names[severity_level];
    } else if (severity_level == LOG_DEBUG) {
        stream_ << "DEBUG";
    } else if (severity_level < LOG_DEBUG) {
        stream_ << "VERBOSE_" << -severity_level;
    } else {
        stream_ << "LOG_" << severity_level;
    }

    stream_ << ':' << std::this_thread::get_id();

    if (file_name && (*file_name != '\0')) {
        const char* psz1 = strrchr(file_name, '\\');
        const char* psz2 = strrchr(file_name, '/');
        const char* psz = psz1 < psz2 ? psz2 : psz1;
        stream_ << ' ' << (psz ? psz + 1 : file_name) << ':' << line_number;
    }

    stream_ << "] ";
    message_offset_ = static_cast<unsigned int>(stream_.tellp());
}

log_message::~log_message(void) {
    if (output_handler())
        (output_handler())(severity_level_, stream_.str().c_str(), message_offset_);
}

int& log_message::reporting_level(void) {
    static int k_reporting_level = LOG_VERBOSE;
    return k_reporting_level;
}

log_message::output_handler_type& log_message::output_handler(void) {
    static output_handler_type k_output_handler = default_console_output_handle;
    return k_output_handler;
}

instance_log::instance_log(const char* function_name, void* instance_this, int log_severity_level_up) : function_name_(function_name),
                           instance_this_(instance_this), log_severity_level_(log_severity_level_up + LOG_VERBOSE) {
    LOG_EX(log_severity_level_) << function_name_ << "(" << instance_this_ << ") constructed.";
}

instance_log::~instance_log(void) {
    auto destruct_function_name(function_name_);
    auto pos = destruct_function_name.find("::");
    if (pos != std::string::npos)
        destruct_function_name = destruct_function_name.replace(pos, 2, "::~");
    LOG_EX(log_severity_level_) << destruct_function_name << "(" << instance_this_ << ") destructed.";
}

scope_guard make_function_scope_log(const char* function_name, int log_severity_level_up) {
    LOG_EX(LOG_VERBOSE + log_severity_level_up) << function_name << "(...) entering...";
    return make_scope_guard([function_name, log_severity_level_up]() {
        LOG_EX(LOG_VERBOSE + log_severity_level_up) << function_name << "(...) exited.";
    });
}

//////////////////////////////////////// scope_guard ////////////////////////////////////////

scope_guard::scope_guard(execution policy) : policy_(policy) {
}

scope_guard::scope_guard(scope_guard&& other) : handlers_(std::move(other.handlers_)), policy_(other.policy_) {
    other.handlers_.clear();
}

scope_guard::~scope_guard(void) {
    commit();
}

void scope_guard::commit(void) noexcept {
    if (policy_ == always || (std::uncaught_exception() == (policy_ == exception))) {
        for (auto& f : handlers_) try {
            f();  // must not throw
        }
        catch (...) {
        }
    }
    dismiss();
}

void scope_guard::dismiss(void) noexcept {
    handlers_.clear();
}

scope_guard make_scope_guard(void) {
    return scope_guard();
}
