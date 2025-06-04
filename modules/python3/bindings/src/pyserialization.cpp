/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <inviwopy/pyserialization.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>

namespace inviwo {

namespace py = pybind11;

void exposeSerialization(pybind11::module& m) {

    py::enum_<SerializationTarget>(m, "SerializationTarget")
        .value("Node", SerializationTarget::Node)
        .value("Attribute", SerializationTarget::Attribute);

    py::classh<Serializer>(m, "Serializer")
        .def(py::init<const std::filesystem::path&, std::string_view>(), py::arg("filename"),
             py::arg("rootElement") = SerializeConstants::InviwoWorkspace)
        .def("getFileName", &Serializer::getFileName)
        .def("getFileDir", &Serializer::getFileDir)
        .def("writeFile", [](Serializer& s) { s.writeFile(); })
        .def(
            "serialize",
            [](Serializer& s, std::string_view key, std::string_view data,
               SerializationTarget target) { s.serialize(key, data, target); },
            py::arg("key"), py::arg("data"), py::arg("target") = SerializationTarget::Node)
        .def(
            "serialize",
            [](Serializer& s, std::string_view key, const std::filesystem::path& data,
               SerializationTarget target) { s.serialize(key, data, target); },
            py::arg("key"), py::arg("data"), py::arg("target") = SerializationTarget::Node)

        .def(
            "serialize",
            [](Serializer& s, std::string_view key, double data, SerializationTarget target) {
                s.serialize(key, data, target);
            },
            py::arg("key"), py::arg("data"), py::arg("target") = SerializationTarget::Node)

        .def(
            "serialize",
            [](Serializer& s, std::string_view key, py::typing::Iterable<double> data,
               std::string_view itemKey) {
                std::vector<double> vData;
                for (auto i : data) {
                    vData.emplace_back(i.cast<double>());
                }
                s.serialize(key, vData, itemKey);
            },
            py::arg("key"), py::arg("data"), py::arg("itemKey") = "item")

        .def(
            "serialize",
            [](Serializer& s, std::string_view key, int data, SerializationTarget target) {
                s.serialize(key, data, target);
            },
            py::arg("key"), py::arg("data"), py::arg("target") = SerializationTarget::Node)

        .def(
            "serialize",
            [](Serializer& s, std::string_view key, py::typing::Iterable<int> data,
               std::string_view itemKey) {
                std::vector<int> vData;
                for (auto i : data) {
                    vData.emplace_back(i.cast<int>());
                }
                s.serialize(key, vData, itemKey);
            },
            py::arg("key"), py::arg("data"), py::arg("itemKey") = "item")

        .def(
            "serialize",
            [](Serializer& s, std::string_view key, py::typing::Iterable<std::string> data,
               std::string_view itemKey) {
                std::vector<std::string> vData;
                for (auto i : data) {
                    vData.emplace_back(i.cast<std::string>());
                }
                s.serialize(key, vData, itemKey);
            },
            py::arg("key"), py::arg("data"), py::arg("itemKey") = "item");

    py::classh<Deserializer>(m, "Deserializer")
        .def(py::init<const std::filesystem::path&, std::string_view>(), py::arg("filename"),
             py::arg("rootElement") = SerializeConstants::InviwoWorkspace)
        .def("getFileName", &Deserializer::getFileName)
        .def("getFileDir", &Deserializer::getFileDir)

        .def(
            "deserializeString",
            [](Deserializer& d, std::string_view key, SerializationTarget target) {
                std::string data;
                d.deserialize(key, data, target);
                return data;
            },
            py::arg("key"), py::arg("target") = SerializationTarget::Node)
        .def(
            "deserializePath",
            [](Deserializer& d, std::string_view key, SerializationTarget target) {
                std::filesystem::path data;
                d.deserialize(key, data, target);
                return data;
            },
            py::arg("key"), py::arg("target") = SerializationTarget::Node)

        .def(
            "deserializeDouble",
            [](Deserializer& d, std::string_view key, SerializationTarget target) {
                double data;
                d.deserialize(key, data, target);
                return data;
            },
            py::arg("key"), py::arg("target") = SerializationTarget::Node)

        .def(
            "deserializeDoubleVector",
            [](Deserializer& d, std::string_view key, std::string_view itemKey) {
                std::vector<double> vData;
                d.deserialize(key, vData, itemKey);
                return vData;
            },
            py::arg("key"), py::arg("itemKey") = "item")

        .def(
            "deserializeInt",
            [](Deserializer& d, std::string_view key, SerializationTarget target) {
                int data;
                d.deserialize(key, data, target);
                return data;
            },
            py::arg("key"), py::arg("target") = SerializationTarget::Node)

        .def(
            "deserializeIntVector",
            [](Deserializer& d, std::string_view key, std::string_view itemKey) {
                std::vector<int> vData;
                d.deserialize(key, vData, itemKey);
                return vData;
            },
            py::arg("key"), py::arg("itemKey") = "item")
        .def(
            "deserializeStringVector",
            [](Deserializer& d, std::string_view key, std::string_view itemKey) {
                std::vector<std::string> vData;
                d.deserialize(key, vData, itemKey);
                return vData;
            },
            py::arg("key"), py::arg("itemKey") = "item");
}

}  // namespace inviwo
