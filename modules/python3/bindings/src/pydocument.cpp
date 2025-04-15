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

#include <inviwopy/pydocument.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/util/document.h>

#include <inviwo/core/algorithm/markdown.h>

#include <modules/python3/polymorphictypehooks.h>

#include <functional>

namespace inviwo {

void exposeDocument(pybind11::module& m) {
    auto doc = m.def_submodule("doc", "Inviwo document classes");

    namespace py = pybind11;

    py::enum_<Document::ElementType>(doc, "ElementType")
        .value("Node", Document::ElementType::Node)
        .value("Text", Document::ElementType::Text);

    py::classh<Document::Element>(doc, "Element")
        .def(py::init<Document::ElementType, std::string_view>(), py::arg("type"),
             py::arg("content"))
        .def(py::init<std::string_view, std::string_view, const UnorderedStringMap<std::string>&>(),
             py::arg("name"), py::arg("content") = std::string_view{""},
             py::arg("attributes") = UnorderedStringMap<std::string>{})
        .def_property(
            "name", [](Document::Element& d) { return d.name(); },
            [](Document::Element& d, std::string_view val) { return d.name() = val; })
        .def_property(
            "content", [](Document::Element& d) { return d.content(); },
            [](Document::Element& d, std::string_view val) { return d.content() = val; })
        .def_property_readonly("attributes", [](Document::Element& d) { return d.attributes(); })
        .def_property_readonly("type", &Document::Element::type)
        .def("isText", &Document::Element::isText)
        .def("isNode", &Document::Element::isNode)
        .def("emptyTag", &Document::Element::emptyTag)
        .def("noIndent", &Document::Element::noIndent);

    py::classh<Document::PathComponent>(doc, "PathComponent")
        .def(py::init<int>(), py::arg("index"))
        .def(py::init<std::string_view>(), py::arg("name"))
        .def(py::init<const UnorderedStringMap<std::string>&>(), py::arg("attributes"))
        .def(py::init<std::string_view, const UnorderedStringMap<std::string>&>(), py::arg("name"),
             py::arg("attributes"))
        .def_static("first", &Document::PathComponent::first)
        .def_static("last", &Document::PathComponent::last)
        .def_static("end", &Document::PathComponent::end);

    py::classh<Document::DocumentHandle>(doc, "DocumentHandle")
        .def("get", &Document::DocumentHandle::get)
        .def("insert",
             static_cast<Document::DocumentHandle (Document::DocumentHandle::*)(
                 Document::PathComponent, std::string_view, std::string_view,
                 const UnorderedStringMap<std::string>&)>(&Document::DocumentHandle::insert))
        .def("append",
             static_cast<Document::DocumentHandle (Document::DocumentHandle::*)(
                 std::string_view, std::string_view, const UnorderedStringMap<std::string>&)>(
                 &Document::DocumentHandle::append))
        .def("insert", static_cast<Document::DocumentHandle (Document::DocumentHandle::*)(
                           Document::PathComponent, Document)>(&Document::DocumentHandle::insert))
        .def("insertText", &Document::DocumentHandle::insertText)
        .def("appendText", &Document::DocumentHandle::appendText)
        .def("append",
             static_cast<Document::DocumentHandle (Document::DocumentHandle::*)(Document)>(
                 &Document::DocumentHandle::append))
        .def_property_readonly("element", [](Document::DocumentHandle& d) { return d.element(); });

    py::classh<Document>(doc, "Document")
        .def(py::init<>())
        .def(py::init<std::string_view>())
        .def("str", &Document::str)
        .def("empty", &Document::empty)

        .def("handle", &Document::handle)
        .def("get", &Document::get)
        .def("insert", static_cast<Document::DocumentHandle (Document::*)(
                           Document::PathComponent, std::string_view, std::string_view,
                           const UnorderedStringMap<std::string>&)>(&Document::insert))
        .def("append",
             static_cast<Document::DocumentHandle (Document::*)(
                 std::string_view, std::string_view, const UnorderedStringMap<std::string>&)>(
                 &Document::append))
        .def("insert",
             static_cast<Document::DocumentHandle (Document::*)(Document::PathComponent, Document)>(
                 &Document::insert))
        .def("insertText", &Document::insertText)
        .def("appendText", &Document::appendText)
        .def("append",
             static_cast<Document::DocumentHandle (Document::*)(Document)>(&Document::append))
        .def("visit",
             [](Document& d,
                std::function<void(Document::Element*, std::vector<Document::Element*>&)> before,
                std::function<void(Document::Element*, std::vector<Document::Element*>&)> after)
                 -> void { d.visit(before, after); });

    m.def("md2doc", &util::md2doc);
    m.def("unindentMd2doc", &util::unindentMd2doc);
}

}  // namespace inviwo
