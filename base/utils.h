// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//
// Logging:
//
//      set_log_reporting_level(LOG_DEBUG);
//      set_log_handle([](uintptr_t user_data, int severity_level, const char* message, uint32_t content_offset) {
//          std::cout << message << std::endl; }, 0, false);  //optional
//      LOG(INFO) << "Hello world!" << std::endl;

//
// Function scope log:
//
//      void my_function(...) {
//          FUNCTION_SCOPE_LOG;
//      }
//
// Output:
//      my_function(...) entering...
//      my_function(...) exited.
//

//
// Instance scope log:
//
//      class MyObject
//      {
//      public:
//          MyObject(void) : INSTANCE_LOG_IMPL {}
//
//      private:
//          INSTANCE_LOG_DECLARE;
//      };
//
// Output:
//      MyObject::MyObject(00000265DA043DC0) constructed.
//      MyObject::MyObject(00000265DA043DC0) destructed.
//

//
// reference: https://stackoverflow.com/questions/10270328/the-simplest-and-neatest-c11-scopeguard
//
// Usage 1:
//
//      scope_guard scope_exit, scope_fail(scope_guard::execution::exception);
//
//      action1();
//      scope_exit += []() { cleanup1(); };
//      scope_fail += []() { rollback1(); };
//
//      action2();
//      scope_exit += []() { cleanup2(); };
//      scope_fail += []() { rollback2(); };
//
// Usage 2:
//
//      auto _ = make_scope_guard([]() { std::cout << "b" << std:endl; });
//
// Usage 3:
//
//      ON_SCOPE_EXIT(std::cout << "b" << std:endl);
//
// Usage 4:
//
//      auto scope_exit = make_scope_guard([]() { std::cout << "b" << std:endl; });
//      scope_exit.commit()
//

//
// Stream to a string:
//
//   std::string result_string;
//   STRING_STREAM(result_string) << "Hello" << " world!";   // result_string == "Hello world!"
//
//
// String conversion:
//
//   std::string str_age = to_string(18);  // str_age == "18"
//   std::string str_result = string_join(" ", "Hello", "world");  // str_result == "Hello world"
//

#ifndef BASE_UTILS_H_
#define BASE_UTILS_H_

#include <utility>
#include <string>
#include <functional>
#include <sstream>
#include <deque>
#include <chrono>

//////////////////////////////////////// scope_guard declarations ////////////////////////////////////////

class scope_guard;
scope_guard make_scope_guard(void);
template<class Callable> scope_guard make_scope_guard(Callable&& func);
template<class EnterCallable, class ExitCallable> scope_guard make_scope_guard(EnterCallable&& enter_func, ExitCallable&& exit_func);
#define ON_SCOPE_EXIT(code) auto STRING_JOIN(scope_guard, __LINE__) = make_scope_guard([&](){ code; })

//////////////////////////////////////// logging declarations ////////////////////////////////////////

enum { LOG_VERBOSE = -1, LOG_DEBUG = LOG_VERBOSE, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_ERROR_REPORT, LOG_FATAL, LOG_NUM_SEVERITIES };

#define LOG(severity) \
if (LOG_ ## severity < log_message::reporting_level()); \
else log_message(__FILE__, __LINE__, LOG_ ## severity).stream()

#define LOG_EX(severity) \
if (severity < log_message::reporting_level()); \
else log_message(__FILE__, __LINE__, severity).stream()

#define FUNCTION_SCOPE_LOG auto STRING_JOIN(scope_guard, __LINE__) = make_function_scope_log(__FUNCTION__, 0)

#define INSTANCE_LOG_DECLARE                           std::unique_ptr<instance_log>  sp_instance_log_
#define INSTANCE_LOG_IMPL                              sp_instance_log_(std::make_unique<instance_log>(__FUNCTION__, this, 0))
#define INSTANCE_LOG_EX(severity_inc)                   sp_instance_log_(std::make_unique<instance_log>(__FUNCTION__, this, severity_inc))

int& log_reporting_level(void);
typedef void (*log_handler_type)(uintptr_t user_data, int severity_level, const char* message, uint32_t message_content_offset);
void set_log_handle(log_handler_type log_handler, uintptr_t user_data, bool multi_threaded);

scope_guard make_function_scope_log(const char* function_name, int log_severity_level_up);

//////////////////////////////////////// misc declarations ////////////////////////////////////////

#define STRING_STREAM(result_string) string_stream(result_string).stream()
template <class ...T> const std::string string_join(const std::string seperator, T... args);
const std::chrono::system_clock::time_point now(void);
template <class T> const std::string to_string(T t);
const std::string to_string(const std::chrono::system_clock::time_point& time);
const std::string to_time_string(const std::chrono::system_clock::time_point& time);
const std::string to_date_string(const std::chrono::system_clock::time_point& time);
const std::string now_to_string(void);
const std::string now_to_time_string(void);
const std::string now_to_date_string(void);

//////////////////////////////////////// logging implements ////////////////////////////////////////

class log_message {
 public:
    typedef log_message                                                                                     this_type;
    typedef std::function<void(int severity_level, const char* message, uint32_t message_content_offset)>   output_handler_type;

 public:
    log_message(const char* file_name, int line_number, int severity_level);
    ~log_message(void);

 private:
    explicit log_message(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

 public:
    static int& reporting_level(void);
    static output_handler_type& output_handler(void);
    std::ostream& stream(void) { return stream_; }

 private:
    int                     severity_level_;
    std::ostringstream      stream_;
    unsigned int            message_offset_;
};

class instance_log {
 public:
    typedef instance_log                        this_type;

 public:
    explicit instance_log(const char* function_name, void* instance_this, int log_severity_level_up);
    ~instance_log(void);
    explicit instance_log(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

 private:
    std::string     function_name_;
    void*           instance_this_;
    int             log_severity_level_;
};

//////////////////////////////////////// scope_guard implements ////////////////////////////////////////

class scope_guard {
 public:
    typedef scope_guard                                                 this_type;
    typedef std::function<void()>                                       handler_type;
    typedef std::deque<handler_type>                                    handler_container_type;
    enum execution { always, no_exception, exception };

 public:
    explicit scope_guard(execution policy = always);
    scope_guard(scope_guard&& other);
    template<class Callable> scope_guard(Callable&& func, execution policy = always);
    ~scope_guard(void);

 private:
    scope_guard(const scope_guard&) = delete;
    void operator=(const scope_guard&) = delete;

 public:
    template<class Callable> scope_guard& operator += (Callable&& func);
    void commit(void) noexcept;
    void dismiss(void) noexcept;

 private:
    execution               policy_ = always;
    handler_container_type  handlers_;
};

template<class Callable>
inline scope_guard::scope_guard(Callable&& func, execution policy) : policy_(policy) {
    this->operator += <Callable>(std::forward<Callable>(func));
}

template<class Callable>
inline scope_guard& scope_guard::operator += (Callable&& func) try {
    handlers_.emplace_front(std::forward<Callable>(func));
    return *this;
}
catch (...) {
    if (policy_ != no_exception)
        func();
    throw;
}

template<class Callable>
inline scope_guard make_scope_guard(Callable&& func) {
    return scope_guard(func);
}

template<class EnterCallable, class ExitCallable>
inline scope_guard make_scope_guard(EnterCallable&& enter_func, ExitCallable&& exit_func) {
    if (enter_func)
        enter_func();
    return scope_guard(exit_func);
}

#define STRING_JOIN(arg1, arg2) STRING_JOIN2(arg1, arg2)
#define STRING_JOIN2(arg1, arg2) arg1 ## arg2

//////////////////////////////////////// misc implements ////////////////////////////////////////

class string_stream {
 public:
    typedef string_stream                        this_type;

 public:
    explicit string_stream(std::string& result_string) : result_string_(result_string) {}
    ~string_stream(void) { result_string_ = stream_.str(); }
    explicit string_stream(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

 public:
    std::ostream& stream(void) { return stream_; }

 private:
    std::string&            result_string_;
    std::ostringstream      stream_;
};

template <class T>
inline void stream_out(std::ostream& os, const std::string& seperator, T t) {
    os << t;
}

template <class T, class ...Args>
inline void stream_out(std::ostream& os, const std::string& seperator, T head, Args... rest) {
    os << head << seperator;
    stream_out(os, seperator, rest...);
}

template <class ...T>
inline const std::string string_join(const std::string seperator, T... args) {
    std::string result;
    stream_out(STRING_STREAM(result), seperator, args...);
    return result;
}

template <class T>
inline const std::string to_string(T t) {
    std::ostringstream os;
    os << t;
    return os.str();
}

#endif  // BASE_UTILS_H_
