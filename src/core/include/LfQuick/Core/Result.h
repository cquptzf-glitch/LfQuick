
#pragma once

#include <LfQuick/Core/Error.h>

#include <optional>
#include <utility>

namespace LfQuick {

template <typename T>
class Result
{
public:
    static Result success(T value) { return Result(std::move(value)); }
    static Result failure(Error error) { return Result(std::move(error)); }

    bool hasValue() const noexcept { return m_value.has_value(); }
    explicit operator bool() const noexcept { return hasValue(); }

    const T &value() const { return m_value.value(); }
    T &value() { return m_value.value(); }
    const Error &error() const noexcept { return m_error; }

private:
    explicit Result(T value) : m_value(std::move(value)) {}
    explicit Result(Error error) : m_error(std::move(error)) {}

    std::optional<T> m_value;
    Error m_error;
};

template <>
class Result<void>
{
public:
    static Result success() { return Result(true, {}); }
    static Result failure(Error error) { return Result(false, std::move(error)); }

    bool hasValue() const noexcept { return m_ok; }
    explicit operator bool() const noexcept { return m_ok; }
    const Error &error() const noexcept { return m_error; }

private:
    Result(bool ok, Error error) : m_ok(ok), m_error(std::move(error)) {}
    bool m_ok = false;
    Error m_error;
};

} // namespace LfQuick
