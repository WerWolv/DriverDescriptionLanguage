#pragma once

#include <memory>

namespace compiler::hlp {

    template<typename T>
    auto unique_ptr_cast(auto &&ptr) -> std::unique_ptr<T> {
        return std::unique_ptr<T>(dynamic_cast<T *>(ptr.release()));
    }

}