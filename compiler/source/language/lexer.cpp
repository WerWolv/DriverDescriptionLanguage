#include <compiler/language/lexer.hpp>

#include <tuple>
#include <optional>
#include <stdexcept>

#include <compiler/helpers/static_string.hpp>
#include <wolv/utils/string.hpp>

namespace compiler::language::lexer {

    template<hlp::StaticString Value>
    using LexKeyword = decltype(
        [](std::string_view &source) -> std::optional<LexResult> {
            // Check if the source starts with the keyword
            if (source.starts_with(Value)) {

                // Make sure the keyword is doesn't have any other alphanumerical characters following it
                if (source.size() == Value.size() || !std::isalnum(source[Value.size()])) {
                    return LexedData { Token(Token::Type::Keyword, Value), Value.size() };
                }

            }

            return std::nullopt;
        }
    );

    template<hlp::StaticString Value>
    using LexBuiltinType = decltype(
        [](std::string_view &source) -> std::optional<LexResult> {
            // Check if the source starts with the keyword
            if (source.starts_with(Value)) {

                // Make sure the keyword is doesn't have any other alphanumerical characters following it
                if (source.size() == Value.size() || !std::isalnum(source[Value.size()])) {
                    return LexedData { Token(Token::Type::BuiltinType, Value), Value.size() };
                }

            }

            return std::nullopt;
        }
    );

    using LexWhitespace = decltype(
        [](std::string_view &source) -> std::optional<LexResult> {

            // Remove all whitespace characters from the source
            while (!source.empty() && std::isspace(source.front())) {
                source.remove_prefix(1);
            }

            return std::nullopt;
        }
    );

    using LexEndOfFile = decltype(
        [](std::string_view &source) -> std::optional<LexResult> {

            // Check if the source is empty. If it is, we reached the end of the input
            if (source.empty()) {
                return LexedData { Token(Token::Type::EndOfInput, ""), 0 };
            }

            return std::nullopt;
        }
    );

    using LexIdentifier = decltype(
        [](std::string_view &source) -> std::optional<LexResult> {

            // Check if the source starts with an alphabetical character
            if (std::isalpha(source.front())) {

                // Get the length of the identifier. An identifier is a sequence of alphanumeric characters starting with an alphabetical character
                size_t length = 1;
                while (length < source.size() && std::isalnum(source[length])) {
                    length++;
                }

                // Extract the entire identifier from the source
                auto value = source.substr(0, length);

                return LexedData { Token(Token::Type::Identifier, value), length };
            }

            return std::nullopt;
        }
    );

    using LexNumericLiteral = decltype(
        [](std::string_view &source) -> std::optional<LexResult> {

            if (source.starts_with("0x")) {
                // Hexadecimal literal
                size_t length = 2;
                while (length < source.size() && std::isxdigit(source[length])) {
                    length++;
                }

                return LexedData { Token(Token::Type::NumericLiteral, source.substr(0, length)), length };
            } else if (source.starts_with("0b")) {
                // Binary literal
                size_t length = 2;
                while (length < source.size() && (source[length] == '0' || source[length] == '1')) {
                    length++;
                }

                return LexedData { Token(Token::Type::NumericLiteral, source.substr(0, length)), length };
            } else if (source.starts_with("0o")) {
                // Octal literal
                size_t length = 2;
                while (length < source.size() && (source[length] >= '0' && source[length] <= '7')) {
                    length++;
                }

                return LexedData { Token(Token::Type::NumericLiteral, source.substr(0, length)), length };
            } else if (std::isdigit(source.front())) {
                // Decimal literal
                size_t length = 1;
                while (length < source.size() && std::isdigit(source[length])) {
                    length++;
                }

                return LexedData { Token(Token::Type::NumericLiteral, source.substr(0, length)), length };
            }

            return std::nullopt;
        }
    );

    template<hlp::StaticString Begin, hlp::StaticString End, Token::Type Type>
    using LexStringLike = decltype(
        [](std::string_view &source) -> std::optional<LexResult> {
            // Check if the source starts with the start sequence
            if (source.starts_with(Begin)) {
                // Get the length of the string literal.
                // A string literal is a sequence of characters starting with the start sequence and ending with the end sequence
                size_t length = 1;
                while (length < source.size() && !source.substr(length).starts_with(End)) {
                    length++;
                }

                // Check if the string literal is terminated
                if (length == source.size()) {
                    return std::unexpected(LexError::UnterminatedStringLiteral);
                }

                return LexedData { Token(Type, source.substr(Begin.size(), length - End.size())), length + End.size() };
            }

            return std::nullopt;
        }
    );

    template<hlp::StaticString Value>
    using LexSeparator = decltype(
        [](std::string_view &source) -> std::optional<LexedData> {
            // Check if the source starts with the separator
            // A separator is a sequence of non-alphanumerical characters that is not part of any other token
            if (source.starts_with(Value)) {
                return LexedData { Token(Token::Type::Separator, Value), Value.size() };
            }

            return std::nullopt;
        }
    );

    template<hlp::StaticString Value>
    using LexOperator = decltype(
        [](std::string_view &source) -> std::optional<LexedData> {
            // Check if the source starts with the operator
            // A operator is a sequence of non-alphanumerical characters that is not part of any other token
            // It's the same as a separator but is used in different contexts
            if (source.starts_with(Value)) {
                return LexedData { Token(Token::Type::Operator, Value), Value.size() };
            }

            return std::nullopt;
        }
    );

    using LexComment = decltype(
        [](std::string_view &source) -> std::optional<LexResult> {
            // Check if the source starts with the comment sequence
            if (source.starts_with("//")) {
                // Get the length of the comment.
                // A comment is a sequence of characters starting with the comment sequence and ending with a newline character
                size_t length = 2;
                while (length < source.size() && source[length] != '\n') {
                    length++;
                }

                return LexedData { Token(Token::Type::Comment, source.substr(0, length)), length };
            } else if (source.starts_with("/*")) {
                // Get the length of the comment.
                // A comment is a sequence of characters starting with the comment sequence and ending with a newline character
                size_t length = 2;
                while (length < source.size() && !source.substr(length).starts_with("*/")) {
                    length++;
                }

                // Check if the comment is terminated
                if (length == source.size()) {
                    return std::unexpected(LexError::UnterminatedComment);
                }

                return LexedData { Token(Token::Type::Comment, source.substr(0, length + 2)), length + 2 };
            }

            return std::nullopt;
        });

    /*
        This is the list of all tokens and token types that can be lexed by the lexer.
        The order of the tokens is important, because the lexer will try to lex the input with each token in order
        until it finds a lexer that can lex the input. If no lexer can lex the input, the lexer will return an error.
     */
    constexpr static auto Tokens = std::tuple<
            // Auxiliary tokens
            LexWhitespace,
            LexEndOfFile,
            LexComment,

            // Keywords
            LexKeyword<"driver">,
            LexKeyword<"fn">,
            LexKeyword<"namespace">,
            LexKeyword<"struct">,

            // Types
            LexBuiltinType<"u8">,
            LexBuiltinType<"u16">,
            LexBuiltinType<"u32">,
            LexBuiltinType<"u64">,
            LexBuiltinType<"i8">,
            LexBuiltinType<"i16">,
            LexBuiltinType<"i32">,
            LexBuiltinType<"i64">,
            LexBuiltinType<"f32">,
            LexBuiltinType<"f64">,
            LexBuiltinType<"bool">,
            LexBuiltinType<"char">,
            LexBuiltinType<"string">,
            LexBuiltinType<"bytes">,
            LexBuiltinType<"void">,

            // Literals
            LexStringLike<"\"", "\"", Token::Type::StringLiteral>,
            LexStringLike<"\'", "\'", Token::Type::CharacterLiteral>,
            LexStringLike<"[[", "]]", Token::Type::RawCodeBlock>,
            LexStringLike<"{%", "%}", Token::Type::Placeholder>,
            LexNumericLiteral,

            // Separators
            LexSeparator<"{">,
            LexSeparator<"}">,
            LexSeparator<"(">,
            LexSeparator<")">,
            LexSeparator<"[">,
            LexSeparator<"]">,
            LexSeparator<";">,
            LexSeparator<",">,

            // Operators
            LexOperator<"<">,
            LexOperator<">">,
            LexOperator<":">,
            LexOperator<"->">,

            // Identifiers
            LexIdentifier
    >();

    constexpr auto lexString(std::string_view &source, const auto &first, const auto &...rest) -> LexResult {

        // Iterate over all lexers and try to lex the input with each lexer
        if (auto result = first(source); result.has_value()) {
            // If the lexer was able to lex the input, return the result
            return *result;
        } else if constexpr (sizeof...(rest) > 0) {
            // If the lexer was not able to lex the input, try the next lexer
            return lexString(source, rest...);
        }

        // If no lexer was able to lex the input, return an error
        return std::unexpected(LexError::UnknownToken);
    }

    auto lexString(std::string_view &source) -> LexResult {
        return std::apply([&source](const auto &...lexers) {
            return lexString(source, lexers...);
        }, Tokens);
    }

    auto lex(std::string_view &source, const std::map<std::string, std::string> &placeholders) -> hlp::Generator<std::expected<Token, LexError>> {
        // This function is a generator / coroutine that yields tokens from the source code
        // It will try to lex the source code with each lexer in the Tokens tuple and yield the token if one was found.
        // If no lexer was able to lex the input, it will yield an error.
        // The generator will yield an EndOfInput token when the source code has been fully lexed.
        while (true) {

            // Try to lex the input with each lexer in the Tokens tuple
            auto result = lexString(source);

            // Check if the lexer was able to lex the input
            if (!result.has_value()) {
                co_yield std::unexpected(result.error());
                co_return;
            }

            {
                // Unpack the lexed result
                auto [token, length] = *result;

                // Remove the lexed token from the source code
                source = source.substr(length);

                // Handle tokens with special meaning
                switch (token.type()) {
                    using enum Token::Type;
                    case EndOfInput:
                        co_return;
                    case Placeholder:
                        if (auto it = placeholders.find(std::string(wolv::util::trim(token.value()))); it != placeholders.end()) {
                            auto &[key, value] = *it;

                            std::string_view valueView = value;
                            for (auto lexer = lexer::lex(valueView, placeholders); lexer;) {
                                auto newResult = lexer();
                                co_yield newResult;
                            }

                            continue;
                        } else {
                            co_yield std::unexpected(LexError::UnknownPlaceholder);
                            co_return;
                        }
                        break;
                    default:
                        break;
                }
            }

            // Get the lexed token and the length of the token
            auto [token, length] = *result;

            // Yield the token
            co_yield token;
        }
    }

}