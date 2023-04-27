#include <compiler/specs/specs_file.hpp>

#include <toml++/toml.h>

#include <wolv/io/file.hpp>

namespace compiler::specs {
    
    SpecsFile::SpecsFile(const std::filesystem::path &specsFilePath) {
        auto specs = toml::parse_file(specsFilePath.u8string());

        // Loop over the entire specs file, looking for driver definitions
        for (auto &&[driverName, driverContent] : specs) {
            Driver driver;

            // Make sure the driver content is a table
            if (!driverContent.is_table()) {
                throw std::runtime_error("Driver content must be a table");
            }

            auto &driverTable = *driverContent.as_table();

            // Read the "path" key from the driver table
            {
                // Make sure the "path" key exists
                auto pathValue = driverTable["path"];
                if (!pathValue.is_string()) {
                    throw std::runtime_error("Driver path must be a string");
                }

                // Make sure the path exists
                std::filesystem::path path = *pathValue.value<std::u8string>();
                if (!std::filesystem::exists(path)) {
                    throw std::runtime_error("Driver path does not exist");
                }

                // Make sure the path is a file
                wolv::io::File file(path, wolv::io::File::Mode::Read);
                if (!file.isValid()) {
                    throw std::runtime_error("Driver path is not a file");
                }

                // Read the driver code from the file
                driver.code = file.readString();
            }

            // Read the "config" object from the driver table
            // This key is optional
            {
                auto config = driverTable["config"];
                if (config) {
                    if (!config.is_table()) {
                        throw std::runtime_error("Driver config must be a table");
                    }

                    auto &configTable = *config.as_table();

                    // Process the config table
                    for (auto &&[key, value] : configTable) {
                        if (!value.is_string()) {
                            throw std::runtime_error("Driver config values must be strings");
                        }

                        driver.config[std::string(key)] = *value.value<std::string>();
                    }
                }
            }

            // Read the "depends" array from the driver table
            // This key is optional
            {
                auto dependencies = driverTable["depends"];
                if (dependencies) {
                    if (!dependencies.is_array()) {
                        throw std::runtime_error("Driver dependencies must be an array");
                    }

                    auto &dependenciesArray = *dependencies.as_array();

                    // Process the dependencies array
                    for (auto &&dependency : dependenciesArray) {
                        if (!dependency.is_string()) {
                            throw std::runtime_error("Driver dependency values must be strings");
                        }

                        driver.dependencies.emplace_back(*dependency.value<std::string>());
                    }
                }
            }

            this->m_drivers.emplace(driverName, std::move(driver));
        }
    }
    
}