#pragma once

#include <filesystem>
#include <set>
#include <string>

#include <compiler/specs/specs_file.hpp>
#include <compiler/language/ast/node.hpp>

namespace compiler::language {

    class Compiler {
    public:
        explicit Compiler(const std::filesystem::path &path) : m_specsFile(path) {}

        auto compile(ast::Visitor &visitor) {
            auto nodes = this->processSpecsFile(this->m_specsFile);

            for (auto &node : nodes) {
                node->accept(visitor);
            }
        }

    private:
        auto processSpecsFile(const compiler::specs::SpecsFile &specsFile) -> std::vector<std::unique_ptr<ast::Node>>;
        auto processDriver(const compiler::specs::Driver &driver) -> std::vector<std::unique_ptr<ast::Node>>;
        auto compileCode(std::string_view code, const std::map<std::string, std::string> &placeholders) -> std::vector<std::unique_ptr<ast::Node>>;

    private:
        compiler::specs::SpecsFile m_specsFile;
        std::set<std::string> m_compiledDrivers;
        std::map<std::string, ast::NodeDriver*> m_drivers;
    };

}