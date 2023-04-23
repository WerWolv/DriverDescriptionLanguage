#pragma once

#include <compiler/types.hpp>
#include <compiler/language/lexer.hpp>
#include <compiler/language/ast/node.hpp>

#include <vector>

namespace compiler::language::parser {

    enum class ParseError {
        UnexpectedToken,
        EndOfInput,
    };

    template<typename T>
    using ParseResult = std::expected<std::unique_ptr<T>, ParseError>;

    using ASTGenerator = hlp::Generator<ParseResult<ast::NodeBase>>;

    struct Parser {
    public:
        [[nodiscard]] auto parseDriver() -> ParseResult<ast::NodeBase>;
        [[nodiscard]] auto parseFunction() -> ParseResult<ast::NodeFunction>;
        [[nodiscard]] auto parseType() -> ParseResult<ast::NodeType>;

        [[nodiscard]] auto parse(const std::vector<lexer::Token> &tokens) -> ASTGenerator;

    private:
        [[nodiscard]] auto peek() const -> const lexer::Token & {
            return *this->m_current;
        }

        [[nodiscard]] auto next() {
            *this->m_current++;
        }

        [[nodiscard]] auto matchesSequence(const auto &... tokens) -> bool {
            auto current = this->m_current;
            for (const auto &token : { tokens... }) {
                if (current == this->m_end ||                                       // Check if we have reached the end of the input
                    current->type() != token.type() ||                              // Check if the token type matches
                    (current->value() != token.value() && !token.value().empty()))  // Check if the token value matches
                {
                    // The current token does not match the expected token
                    return false;
                } else {
                    // The current token matches the expected token
                    // Move to the next token
                    current++;
                }
            }

            // All tokens matched, update the current token iterator
            this->m_current = current;

            return true;
        }

        auto getValue(i32 offset) -> std::string_view {
            return this->m_current[offset].value();
        }

    private:
        std::vector<lexer::Token>::const_iterator m_current;
        std::vector<lexer::Token>::const_iterator m_end;
    };

}

template <> struct fmt::formatter<compiler::language::parser::ParseError>: formatter<std::string_view> {
    template <typename FormatContext>
    auto format(compiler::language::parser::ParseError error, FormatContext& ctx) const {
        string_view name = "unknown";

        switch (error) {
            using enum compiler::language::parser::ParseError;
            case UnexpectedToken: name = "unexpected token"; break;
            case EndOfInput:      name = "end of input";     break;
        }

        return formatter<string_view>::format(name, ctx);
    }
};