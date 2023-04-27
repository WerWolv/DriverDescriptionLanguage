#include <compiler/language/parser.hpp>

#include <utility>

#include <wolv/utils/string.hpp>
#include <fmt/format.h>

namespace compiler::language::parser {
    using namespace lexer;

    auto Parser::getFullTypeName(std::string_view typeName) -> std::string {
        if (!this->m_namespaces.empty()) {
            return fmt::format("{}::{}", fmt::join(this->m_namespaces, "::"), typeName);
        } else {
            return std::string(typeName);
        }
    }

    auto Parser::parseDriver() -> ParseResult<ast::Node> {
        // Read the driver's name
        auto driverName = this->getFullTypeName(this->getValue(-1));

        // Parse template list
        std::vector<std::unique_ptr<ast::NodeVariable>> templateParameters;
        if (matchesSequence(OperatorLessThan)) {
            for (auto listParser = this->parseParameterList(); listParser;) {
                auto parameter = listParser();

                if (!parameter.has_value())
                    return std::unexpected(parameter.error());

                templateParameters.push_back(std::move(parameter.value()));
            }

            if (!matchesSequence(OperatorGreaterThan))
                return std::unexpected(ParseError::UnexpectedToken);
        }

        // Parse inheritance
        std::unique_ptr<ast::NodeDriver> inheritance;
        if (matchesSequence(OperatorColon)) {
            auto result = parseType(false);

            if (!result.has_value())
                return std::unexpected(result.error());

            inheritance = hlp::unique_ptr_cast<ast::NodeDriver>(result.value()->type()->clone());
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

        auto result = std::make_unique<ast::NodeDriver>(driverName, std::move(inheritance), std::move(templateParameters), std::move(functions));

        this->m_drivers[driverName] = result.get();

        return result;
    }

    auto Parser::parseParameterList() -> hlp::Generator<ParseResult<ast::NodeVariable>> {
        while (true) {
            // Parse the parameter type
            auto type = parseType();
            if (!type.has_value()) {
                co_yield std::unexpected(type.error());
                co_return;
            }

            // Parse the name of a parameter
            if (matchesSequence(Identifier)) {
                auto parameterName = this->getValue(-1);

                co_yield std::make_unique<ast::NodeVariable>(parameterName, std::move(*type));

                // Check if we have reached the end of the parameter list
                if (matchesSequence(SeparatorComma)) {
                    continue;
                } else {
                    break;
                }

            } else {
                co_yield std::unexpected(ParseError::UnexpectedToken);
                co_return;
            }
        }
    }

    [[nodiscard]] auto Parser::parseFunction() -> ParseResult<ast::NodeFunction> {
        auto functionName = this->getValue(-2);

        // Parse the function header
        std::vector<std::unique_ptr<ast::NodeVariable>> parameters;
        while (!matchesSequence(SeparatorCloseParenthesis)) {
            for (auto listParser = this->parseParameterList(); listParser;) {
                auto parameter = listParser();

                if (!parameter.has_value())
                    return std::unexpected(parameter.error());

                parameters.push_back(std::move(parameter.value()));
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
                } else {
                    return std::unexpected(ParseError::UnexpectedToken);
                }
            }
        }

        return std::make_unique<ast::NodeFunction>(functionName, std::move(parameters), std::move(body));
    }

    auto Parser::parseType(bool allowBuiltinTypes) -> ParseResult<ast::NodeType> {
        if (allowBuiltinTypes && matchesSequence(BuiltinType)) {
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
            auto typeName = std::string(this->getValue(-1));
            while (matchesSequence(OperatorColon, OperatorColon, Identifier)) {
                typeName += fmt::format("::{}", this->getValue(-1));
            }

            if (!this->m_drivers.contains(typeName))
                typeName = this->getFullTypeName(typeName);

            if (this->m_drivers.contains(typeName)) {
                auto driver = hlp::unique_ptr_cast<ast::NodeDriver>(this->m_drivers[typeName]->clone());

                if (matchesSequence(OperatorLessThan)) {
                    // Parse the template parameters
                    std::vector<lexer::Token> templateValues;
                    while (!matchesSequence(OperatorGreaterThan)) {
                        if (matchesSequence(NumericLiteral) || matchesSequence(StringLiteral) || matchesSequence(CharacterLiteral)) {
                            templateValues.emplace_back(this->m_current[-1]);
                        } else {
                            return std::unexpected(ParseError::UnexpectedToken);
                        }

                        if (matchesSequence(SeparatorComma)) {
                            continue;
                        }
                    }

                    if (templateValues.size() != driver->templateParameters().size())
                        return std::unexpected(ParseError::InvalidTemplateParameterCount);

                    driver->setTemplateValues(std::move(templateValues));
                }

                return std::make_unique<ast::NodeType>(typeName, std::move(driver));
            } else {
                return std::unexpected(ParseError::UnknownType);
            }
        } else {
            return std::unexpected(ParseError::UnexpectedToken);
        }
    }

    [[nodiscard]] auto Parser::parseNamespace() -> ASTGenerator {
        bool usedNamespace = false;
        if (matchesSequence(KeywordNamespace)) {
            usedNamespace = true;

            if (matchesSequence(Identifier, SeparatorOpenBrace)) {
                auto namespaceName = this->getValue(-2);

                this->m_namespaces.push_back(namespaceName);
            } else {
                co_yield std::unexpected(ParseError::UnexpectedToken);
                co_return;
            }
        }

        while (true) {
            // Check if we have reached the end of the input
            if (this->m_current == this->m_end || this->peek().type() == Token::Type::EndOfInput) {
                co_return;
            }

            if (matchesSequence(KeywordDriver, Identifier)) {
                co_yield parseDriver();
            } else if (this->peek() == KeywordNamespace) {
                for (auto namespaceParser = parseNamespace(); namespaceParser;) {
                    co_yield namespaceParser();
                }
            } else if (this->peek() == SeparatorCloseBrace || this->peek().type() == Token::Type::EndOfInput) {
                break;
            } else {
                co_yield std::unexpected(ParseError::UnexpectedToken);
                co_return;
            }
        }

        if (usedNamespace) {
            if (!matchesSequence(SeparatorCloseBrace)) {
                co_yield std::unexpected(ParseError::UnexpectedToken);
                co_return;
            }
            this->m_namespaces.pop_back();
        }
    }

    auto Parser::parse(const std::vector<lexer::Token> &tokens) -> ASTGenerator {
        this->m_current = tokens.begin();
        this->m_end     = tokens.end();

        while (true) {
            for (auto namespaceParser = parseNamespace(); namespaceParser;) {
                auto node = namespaceParser();

                if (!node.has_value()) {
                    co_yield std::unexpected(node.error());
                    co_return;
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

            // Check if we have reached the end of the input
            if (this->m_current == this->m_end || this->peek().type() == Token::Type::EndOfInput) {
                co_return;
            }
        }
    }

}