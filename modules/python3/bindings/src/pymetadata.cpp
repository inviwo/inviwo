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

#include <inviwopy/pymetadata.h>

#include <pybind11/stl.h>

#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadatamap.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/metadata/positionmetadata.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

namespace py = pybind11;

namespace {

using StringVectorMetaData = MetaDataType<std::vector<std::string>>;
using StringMapMetaData = MetaDataType<std::map<std::string, std::string, std::less<>>>;

template <typename T>
void exposeMetaDataType(py::module& m, const char* name) {
    using MD = MetaDataType<T>;
    py::classh<MD, MetaData>(m, name)
        .def(py::init<>())
        .def(py::init<T>(), py::arg("value"))
        .def_property(
            "value", [](const MD& md) -> T { return md.get(); },
            [](MD& md, T value) { md.set(std::move(value)); })
        .def("__repr__", [name](const MD& md) {
            return fmt::format("{}({})", name, std::string{py::repr(py::cast(md.get()))});
        });
}

template <typename... Ts>
std::unique_ptr<MetaData> createFromBoundType(py::handle value) {
    std::unique_ptr<MetaData> result;
    (void)((py::isinstance<Ts>(value)
                ? (result = std::make_unique<MetaDataType<Ts>>(value.cast<Ts>()), true)
                : false) ||
           ...);
    return result;
}

std::unique_ptr<MetaData> createFromInteger(py::handle value) {
    const auto index = py::reinterpret_steal<py::int_>(PyNumber_Index(value.ptr()));
    if (!index) throw py::error_already_set();

    int overflow = 0;
    const auto v = PyLong_AsLongLongAndOverflow(index.ptr(), &overflow);
    if (v == -1 && PyErr_Occurred()) throw py::error_already_set();

    if (overflow == 0 && v >= std::numeric_limits<int>::min() &&
        v <= std::numeric_limits<int>::max()) {
        return std::make_unique<IntMetaData>(static_cast<int>(v));
    } else if (overflow > 0 || (overflow == 0 && v > 0)) {
        return std::make_unique<SizeMetaData>(index.cast<size_t>());
    } else {
        throw py::value_error(fmt::format("Integer {} is out of range for MetaData",
                                          py::str(index).cast<std::string>()));
    }
}

/*
 * Deduce a MetaData type from a python value. MetaData instances are cloned, python built-in types
 * are mapped to bool -> BoolMetaData, int -> IntMetaData (SizeMetaData if too large),
 * float -> DoubleMetaData, str -> StringMetaData, list[str] -> StringVectorMetaData, and
 * dict[str, str] -> StringMapMetaData. glm vectors and matrices map to their corresponding types.
 */
std::unique_ptr<MetaData> createMetaData(py::handle value) {
    if (py::isinstance<MetaData>(value)) {
        return std::unique_ptr<MetaData>(value.cast<const MetaData&>().clone());
    }
    if (PyBool_Check(value.ptr())) {
        return std::make_unique<BoolMetaData>(value.cast<bool>());
    }
    if (PyIndex_Check(value.ptr())) {
        return createFromInteger(value);
    }
    if (PyFloat_Check(value.ptr())) {
        return std::make_unique<DoubleMetaData>(value.cast<double>());
    }
    if (PyUnicode_Check(value.ptr())) {
        return std::make_unique<StringMetaData>(value.cast<std::string>());
    }
    // clang-format off
    if (auto md = createFromBoundType<ivec2, ivec3, ivec4,
                                      uvec2, uvec3, uvec4,
                                      size2_t, size3_t, size4_t,
                                      vec2, vec3, vec4, mat2, mat3, mat4,
                                      dvec2, dvec3, dvec4, dmat2, dmat3, dmat4,
                                      std::vector<std::string>>(value)) {
        return md;
    }
    // clang-format on
    if (py::isinstance<py::list>(value) || py::isinstance<py::tuple>(value)) {
        const auto seq = py::reinterpret_borrow<py::sequence>(value);
        std::vector<std::string> strings;
        strings.reserve(seq.size());
        for (auto item : seq) {
            if (!PyUnicode_Check(item.ptr())) {
                throw py::type_error(
                    "Only sequences of str can be converted to MetaData (StringVectorMetaData)");
            }
            strings.push_back(item.cast<std::string>());
        }
        return std::make_unique<StringVectorMetaData>(std::move(strings));
    }
    if (py::isinstance<py::dict>(value)) {
        std::map<std::string, std::string, std::less<>> strings;
        for (auto [k, v] : py::reinterpret_borrow<py::dict>(value)) {
            if (!PyUnicode_Check(k.ptr()) || !PyUnicode_Check(v.ptr())) {
                throw py::type_error(
                    "Only dicts of str to str can be converted to MetaData (StringMapMetaData)");
            }
            strings.emplace(k.cast<std::string>(), v.cast<std::string>());
        }
        return std::make_unique<StringMapMetaData>(std::move(strings));
    }
    if (PyNumber_Check(value.ptr()) && py::hasattr(value, "__float__")) {
        return std::make_unique<DoubleMetaData>(value.cast<double>());
    }

    throw py::type_error(fmt::format("Unable to deduce a MetaData type for a value of type '{}'",
                                     Py_TYPE(value.ptr())->tp_name));
}

/*
 * Returns the value of the MetaData if it has one, i.e. if it is a MetaDataType<T>, otherwise the
 * MetaData object itself.
 */
py::object toPython(MetaData* md, py::handle parent) {
    auto obj = py::cast(md, py::return_value_policy::reference_internal, parent);
    if (py::hasattr(obj, "value")) {
        return obj.attr("value");
    }
    return obj;
}

void setMetaData(MetaDataOwner& owner, std::string_view key, py::handle value) {
    auto* map = owner.getMetaDataMap();
    if (!py::isinstance<MetaData>(value)) {
        // Keep the type of any existing entry if the value can be converted to it
        if (auto* existing = map->get(key)) {
            auto pyExisting = py::cast(existing, py::return_value_policy::reference);
            if (py::hasattr(pyExisting, "value")) {
                try {
                    pyExisting.attr("value") = value;
                    return;
                } catch (py::error_already_set& e) {
                    if (!e.matches(PyExc_TypeError)) throw;
                }
            }
        }
    }
    map->add(key, createMetaData(value));
}

}  // namespace

void exposeMetaData(pybind11::module& m) {
    py::classh<MetaData>(m, "MetaData")
        .def_property_readonly("classIdentifier", &MetaData::getClassIdentifier)
        .def("clone", [](const MetaData& md) { return std::unique_ptr<MetaData>(md.clone()); })
        .def(
            "__eq__", [](const MetaData& lhs, const MetaData& rhs) { return lhs == rhs; },
            py::is_operator())
        .def(
            "__ne__", [](const MetaData& lhs, const MetaData& rhs) { return lhs != rhs; },
            py::is_operator())
        .def("__repr__", [](const MetaData& md) {
            return fmt::format("<MetaData: {}>", md.getClassIdentifier());
        });

    exposeMetaDataType<int>(m, "IntMetaData");
    exposeMetaDataType<ivec2>(m, "IntVec2MetaData");
    exposeMetaDataType<ivec3>(m, "IntVec3MetaData");
    exposeMetaDataType<ivec4>(m, "IntVec4MetaData");

    exposeMetaDataType<unsigned int>(m, "UIntMetaData");
    exposeMetaDataType<uvec2>(m, "UIntVec2MetaData");
    exposeMetaDataType<uvec3>(m, "UIntVec3MetaData");
    exposeMetaDataType<uvec4>(m, "UIntVec4MetaData");

    exposeMetaDataType<size_t>(m, "SizeMetaData");
    exposeMetaDataType<size2_t>(m, "Size2MetaData");
    exposeMetaDataType<size3_t>(m, "Size3MetaData");
    exposeMetaDataType<size4_t>(m, "Size4MetaData");

    exposeMetaDataType<float>(m, "FloatMetaData");
    exposeMetaDataType<vec2>(m, "FloatVec2MetaData");
    exposeMetaDataType<vec3>(m, "FloatVec3MetaData");
    exposeMetaDataType<vec4>(m, "FloatVec4MetaData");
    exposeMetaDataType<mat2>(m, "FloatMat2MetaData");
    exposeMetaDataType<mat3>(m, "FloatMat3MetaData");
    exposeMetaDataType<mat4>(m, "FloatMat4MetaData");

    exposeMetaDataType<double>(m, "DoubleMetaData");
    exposeMetaDataType<dvec2>(m, "DoubleVec2MetaData");
    exposeMetaDataType<dvec3>(m, "DoubleVec3MetaData");
    exposeMetaDataType<dvec4>(m, "DoubleVec4MetaData");
    exposeMetaDataType<dmat2>(m, "DoubleMat2MetaData");
    exposeMetaDataType<dmat3>(m, "DoubleMat3MetaData");
    exposeMetaDataType<dmat4>(m, "DoubleMat4MetaData");

    exposeMetaDataType<bool>(m, "BoolMetaData");
    exposeMetaDataType<std::string>(m, "StringMetaData");
    exposeMetaDataType<std::vector<std::string>>(m, "StringVectorMetaData");
    exposeMetaDataType<std::map<std::string, std::string, std::less<>>>(m, "StringMapMetaData");

    py::classh<PositionMetaData, IntVec2MetaData>(m, "PositionMetaData")
        .def(py::init<>())
        .def(py::init<int, int>(), py::arg("x"), py::arg("y"));

    py::classh<MetaDataMap>(m, "MetaDataMap")
        .def("keys",
             [](const MetaDataMap& map) {
                 py::list keys;
                 for (auto& key : map.getKeys()) keys.append(py::str(key));
                 return keys;
             })
        .def("items",
             [](py::handle self) {
                 auto& map = self.cast<MetaDataMap&>();
                 py::list items;
                 for (auto& key : map.getKeys()) {
                     items.append(py::make_tuple(
                         key, py::cast(map.get(key), py::return_value_policy::reference_internal,
                                       self)));
                 }
                 return items;
             })
        .def(
            "get",
            [](py::handle self, std::string_view key, py::object fallback) -> py::object {
                if (auto* md = self.cast<MetaDataMap&>().get(key)) {
                    return py::cast(md, py::return_value_policy::reference_internal, self);
                }
                return fallback;
            },
            py::arg("key"), py::arg("default") = py::none())
        .def("remove", &MetaDataMap::remove, py::arg("key"))
        .def("clear", &MetaDataMap::removeAll)
        .def("rename", &MetaDataMap::rename, py::arg("newKey"), py::arg("oldKey"))
        .def("empty", &MetaDataMap::empty)
        .def("__len__", [](const MetaDataMap& map) { return map.getKeys().size(); })
        .def("__bool__", [](const MetaDataMap& map) { return !map.empty(); })
        .def("__contains__", &MetaDataMap::contains)
        .def("__iter__",
             [](const MetaDataMap& map) {
                 py::list keys;
                 for (auto& key : map.getKeys()) keys.append(py::str(key));
                 return py::iter(keys);
             })
        .def(
            "__getitem__",
            [](MetaDataMap& map, std::string_view key) -> MetaData* {
                if (auto* md = map.get(key)) return md;
                throw py::key_error(std::string{key});
            },
            py::return_value_policy::reference_internal)
        .def("__setitem__",
             [](MetaDataMap& map, std::string_view key, const MetaData& md) {
                 map.add(key, std::unique_ptr<MetaData>(md.clone()));
             })
        .def("__delitem__",
             [](MetaDataMap& map, std::string_view key) {
                 if (!map.remove(key)) throw py::key_error(std::string{key});
             })
        .def(
            "__eq__", [](const MetaDataMap& lhs, const MetaDataMap& rhs) { return lhs == rhs; },
            py::is_operator())
        .def(
            "__ne__", [](const MetaDataMap& lhs, const MetaDataMap& rhs) { return lhs != rhs; },
            py::is_operator())
        .def("__repr__", [](MetaDataMap& map) {
            std::string items;
            for (auto& key : map.getKeys()) {
                if (!items.empty()) items += ", ";
                fmt::format_to(
                    std::back_inserter(items), "{}: {}", std::string{py::repr(py::str(key))},
                    std::string{
                        py::repr(py::cast(map.get(key), py::return_value_policy::reference))});
            }
            return fmt::format("<MetaDataMap: {{{}}}>", items);
        });

    py::classh<MetaDataOwner>(m, "MetaDataOwner", R"doc(
        Base class for objects that own MetaData, i.e. a key value store of typed values.

        Use ``getMetaData`` and ``setMetaData`` to access the values directly, or the ``metaData``
        property for access to the underlying MetaData objects.
        )doc")
        .def_property_readonly(
            "metaData", [](MetaDataOwner& owner) { return owner.getMetaDataMap(); },
            py::return_value_policy::reference_internal)
        .def("copyMetaDataFrom", &MetaDataOwner::copyMetaDataFrom, py::arg("src"))
        .def("copyMetaDataTo", &MetaDataOwner::copyMetaDataTo, py::arg("dst"))
        .def(
            "hasMetaData",
            [](const MetaDataOwner& owner, std::string_view key) { return owner.hasMetaData(key); },
            py::arg("key"))
        .def(
            "hasMetaData",
            [](const MetaDataOwner& owner, std::string_view key, const py::type& type) {
                if (const auto* md = owner.getMetaDataMap()->get(key)) {
                    return py::isinstance(py::cast(md, py::return_value_policy::reference), type);
                }
                return false;
            },
            py::arg("key"), py::arg("type"),
            "Check if there is MetaData for the given key that is an instance of the given type")
        .def(
            "getMetaData",
            [](py::handle self, std::string_view key, py::object fallback) -> py::object {
                if (auto* md = self.cast<MetaDataOwner&>().getMetaDataMap()->get(key)) {
                    return toPython(md, self);
                }
                return fallback;
            },
            py::arg("key"), py::arg("default") = py::none(),
            R"doc(
            Get the value of the MetaData for the given key, or the default if not found.
            For MetaData types without a value, the MetaData object itself is returned.
            )doc")
        .def("setMetaData", &setMetaData, py::arg("key"), py::arg("value"),
             R"doc(
            Set the MetaData for the given key.
            If value is a MetaData object, a copy of it is stored. Otherwise, if there already is
            MetaData for the key and the value is convertible to its type, that MetaData is updated.
            Else new MetaData is created with a type deduced from the value: bool -> BoolMetaData,
            int -> IntMetaData, float -> DoubleMetaData, str -> StringMetaData,
            list[str] -> StringVectorMetaData, dict[str, str] -> StringMapMetaData, and glm types
            to the corresponding MetaData type, i.e. vec3 -> FloatVec3MetaData.
            )doc")
        .def(
            "unsetMetaData",
            [](MetaDataOwner& owner, std::string_view key) {
                return owner.getMetaDataMap()->remove(key);
            },
            py::arg("key"), "Remove the MetaData for the given key");
}

}  // namespace inviwo
