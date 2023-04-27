cmake_minimum_required(VERSION 3.25)
project(DriverDefinitionLanguage)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
        GIT_TAG         9.1.0
        GIT_PROGRESS    TRUE
        USES_TERMINAL_DOWNLOAD TRUE
)

FetchContent_Declare(
        libwolv
        GIT_REPOSITORY https://github.com/WerWolv/libwolv.git
        GIT_PROGRESS TRUE
        USES_TERMINAL_DOWNLOAD TRUE
)

FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG v3.1.1
        GIT_PROGRESS TRUE
        USES_TERMINAL_DOWNLOAD TRUE
)

FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.3.0
        GIT_PROGRESS TRUE
        USES_TERMINAL_DOWNLOAD TRUE
)

FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(libwolv)
FetchContent_MakeAvailable(cxxopts)
FetchContent_MakeAvailable(tomlplusplus)

add_subdirectory(compiler)
