#pragma once

#include <compiler/helpers/generator.hpp>

#include <expected>
#include <string_view>

#include <fmt/format.h>

namespace compiler::language::lexer {

    class Token {
    public:
        enum class Type {
            Identifier,
            Keyword,
            BuiltinType,
            Operator,
            StringLiteral,
            CharacterLiteral,
            NumericLiteral,
            RawCodeBlock,
            Separator,
            Comment,
            EndOfInput
        };

        constexpr Token() = default;
        constexpr Token(Type type, std::string_view value) : m_type(type), m_value(value) { }

        [[nodiscard]] auto type() const -> Type { return m_type; }
        [[nodiscard]] auto value() const -> std::string_view { return m_value; }

    private:
        Type m_type = Type::EndOfInput;
        std::string_view m_value;
    };

    struct LexedData {
        Token  token;
        size_t length = 0;
    };

    enum class LexError {
        UnterminatedStringLiteral,
        UnterminatedComment,
        InvalidCharacter,
        InvalidNumericLiteral,
        UnknownToken
    };

    constexpr static inline auto KeywordDriver              = Token(Token::Type::Keyword, "driver");
    constexpr static inline auto KeywordFunction            = Token(Token::Type::Keyword, "fn");

    constexpr static inline auto RawCodeBlock               = Token(Token::Type::RawCodeBlock, "");

    constexpr static inline auto BuiltinType                = Token(Token::Type::BuiltinType, "");

    constexpr static inline auto SeparatorOpenBrace         = Token(Token::Type::Separator, "{");
    constexpr static inline auto SeparatorCloseBrace        = Token(Token::Type::Separator, "}");
    constexpr static inline auto SeparatorOpenParenthesis   = Token(Token::Type::Separator, "(");
    constexpr static inline auto SeparatorCloseParenthesis  = Token(Token::Type::Separator, ")");
    constexpr static inline auto SeparatorSemicolon         = Token(Token::Type::Separator, ";");
    constexpr static inline auto SeparatorComma             = Token(Token::Type::Separator, ",");
    constexpr static inline auto SeparatorColon             = Token(Token::Type::Separator, ":");
    constexpr static inline auto SeparatorDot               = Token(Token::Type::Separator, ".");

    constexpr static inline auto Identifier                 = Token(Token::Type::Identifier, "");

    using LexResult = std::expected<LexedData, LexError>;
    using TokenGenerator = hlp::Generator<std::expected<Token, LexError>>;

    auto lex(std::string_view &source) -> TokenGenerator;

}

template <> struct fmt::formatter<compiler::language::lexer::LexError>: formatter<std::string_view> {
    template <typename FormatContext>
    auto format(compiler::language::lexer::LexError error, FormatContext& ctx) const {
        string_view name = "unknown";

        switch (error) {
            using enum compiler::language::lexer::LexError;
            case UnterminatedStringLiteral: name = "unterminated string literal";   break;
            case UnterminatedComment:       name = "unterminated comment";          break;
            case InvalidCharacter:          name = "invalid character";             break;
            case InvalidNumericLiteral:     name = "invalid numeric literal";       break;
            case UnknownToken:              name = "unknown token";                 break;
        }

        return formatter<string_view>::format(name, ctx);
    }
};