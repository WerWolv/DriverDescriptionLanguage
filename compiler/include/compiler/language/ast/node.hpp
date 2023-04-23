#pragma once

#include <memory>
#include <vector>

namespace compiler::language::ast {

    struct NodeDriver;
    struct NodeFunction;
    struct NodeVariable;
    struct NodeBuiltinType;
    struct NodeType;
    struct NodeRawCodeBlock;

    struct Visitor {
        virtual ~Visitor() = default;

        virtual void visit(const NodeDriver &node) = 0;
        virtual void visit(const NodeFunction &node) = 0;
        virtual void visit(const NodeVariable &node) = 0;
        virtual void visit(const NodeBuiltinType &node) = 0;
        virtual void visit(const NodeType &node) = 0;
        virtual void visit(const NodeRawCodeBlock &node) = 0;
    };

    struct NodeBase {
        virtual ~NodeBase() = default;

        virtual void accept(Visitor &visitor) const = 0;
    };

    template<typename T>
    struct Node : public NodeBase {
        [[nodiscard]] std::unique_ptr<T> clone() const {
            return std::make_unique<T>(static_cast<const T &>(*this));
        }
    };

    struct NodeBuiltinType : public Node<NodeBuiltinType> {
        enum class Type {
            Unsigned,
            Signed,
            FloatingPoint,
            Boolean
        };

        explicit NodeBuiltinType(Type type, size_t size) : m_type(type), m_size(size) {}

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

    struct NodeType : public Node<NodeType> {
        NodeType(std::string_view name, std::unique_ptr<ast::NodeBase> &&type)
                : m_name(name), m_type(std::move(type)) {}

        ~NodeType() override = default;

        void accept(Visitor &visitor) const override {
            visitor.visit(*this);
        }

        [[nodiscard]] auto name() const -> std::string_view {
            return this->m_name;
        }

        [[nodiscard]] auto type() const -> const NodeBase * {
            return this->m_type.get();
        }

    private:
        std::string_view m_name;
        std::unique_ptr<NodeBase> m_type;
    };

    struct NodeVariable : public Node<NodeVariable> {
        NodeVariable(std::string_view name, std::unique_ptr<NodeType> &&type) : m_name(name), m_type(std::move(type)) {}

        ~NodeVariable() override = default;

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

    struct NodeFunction : public Node<NodeFunction> {
        explicit NodeFunction(std::string_view name,
                              std::vector<std::unique_ptr<ast::NodeVariable>> &&parameters,
                              std::vector<std::unique_ptr<ast::NodeBase>> &&body)
            : m_name(name), m_parameters(std::move(parameters)), m_body(std::move(body)) { }

        ~NodeFunction() override = default;

        void accept(Visitor &visitor) const override {
            visitor.visit(*this);
        }

        [[nodiscard]] auto name() const -> std::string_view {
            return this->m_name;
        }

        [[nodiscard]] auto parameters() const -> const std::vector<std::unique_ptr<ast::NodeVariable>> & {
            return this->m_parameters;
        }

        [[nodiscard]] auto body() const -> const std::vector<std::unique_ptr<ast::NodeBase>> & {
            return this->m_body;
        }

    private:
        std::string_view m_name;
        std::vector<std::unique_ptr<ast::NodeVariable>> m_parameters;
        std::vector<std::unique_ptr<ast::NodeBase>> m_body;
    };

    struct NodeDriver : public Node<NodeDriver> {
        NodeDriver(std::string_view name, std::unique_ptr<NodeDriver> &&inheritance, std::vector<std::unique_ptr<NodeFunction>> &&functions)
            : m_name(name), m_inheritance(std::move(inheritance)), m_functions(std::move(functions)) { }
        ~NodeDriver() override = default;

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

    private:
        std::string_view m_name;
        std::unique_ptr<NodeDriver> m_inheritance;
        std::vector<std::unique_ptr<NodeFunction>> m_functions;
    };

    struct NodeRawCodeBlock : public Node<NodeRawCodeBlock> {
        explicit NodeRawCodeBlock(std::string_view code) : m_code(code) { }
        ~NodeRawCodeBlock() override = default;

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