#include <compiler/language/compiler.hpp>

#include <compiler/language/lexer.hpp>
#include <compiler/language/parser.hpp>

#include <wolv/io/file.hpp>

namespace compiler::language {

    auto Compiler::compileCode(std::string_view code, const std::map<std::string, std::string> &placeholders) -> std::vector<std::unique_ptr<ast::Node>> {

        // Lex the source code into tokens
        std::vector<lexer::Token> tokens;
        for (auto lexer = lexer::lex(code, placeholders); lexer;) {
            auto result = lexer();

            // Handle lexer errors
            if (!result.has_value()) {
                throw std::runtime_error(fmt::format("Lexer Error: {}", result.error()));
            }

            // Insert new tokens into list
            tokens.emplace_back(result.value());
        }

        std::vector<std::unique_ptr<ast::Node>> nodes;

        // Prepare the parser, make drivers from dependencies available to the new parser
        auto parser = parser::Parser();
        parser.setDrivers(std::move(this->m_drivers));

        // Parse the tokens into an AST
        for (auto parse = parser.parse(tokens); parse;) {
            auto result = parse();

            // Handle parser errors
            if (!result.has_value()) {
                throw std::runtime_error(fmt::format("Parser Error: {}", result.error()));
            }

            // Insert new AST nodes into list
            nodes.emplace_back(std::move(result.value()));
        }

        // Save new drivers for later use
        this->m_drivers = parser.getDrivers();

        return nodes;
    }

    auto Compiler::processDriver(const compiler::specs::Driver &driver) -> std::vector<std::unique_ptr<ast::Node>> {
        std::vector<std::unique_ptr<ast::Node>> nodes;

        auto &drivers = this->m_specsFile.drivers();

        // Recursively process all dependencies of the current driver
        for (auto &dependency : driver.dependencies) {

            // Make sure the dependency hasn't been compiled already
            if (this->m_compiledDrivers.contains(dependency)) {
                continue;
            }

            // Make sure the dependency exists
            if (!drivers.contains(dependency))
                throw std::runtime_error(fmt::format("Dependency \"{}\" does not exist", dependency));

            // Process the dependency
            auto newNodes = processDriver(drivers.at(dependency));
            std::move(newNodes.begin(), newNodes.end(), std::back_inserter(nodes));

            // Mark the dependency as compiled
            this->m_compiledDrivers.insert(dependency);
        }

        auto newNodes = this->compileCode(driver.code, driver.config);
        std::move(newNodes.begin(), newNodes.end(), std::back_inserter(nodes));

        return nodes;
    }

    auto Compiler::processSpecsFile(const compiler::specs::SpecsFile &specsFile) -> std::vector<std::unique_ptr<ast::Node>> {
        // Clear what drivers have been compiled already
        this->m_compiledDrivers.clear();

        // Process all drivers mentioned in the specs file
        std::vector<std::unique_ptr<ast::Node>> nodes;
        for (const auto &[name, driver] : specsFile.drivers()) {
            auto newNodes = processDriver(driver);

            // Insert new nodes into result
            std::move(newNodes.begin(), newNodes.end(), std::back_inserter(nodes));
        }

        return nodes;
    }

}