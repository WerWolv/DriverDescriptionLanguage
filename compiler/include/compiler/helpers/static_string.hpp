#pragma once

#include <array>
#include <cstdint>

namespace compiler::hlp {

    template<size_t N>
    struct StaticString {
        constexpr StaticString(const char (&str)[N]) noexcept {
            std::copy(str, str + N, string.begin());
        }

        constexpr operator std::string_view() const noexcept {
            return std::string_view(string.data(), size());
        }

        [[nodiscard]] constexpr auto size() const noexcept -> size_t {
            return string.size() - 1;
        }

        std::array<char, N> string;
    };
}