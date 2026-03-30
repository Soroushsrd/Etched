/**
 * @file Result.h
 * @brief Declaration of Result class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-24
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

// Structured error type — carries phase, location, and message.
// Line/column default to 0 when not applicable (e.g. layout phase).
struct Error {
    std::string phase;
    std::string message;
    size_t line = 0;
    size_t column = 0;

    std::string format() const {
        if (line != 0) {
            return "[" + phase + "] " + message + " (line " +
                   std::to_string(line) + ", col " + std::to_string(column) +
                   ")";
        }
        return "[" + phase + "] " + message;
    }
};

// -----------------------------------------------------------------
// Result<T>
//
// Holds either a T (success) or an Error (failure).
// Intentionally kept simple: no monadic chaining yet, that can be
// layered on once every pipeline stage is migrated.
//
// Usage:
//   Result<Foo> r = Result<Foo>::ok(myFoo);
//   Result<Foo> r = Result<Foo>::err({"parser", "unexpected token", 3, 7});
//
//   if (!r) { std::cerr << r.error().format(); }
//   Foo& f = r.value();          // throws if error
//   Foo  f = std::move(r).take();// move out the value
// -----------------------------------------------------------------
template <typename T> class Result {
  private:
    Result(std::variant<T, Error> d) : data(std::move(d)) {}

    std::variant<T, Error> data;

  public:
    // Error-forwarding constructor: enables TRY/TRY_VOID macros.
    // WARNING: Only valid when `other` is in error state.
    template <typename U>
    Result(const Result<U> &other) : data(other.error()) {}

    static Result ok(T val) {

        return Result(
            std::variant<T, Error>(std::in_place_type<T>, std::move(val)));
    }

    static Result err(Error err) {
        return Result(
            std::variant<T, Error>(std::in_place_type<Error>, std::move(err)));
    }

    static Result err(std::string phase, std::string msg, size_t line = 0,
                      size_t column = 0) {
        return err(Error{std::move(phase), std::move(msg), line, column});
    }

    bool isOk() const { return std::holds_alternative<T>(data); }
    bool isErr() const { return std::holds_alternative<Error>(data); }
    // allows: if (!result) {...}
    explicit operator bool() const { return isOk(); }

    T &value() & {
        if (isErr())
            throw std::logic_error("Result::value() called on Error: " +
                                   std::get<Error>(data).format());
        return std::get<T>(data);
    }

    const T &value() const & {
        if (isErr())
            throw std::logic_error("Result::value() called on Error: " +
                                   std::get<Error>(data).format());
        return std::get<T>(data);
    }

    // move the value out if it exists
    T take() && {
        if (isErr())
            throw std::logic_error("Result::take() called on Error: " +
                                   std::get<Error>(data).format());
        return std::move(std::get<T>(data));
    }

    const Error &error() const {
        if (isOk())
            throw std::logic_error("Result::error() called on Ok value");
        return std::get<Error>(data);
    }
    // print the error to stderr and return false or silently return true
    bool check() const {
        if (isErr()) {
            std::cerr << std::get<Error>(data).format() << "\n";
            return false;
        }
        return true;
    }
};

// -----------------------------------------------------------------
// Result<void>
//
// Specialization for functions that succeed with no value but can
// still fail. No value(), take(), or ok(T) , those don't apply.
// -----------------------------------------------------------------
template <> class Result<void> {
  private:
    Result() = default;
    // bool is a cheap stand-in for the empty ok state
    // we only ever store true here, the Error arm carries all meaning
    std::variant<bool, Error> data;

  public:
    // Error-forwarding constructor: enables TRY/TRY_VOID macros.
    // WARNING: Only valid when `other` is in error state.
    template <typename U>
    Result(const Result<U> &other) : data(other.error()) {}

    static Result ok() {
        Result r;
        r.data = true;
        return r;
    }
    static Result err(Error e) {
        Result r;
        r.data = std::move(e);
        return r;
    }
    static Result err(std::string phase, std::string msg, size_t line = 0,
                      size_t column = 0) {
        return err(Error{std::move(phase), std::move(msg), line, column});
    }

    bool isOk() const { return std::holds_alternative<bool>(data); }
    bool isErr() const { return std::holds_alternative<Error>(data); }

    explicit operator bool() const { return isOk(); }

    const Error &error() const {
        if (isOk())
            throw std::logic_error("Result::error() called on Ok value");
        return std::get<Error>(data);
    }
    bool check() const {
        if (isErr()) {
            std::cerr << std::get<Error>(data).format() << "\n";
            return false;
        }
        return true;
    }
};

using VoidResult = Result<void>;

#define TRY(dest, expr)                                                        \
    auto _tmp_##dest = (expr);                                                 \
    if (!_tmp_##dest)                                                          \
        return _tmp_##dest;                                                    \
    auto &dest = _tmp_##dest.value();

#define TRY_VOID(expr)                                                         \
    if (auto _tmp = (expr); !_tmp)                                             \
        return _tmp;
