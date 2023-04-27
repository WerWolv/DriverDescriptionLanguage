#include <cstdlib>

#include <compiler/language/compiler.hpp>
#include <compiler/visitors/visitor_ast_printer.hpp>


auto main() -> int {
    compiler::language::Compiler compiler("./test.toml");

    compiler::visitor::VisitorASTPrinter printer;
    compiler.compile(printer);

    return EXIT_SUCCESS;
}