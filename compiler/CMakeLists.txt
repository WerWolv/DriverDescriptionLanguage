project(compiler)

add_executable(compiler
        source/main.cpp
        source/language/lexer.cpp
        source/language/parser.cpp
)

target_include_directories(compiler
        PUBLIC
        include
)

target_link_libraries(compiler
        PUBLIC
        fmt::fmt
        libwolv-utils
)