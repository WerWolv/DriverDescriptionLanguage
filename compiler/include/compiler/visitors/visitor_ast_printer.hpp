#pragma once

#include <compiler/language/ast/node.hpp>

namespace compiler::visitor {

    using namespace compiler::language;
    using namespace compiler::language::ast;

    struct VisitorASTPrinter : Visitor {
        auto visit(const NodeDriver &node) -> void override{
            this->handleIndent();

            fmt::print("driver {}", node.name());

            if (!node.templateParameters().empty()) {
                fmt::print("<");
                for (size_t i = 0; i < node.templateParameters().size(); i++) {
                    node.templateParameters()[i]->accept(*this);

                    if (i != node.templateParameters().size() - 1)
                        fmt::print(", ");
                }
                fmt::print("> ");
            } else {
                fmt::print(" ");
            }

            if (node.inheritance() != nullptr) {
                fmt::print(": {}", node.inheritance()->name());

                if (auto &values = node.inheritance()->templateValues(); !values.empty()) {
                    fmt::print("<");
                    for (size_t i = 0; i < values.size(); i++) {
                        auto &value = values[i];

                        switch (value.type()) {
                            case lexer::Token::Type::StringLiteral:
                                fmt::print("\"{}\"", value.value());
                                break;
                            case lexer::Token::Type::NumericLiteral:
                                fmt::print("{}", value.value());
                                break;
                            case lexer::Token::Type::CharacterLiteral:
                                fmt::print("'{}'", value.value());
                                break;
                            default:
                                break;
                        }

                        if (i != values.size() - 1)
                            fmt::print(", ");
                    }
                    fmt::print("> ");
                } else {
                    fmt::print(" ");
                }
            }

            fmt::print("{{\n\n");

            this->increaseIndent();
            for (auto &function : node.functions()) {
                function->accept(*this);
            }
            this->decreaseIndent();

            this->handleIndent();
            fmt::print("}}\n\n");
        }

        auto visit(const NodeFunction &node) -> void override {
            this->handleIndent();

            fmt::print("fn {}(", node.name());

            auto &parameters = node.parameters();
            for (size_t i = 0; i < parameters.size(); i++) {
                node.parameters()[i]->accept(*this);

                if (i != parameters.size() - 1)
                    fmt::print(", ");
            }
            fmt::print(") {{\n");

            this->increaseIndent();
            for (auto &statement : node.body()) {
                statement->accept(*this);
            }
            this->decreaseIndent();

            this->handleIndent();
            fmt::print("}}\n\n");

        }

        auto visit(const NodeVariable &node) -> void override {
            node.type()->accept(*this);
            fmt::print("{}", node.name());
        }

        auto visit(const NodeBuiltinType &node) -> void override {
            fmt::print("(0x{:02X}) ", node.size());
        }

        auto visit(const NodeType &node) -> void override {
            fmt::print("{} ", node.name());
            node.type()->accept(*this);
        }

        auto visit(const NodeRawCodeBlock &node) -> void override {
            this->handleIndent();

            fmt::print("{}\n", node.code());
        }

    private:
        auto increaseIndent() -> void {
            m_indent++;
        }

        auto decreaseIndent() -> void {
            m_indent--;
        }

        auto handleIndent() const -> void {
            for (size_t i = 0; i < m_indent; i++) {
                fmt::print("    ");
            }
        }

        size_t m_indent = 0;
    };

}