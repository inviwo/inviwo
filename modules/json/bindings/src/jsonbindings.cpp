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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <modules/python3/pybindmodule.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/util/exception.h>

#include <modules/json/json.h>
#include <modules/json/jsonmodule.h>
#include <modules/json/jsonconverterregistry.h>
#include <modules/json/jsonpropertyconverter.h>
#include <modules/json/jsoninportconverter.h>
#include <modules/json/jsonoutportconverter.h>

#include <string>

namespace py = pybind11;

namespace inviwo {

namespace {

// Convert a nlohmann::json value to a Python object (dict/list/str/...).
// Done by serializing to a string and using Python's built in json module.
py::object jsonToPyObject(const json& j) {
    auto pyJson = py::module_::import("json");
    return pyJson.attr("loads")(j.dump());
}

// Convert a Python object to a nlohmann::json value via Python's json module.
json pyObjectToJson(const py::handle& obj) {
    auto pyJson = py::module_::import("json");
    const auto s = pyJson.attr("dumps")(obj).cast<std::string>();
    return json::parse(s);
}

JSONModule& getJSONModule() {
    auto* app = InviwoApplication::getPtr();
    if (!app) {
        throw Exception("InviwoApplication is not initialized");
    }
    return util::getModuleByTypeOrThrow<JSONModule>(app);
}

template <typename Base>
void exposeRegistry(py::module& m, const char* name) {
    py::classh<JSONConverterRegistry<Base>>(m, name)
        .def(
            "hasConverter",
            [](const JSONConverterRegistry<Base>& self, std::string_view classIdentifier) {
                return self.getFactoryObject(classIdentifier) != nullptr;
            },
            py::arg("classIdentifier"))
        .def(
            "toJson",
            [](const JSONConverterRegistry<Base>& self, const Base& obj) {
                return jsonToPyObject(self.toJSON(obj));
            },
            py::arg("obj"))
        .def(
            "fromJson",
            [](const JSONConverterRegistry<Base>& self, const py::object& data, Base& obj) {
                self.fromJSON(pyObjectToJson(data), obj);
            },
            py::arg("data"), py::arg("obj"));
}

}  // namespace

void exposeJSON(py::module& m) {
    py::module_::import("inviwopy");

    m.doc() = R"doc(
        JSON Module API

        Bindings for converting Inviwo Properties, Inports, and Outports to and from
        JSON-compatible Python objects (dicts, lists, etc). The conversions go through
        the converter registries owned by the :class:`JSONModule`.

        Example::

            import inviwopy
            import ivwjson

            prop = inviwopy.properties.FloatProperty("threshold", "Threshold", 0.5)
            data = ivwjson.toJson(prop)        # -> Python dict
            ivwjson.fromJson(prop, data)       # update prop from a dict
        )doc";

    exposeRegistry<Property>(m, "JSONPropertyConverter");
    exposeRegistry<Inport>(m, "JSONInportConverter");
    exposeRegistry<Outport>(m, "JSONOutportConverter");

    m.def(
        "getJSONPropertyConverter",
        []() -> const JSONPropertyConverter& { return getJSONModule().getJSONPropertyConverter(); },
        py::return_value_policy::reference,
        "Get the global JSONPropertyConverter from the JSON module.");
    m.def(
        "getJSONInportConverter",
        []() -> const JSONInportConverter& { return getJSONModule().getJSONInportConverter(); },
        py::return_value_policy::reference,
        "Get the global JSONInportConverter from the JSON module.");
    m.def(
        "getJSONOutportConverter",
        []() -> const JSONOutportConverter& { return getJSONModule().getJSONOutportConverter(); },
        py::return_value_policy::reference,
        "Get the global JSONOutportConverter from the JSON module.");

    // Convenience top-level functions, dispatching to the appropriate registry.
    m.def(
        "toJson",
        [](Property& prop) { return jsonToPyObject(getJSONModule().getJSONPropertyConverter().toJSON(prop)); },
        py::arg("property"), "Convert a Property to a JSON-compatible Python object.");
    m.def(
        "toJson",
        [](Inport& port) { return jsonToPyObject(getJSONModule().getJSONInportConverter().toJSON(port)); },
        py::arg("inport"), "Convert an Inport to a JSON-compatible Python object.");
    m.def(
        "toJson",
        [](Outport& port) { return jsonToPyObject(getJSONModule().getJSONOutportConverter().toJSON(port)); },
        py::arg("outport"), "Convert an Outport to a JSON-compatible Python object.");

    m.def(
        "fromJson",
        [](Property& prop, const py::object& data) {
            getJSONModule().getJSONPropertyConverter().fromJSON(pyObjectToJson(data), prop);
        },
        py::arg("property"), py::arg("data"),
        "Update a Property from a JSON-compatible Python object.");
    m.def(
        "fromJson",
        [](Inport& port, const py::object& data) {
            getJSONModule().getJSONInportConverter().fromJSON(pyObjectToJson(data), port);
        },
        py::arg("inport"), py::arg("data"),
        "Update an Inport from a JSON-compatible Python object.");
    m.def(
        "fromJson",
        [](Outport& port, const py::object& data) {
            getJSONModule().getJSONOutportConverter().fromJSON(pyObjectToJson(data), port);
        },
        py::arg("outport"), py::arg("data"),
        "Update an Outport from a JSON-compatible Python object.");
}

}  // namespace inviwo

INVIWO_PYBIND_MODULE(ivwjson, m) { inviwo::exposeJSON(m); }
