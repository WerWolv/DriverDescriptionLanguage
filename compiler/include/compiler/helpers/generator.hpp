#pragma once

#include <coroutine>
#include <exception>

namespace compiler::hlp {

    template <typename T>
    struct Generator {
        struct promise_type;
        using handle_type = std::coroutine_handle<promise_type>;

        struct promise_type {
            promise_type() = default;

            T m_value;
            std::exception_ptr m_exception;

            auto get_return_object() -> Generator {
                return Generator(handle_type::from_promise(*this));
            }

            auto initial_suspend() -> std::suspend_always { return { }; }
            auto final_suspend() noexcept -> std::suspend_always { return { }; }
            auto unhandled_exception() { m_exception = std::current_exception(); }

            template<std::convertible_to<T> From>
            auto yield_value(From&& from) -> std::suspend_always{
                m_value = std::forward<From>(from);
                return { };
            }

            auto return_void() { }
        };

        handle_type m_handle;

        explicit Generator(handle_type handle) : m_handle(handle) { }

        ~Generator() { m_handle.destroy(); }
        explicit operator bool() {
            fill();
            return !m_handle.done();
        }

        auto operator()() -> T {
            fill();
            m_full = false;

            return std::move(m_handle.promise().m_value);
        }

    private:
        bool m_full = false;

        auto fill() {
            if (!m_full) {
                m_handle();
                if (m_handle.promise().m_exception)
                    std::rethrow_exception(m_handle.promise().m_exception);

                m_full = true;
            }
        }
    };

}