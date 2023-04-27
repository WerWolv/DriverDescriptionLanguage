project(compiler)

add_executable(compiler
        source/main.cpp

        source/specs/specs_file.cpp

        source/language/lexer.cpp
        source/language/parser.cpp
        source/language/compiler.cpp
)

target_include_directories(compiler
        PUBLIC
        include
)

target_link_libraries(compiler
        PUBLIC
        fmt::fmt
        libwolv-utils
        libwolv-io
        cxxopts::cxxopts
        tomlplusplus::tomlplusplus
)