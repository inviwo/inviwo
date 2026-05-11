/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <modules/python3/python3moduledefine.h>

#include <pybind11/pybind11.h>

#include <any>
#include <functional>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace inviwo {

/**
 * @brief Extensible factory for converting between std::any and pybind11::object.
 *
 * Provides a registry of converters that can be extended at runtime by different modules.
 * This replaces hard-coded type conversion lists and makes it possible for any module
 * to register its own type converters.
 *
 * The factory supports two conversion directions:
 * - **std::any → py::object**: Uses a `std::type_index`-keyed map for O(1) lookup based on the
 *   type stored in the `std::any`. Register converters with `registerToPy()`.
 * - **py::object → std::any**: Uses an ordered vector of converter functions that are tried in
 *   sequence until one succeeds. Register converters with `registerToAny()`. The registration
 *   order matters — more specific types (e.g., `py::bool_`) must be registered before more
 *   general types (e.g., `py::int_`).
 *
 * Usage:
 * @code
 * PyAnyConverter converter;
 * converter.registerToPy<double>();
 * converter.registerToPy<int>();
 * converter.registerToAny<bool, pybind11::bool_>();
 * converter.registerToAny<double, pybind11::float_>();
 * converter.registerToAny<int, pybind11::int_>();
 *
 * py::object pyVal = converter.toPyObject(std::any(42));
 * std::any cppVal = converter.toAny(py::float_(3.14));
 * @endcode
 */
class IVW_MODULE_PYTHON3_API PyAnyConverter {
public:
    /**
     * Converter function type for std::any → py::object.
     * Takes a reference to the std::any and returns a py::object.
     */
    using ToPyFn = std::function<pybind11::object(const std::any&)>;

    /**
     * Converter function type for py::object → std::any.
     * Takes a pybind11::handle and returns a std::any if the conversion is possible,
     * or std::nullopt if this converter cannot handle the given Python object.
     */
    using ToAnyFn = std::function<std::optional<std::any>(pybind11::handle)>;

    PyAnyConverter() = default;

    /**
     * @brief Register a converter for std::any → py::object for a specific C++ type.
     * @param type The type_index of the C++ type stored in std::any.
     * @param converter The converter function.
     */
    void registerToPy(std::type_index type, ToPyFn converter);

    /**
     * @brief Convenience: register a converter for std::any → py::object using pybind11::cast.
     * Works for any type that pybind11 knows how to convert.
     * @tparam T The C++ type to register.
     */
    template <typename T>
    void registerToPy() {
        registerToPy(std::type_index(typeid(T)), [](const std::any& a) -> pybind11::object {
            return pybind11::cast(std::any_cast<const T&>(a));
        });
    }

    /**
     * @brief Register a converter for py::object → std::any.
     * Converters are tried in the order they were registered. The first one that returns
     * a non-nullopt result wins. More specific types should be registered before more
     * general types.
     * @param converter The converter function.
     */
    void registerToAny(ToAnyFn converter);

    /**
     * @brief Convenience: register a converter for py::object → std::any.
     * Uses pybind11::isinstance to check the Python type and pybind11::cast to convert.
     * @tparam CppType The C++ type to store in the std::any.
     * @tparam PyType The pybind11 type to check with isinstance (e.g., pybind11::float_).
     */
    template <typename CppType, typename PyType>
    void registerToAny() {
        registerToAny([](pybind11::handle obj) -> std::optional<std::any> {
            if (pybind11::isinstance<PyType>(obj)) {
                return std::any(obj.cast<CppType>());
            }
            return std::nullopt;
        });
    }

    /**
     * @brief Convert a std::any to a py::object.
     *
     * Looks up a converter by the std::any's type_index. If no converter is found,
     * returns py::none().
     *
     * @param value The std::any to convert.
     * @return The resulting py::object, or py::none() if no converter was found.
     */
    pybind11::object toPyObject(const std::any& value) const;

    /**
     * @brief Convert a py::object to a std::any.
     *
     * Tries each registered ToAny converter in order. The first converter that returns
     * a non-nullopt result is used. If no converter matches, the py::object itself is
     * stored in the std::any.
     *
     * @param obj The Python object to convert.
     * @return The resulting std::any.
     */
    std::any toAny(pybind11::handle obj) const;

private:
    std::unordered_map<std::type_index, ToPyFn> toPyConverters_;
    std::vector<ToAnyFn> toAnyConverters_;
};

}  // namespace inviwo
