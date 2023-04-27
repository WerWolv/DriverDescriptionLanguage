#pragma once

#include <compiler/language/ast/node.hpp>

#include <wolv/utils/string.hpp>

#include <string>
#include <vector>

namespace compiler::visitor {

    using namespace compiler::language;
    using namespace compiler::language::ast;

    struct VisitorCGenerator : Visitor {
        auto visit(const NodeDriver &node) -> void override {
            this->pushPrefix(node);

            for (const auto& parameter : node.templateParameters()) {
                this->m_templateParameters.emplace_back(parameter.get(), lexer::Token());
            }

            {
                const auto &inheritance = node.inheritance();
                if (inheritance != nullptr) {
                    this->pushPrefix(*inheritance);

                    const auto &templateParameters = inheritance->templateParameters();
                    const auto &templateValues = inheritance->templateValues();

                    for (size_t i = 0; i < templateParameters.size(); i++) {
                        auto templateParameterFunction = fmt::format("static {} {}_{}() {{ return {}; }}\n",
                                                                     templateParameters[i]->type()->name(),
                                                                     this->m_prefixes.back(),
                                                                     templateParameters[i]->name(),
                                                                     templateValues[i].value());

                        this->m_forwardDecls += templateParameterFunction;
                    }

                    this->popPrefix();
                }
            }

            for (auto &child : node.functions())
                child->accept(*this);

            this->m_templateParameters.clear();

            this->popPrefix();
        }

        auto visit(const NodeFunction &node) -> void override {
            std::string function = fmt::format("static void {}_{}(", this->m_prefixes.back(), node.name());

            for (size_t i = 0; i < node.parameters().size(); i++) {
                auto &parameter = node.parameters()[i];

                function += fmt::format("{} {}", parameter->type()->name(), parameter->name());

                if (i != node.parameters().size() - 1)
                    function += ", ";
            }

            function += ")";

            this->m_source += function + " {\n";
            this->m_forwardDecls += function + ";\n";

            for (auto &[parameter, value] : this->m_templateParameters) {
                this->m_source += fmt::format("    const {} {} = {}_{}();\n", parameter->type()->name(), parameter->name(), this->m_prefixes.back(), parameter->name());
            }

            this->m_source += "\n";

            for (auto &child : node.body())
                child->accept(*this);

            this->m_source += "}\n\n";
        }

        auto visit(const NodeVariable &node) -> void override {
            this->m_source += fmt::format("    {} {};\n", node.type()->name(), node.name());
        }

        auto visit(const NodeBuiltinType &node) -> void override {

        }

        auto visit(const NodeType &node) -> void override {

        }

        auto visit(const NodeRawCodeBlock &node) -> void override {
            for (const auto &line : wolv::util::splitString(std::string(node.code()), "\n")) {
                this->m_source += fmt::format("    {}\n", wolv::util::trim(line));
            }
        }

        [[nodiscard]] auto source() const -> std::string {
            return fmt::format("{}\n{}", this->m_forwardDecls, this->m_source);
        }

        [[nodiscard]] auto include() const -> const std::string& {
            return this->m_include;
        }

    private:
        

        auto pushPrefix(const ast::NodeDriver &node) -> void {
            auto prefix = wolv::util::replaceStrings(std::string(node.name()), "::", "_");
            this->m_prefixes.emplace_back("drv_" + prefix);
        }

        auto popPrefix() -> void {
            this->m_prefixes.pop_back();
        }

    private:
        std::string m_source, m_forwardDecls, m_include;

        std::vector<std::string> m_prefixes;
        std::vector<std::pair<NodeVariable*, lexer::Token>> m_templateParameters;
    };

}