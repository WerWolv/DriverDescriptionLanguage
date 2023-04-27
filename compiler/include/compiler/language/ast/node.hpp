#pragma once

#include <memory>
#include <vector>

#include <compiler/helpers/utils.hpp>

namespace compiler::language::ast {

    struct NodeDriver;
    struct NodeFunction;
    struct NodeVariable;
    struct NodeBuiltinType;
    struct NodeType;
    struct NodeRawCodeBlock;

    struct Visitor {
        virtual ~Visitor() = default;

        virtual void visit(const NodeDriver &node)          = 0;
        virtual void visit(const NodeFunction &node)        = 0;
        virtual void visit(const NodeVariable &node)        = 0;
        virtual void visit(const NodeBuiltinType &node)     = 0;
        virtual void visit(const NodeType &node)            = 0;
        virtual void visit(const NodeRawCodeBlock &node)    = 0;
    };

    struct Node {
        virtual ~Node() = default;
        [[nodiscard]] virtual auto clone() const -> std::unique_ptr<Node> = 0;
        virtual auto accept(Visitor &visitor) const -> void = 0;

    };

    struct NodeBuiltinType : public Node {
        enum class Type {
            Unsigned,
            Signed,
            FloatingPoint,
            Boolean
        };

        explicit NodeBuiltinType(Type type, size_t size) : m_type(type), m_size(size) {}

        [[nodiscard]] auto clone() const -> std::unique_ptr<Node> override {
            return std::make_unique<NodeBuiltinType>(*this);
        }

        [[nodiscard]] auto type() const -> Type {
            return this->m_type;
        }

        [[nodiscard]] auto size() const -> size_t {
            return this->m_size;
        }

        void accept(Visitor &visitor) const override {
            visitor.visit(*this);
        }

    private:
        Type m_type;
        size_t m_size;
    };

    struct NodeType : public Node {
        NodeType(std::string_view name, std::unique_ptr<ast::Node> &&type)
                : m_name(name), m_type(std::move(type)) {}

        ~NodeType() override = default;

        NodeType(const NodeType &other) {
            this->m_name = other.m_name;
            this->m_type = hlp::unique_ptr_cast<Node>(other.m_type->clone());
        }

        [[nodiscard]] auto clone() const -> std::unique_ptr<Node> override {
            return std::make_unique<NodeType>(*this);
        }

        void accept(Visitor &visitor) const override {
            visitor.visit(*this);
        }

        [[nodiscard]] auto name() const -> std::string_view {
            return this->m_name;
        }

        [[nodiscard]] auto type() const -> const Node * {
            return this->m_type.get();
        }

    private:
        std::string_view m_name;
        std::unique_ptr<Node> m_type;
    };

    struct NodeVariable : public Node {
        NodeVariable(std::string_view name, std::unique_ptr<NodeType> &&type) : m_name(name), m_type(std::move(type)) {}

        ~NodeVariable() override = default;

        NodeVariable(const NodeVariable &other) {
            this->m_name = other.m_name;
            this->m_type = hlp::unique_ptr_cast<NodeType>(other.m_type->clone());
        }

        [[nodiscard]] auto clone() const -> std::unique_ptr<Node> override {
            return std::make_unique<NodeVariable>(*this);
        }

        void accept(Visitor &visitor) const override {
            visitor.visit(*this);
        }

        [[nodiscard]] auto name() const -> std::string_view {
            return this->m_name;
        }

        [[nodiscard]] auto type() const -> const NodeType * {
            return static_cast<const NodeType *>(this->m_type.get());
        }

    private:
        std::string_view m_name;
        std::unique_ptr<NodeType> m_type;
    };

    struct NodeFunction : public Node {
        explicit NodeFunction(std::string_view name,
                              std::vector<std::unique_ptr<ast::NodeVariable>> &&parameters,
                              std::vector<std::unique_ptr<ast::Node>> &&body)
            : m_name(name), m_parameters(std::move(parameters)), m_body(std::move(body)) { }

        ~NodeFunction() override = default;

        NodeFunction(const NodeFunction &other) {
            this->m_name = other.m_name;
            for (const auto &parameter : other.m_parameters) {
                this->m_parameters.emplace_back(hlp::unique_ptr_cast<NodeVariable>(parameter->clone()));
            }
            for (const auto &node : other.m_body) {
                this->m_body.emplace_back(node->clone());
            }
        }

        [[nodiscard]] auto clone() const -> std::unique_ptr<Node> override {
            return std::make_unique<NodeFunction>(*this);
        }

        void accept(Visitor &visitor) const override {
            visitor.visit(*this);
        }

        [[nodiscard]] auto name() const -> std::string_view {
            return this->m_name;
        }

        [[nodiscard]] auto parameters() const -> const std::vector<std::unique_ptr<ast::NodeVariable>> & {
            return this->m_parameters;
        }

        [[nodiscard]] auto body() const -> const std::vector<std::unique_ptr<ast::Node>> & {
            return this->m_body;
        }

    private:
        std::string_view m_name;
        std::vector<std::unique_ptr<ast::NodeVariable>> m_parameters;
        std::vector<std::unique_ptr<ast::Node>> m_body;
    };

    struct NodeDriver : public Node {
        NodeDriver(
                std::string_view name,
                std::unique_ptr<NodeDriver> &&inheritance,
                std::vector<std::unique_ptr<NodeVariable>> &&templateParameters,
                std::vector<std::unique_ptr<NodeFunction>> &&functions
                ) :
                m_name(name),
                m_inheritance(std::move(inheritance)),
                m_templateParameters(std::move(templateParameters)),
                m_functions(std::move(functions)) { }

        ~NodeDriver() override = default;

        NodeDriver(const NodeDriver &other) {
            this->m_name = other.m_name;

            if (other.m_inheritance != nullptr)
                this->m_inheritance = hlp::unique_ptr_cast<NodeDriver>(other.m_inheritance->clone());

            for (const auto &parameter : other.m_templateParameters) {
                this->m_templateParameters.emplace_back(hlp::unique_ptr_cast<NodeVariable>(parameter->clone()));
            }

            this->m_templateValues = other.m_templateValues;

            for (const auto &function : other.m_functions) {
                this->m_functions.emplace_back(hlp::unique_ptr_cast<NodeFunction>(function->clone()));
            }
        }

        [[nodiscard]] auto clone() const -> std::unique_ptr<Node> override {
            return std::make_unique<NodeDriver>(*this);
        }

        void accept(Visitor &visitor) const override {
            visitor.visit(*this);
        }

        [[nodiscard]] auto name() const -> std::string_view {
            return this->m_name;
        }

        [[nodiscard]] auto inheritance() const -> const NodeDriver * {
            return this->m_inheritance.get();
        }

        [[nodiscard]] auto functions() const -> const std::vector<std::unique_ptr<NodeFunction>> & {
            return this->m_functions;
        }

        [[nodiscard]] auto templateParameters() const -> const std::vector<std::unique_ptr<NodeVariable>> & {
            return this->m_templateParameters;
        }

        [[nodiscard]] auto templateValues() const -> const std::vector<lexer::Token> & {
            return this->m_templateValues;
        }

        auto setTemplateValues(std::vector<lexer::Token> &&arguments) -> void {
            this->m_templateValues = std::move(arguments);
        }

    private:
        std::string_view m_name;
        std::unique_ptr<NodeDriver> m_inheritance;
        std::vector<std::unique_ptr<NodeVariable>> m_templateParameters;
        std::vector<lexer::Token> m_templateValues;
        std::vector<std::unique_ptr<NodeFunction>> m_functions;
    };

    struct NodeRawCodeBlock : public Node {
        explicit NodeRawCodeBlock(std::string_view code) : m_code(code) { }
        ~NodeRawCodeBlock() override = default;

        NodeRawCodeBlock(const NodeRawCodeBlock &other) {
            this->m_code = other.m_code;
        }

        [[nodiscard]] auto clone() const -> std::unique_ptr<Node> override {
            return std::make_unique<NodeRawCodeBlock>(*this);
        }

        void accept(Visitor &visitor) const override {
            visitor.visit(*this);
        }

        [[nodiscard]] auto code() const -> std::string_view {
            return this->m_code;
        }
    private:
        std::string_view m_code;
    };

}