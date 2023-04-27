#include <compiler/language/parser.hpp>

#include <utility>

#include <wolv/utils/string.hpp>

namespace compiler::language::parser {
    using namespace lexer;

    auto Parser::parseDriver() -> ParseResult<ast::Node> {
        // Read the driver's name
        auto driverName = this->getValue(-1);

        // Parse the inheritance
        ParseResult<ast::NodeType> inheritance;
        if (matchesSequence(OperatorColon)) {
            inheritance = parseType();

            if (!inheritance.has_value())
                return std::unexpected(inheritance.error());
        }

        if (!matchesSequence(SeparatorOpenBrace))
            return std::unexpected(ParseError::UnexpectedToken);

        // Parse the content of the driver
        std::vector<std::unique_ptr<ast::NodeFunction>> functions;
        while (!matchesSequence(SeparatorCloseBrace)) {

            if (matchesSequence(KeywordFunction, Identifier, SeparatorOpenParenthesis)) {
                // Parse the function
                auto function = parseFunction();
                if (!function.has_value()) {
                    return function;
                }

                functions.emplace_back(std::move(function.value()));
            } else {
                return std::unexpected(ParseError::UnexpectedToken);
            }
        }

        auto result = std::make_unique<ast::NodeDriver>(driverName, std::move(inheritance.value()), std::move(functions));

        this->m_drivers[driverName] = result.get();

        return result;
    }

    [[nodiscard]] auto Parser::parseFunction() -> ParseResult<ast::NodeFunction> {
        auto functionName = this->getValue(-2);

        // Parse the function header
        std::vector<std::unique_ptr<ast::NodeVariable>> parameters;
        while (!matchesSequence(SeparatorCloseParenthesis)) {
            // Parse the parameter type
            auto type = parseType();
            if (!type.has_value()) {
                return std::unexpected(type.error());
            }

            // Parse the name of a parameter
            if (matchesSequence(Identifier)) {
                auto parameterName = this->getValue(-1);

                parameters.emplace_back(std::make_unique<ast::NodeVariable>(parameterName, std::move(*type)));

                // Check if we have reached the end of the parameter list
                if (matchesSequence(SeparatorComma)) {
                    continue;
                } else if (matchesSequence(SeparatorCloseParenthesis)) {
                    break;
                } else {
                    return std::unexpected(ParseError::UnexpectedToken);
                }

            } else {
                return std::unexpected(ParseError::UnexpectedToken);
            }
        }

        // Parse the function body
        if (!matchesSequence(SeparatorOpenBrace)) {
            return std::unexpected(ParseError::UnexpectedToken);
        }

        std::vector<std::unique_ptr<ast::Node>> body;
        {
            // Parse the function body
            while (!matchesSequence(SeparatorCloseBrace)) {
                if (matchesSequence(RawCodeBlock)) {
                    body.emplace_back(std::make_unique<ast::NodeRawCodeBlock>(wolv::util::trim(this->getValue(-1))));
                }
            }
        }

        return std::make_unique<ast::NodeFunction>(functionName, std::move(parameters), std::move(body));
    }

    auto Parser::parseType() -> ParseResult<ast::NodeType> {
        if (matchesSequence(BuiltinType)) {
            // Parse a builtin type

            // Read the type name
            auto typeName = this->getValue(-1);

            // Determine the type
            ast::NodeBuiltinType::Type builtinType = [&typeName] {
                if (typeName.starts_with('u'))
                    return ast::NodeBuiltinType::Type::Unsigned;
                else if (typeName.starts_with('i'))
                    return ast::NodeBuiltinType::Type::Signed;
                else if (typeName.starts_with('f'))
                    return ast::NodeBuiltinType::Type::FloatingPoint;
                else if (typeName == "bool")
                    return ast::NodeBuiltinType::Type::Boolean;
                else
                    std::unreachable();
            }();

            // Determine the size
            size_t size = [&typeName] {
                if (typeName.ends_with('8') || typeName == "bool")
                    return 1;
                else if (typeName.ends_with("16"))
                    return 2;
                else if (typeName.ends_with("32"))
                    return 4;
                else if (typeName.ends_with("64"))
                    return 8;
                else
                    std::unreachable();
            }();

            auto type = std::make_unique<ast::NodeBuiltinType>(builtinType, size);

            return std::make_unique<ast::NodeType>(typeName, std::move(type));
        } else if (matchesSequence(Identifier)) {
            auto typeName = this->getValue(-1);

            if (this->m_drivers.contains(typeName)) {
                return std::make_unique<ast::NodeType>(typeName, this->m_drivers[typeName]->clone());
            } else {
                return std::unexpected(ParseError::UnknownType);
            }
        } else {
            return std::unexpected(ParseError::UnexpectedToken);
        }
    }

    auto Parser::parse(const std::vector<lexer::Token> &tokens) -> ASTGenerator {
        this->m_current = tokens.begin();
        this->m_end     = tokens.end();

        while (true) {
            // Check if we have reached the end of the input
            if (this->m_current == this->m_end || this->peek().type() == Token::Type::EndOfInput) {
                co_return;
            }

            ParseResult<ast::Node> node;

            // Parse top level constructs
            if (matchesSequence(KeywordDriver, Identifier)) {
                node = parseDriver();
            } else {
                node = std::unexpected(ParseError::UnexpectedToken);
            }

            // Check if we have encountered an error
            bool error = !node.has_value();

            // Yield the node
            co_yield std::move(node);

            // If we have encountered an error, stop parsing
            if (error) {
                co_return;
            }
        }
    }

}