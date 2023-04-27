#pragma once

#include <string>
#include <filesystem>
#include <map>
#include <vector>

namespace compiler::specs {

    struct Driver {
        std::string code;
        std::map<std::string, std::string> config;
        std::vector<std::string> dependencies;
    };

    class SpecsFile {
    public:
        SpecsFile() = default;
        explicit SpecsFile(const std::filesystem::path &path);

        [[nodiscard]] auto drivers() const -> const std::map<std::string, Driver>& {
            return m_drivers;
        }

    private:
        std::map<std::string, Driver> m_drivers;
    };

}

