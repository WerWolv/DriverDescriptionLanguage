#include <cstdlib>

#include <compiler/language/compiler.hpp>
#include <compiler/visitors/visitor_ast_printer.hpp>
#include <compiler/visitors/visitor_c_generator.hpp>

#include <thread>
#include <chrono>

auto main() -> int {
    compiler::language::Compiler compiler("./specs/test.toml");

    compiler::visitor::VisitorCGenerator visitor;
    compiler.compile(visitor);

    fmt::print("{}\n", visitor.source());

    getchar();

    return EXIT_SUCCESS;
}