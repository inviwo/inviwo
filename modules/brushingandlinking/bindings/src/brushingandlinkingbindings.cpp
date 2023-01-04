/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/embed.h>
#include <warn/pop>

#include <modules/python3/pybindflags.h>

#include <modules/brushingandlinking/datastructures/brushingaction.h>
#include <modules/brushingandlinking/datastructures/indexlist.h>
#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace py = pybind11;

namespace inviwo {

void exposeBnL(py::module& m) {

    py::module::import("inviwopy");

    m.doc() = R"doc(
        BrushingAndLinking Module API
    
        .. rubric:: Modules
        
        .. autosummary::
            :toctree: .
            
        )doc";

    py::enum_<BrushingAction>(m, "BrushingAction")
        .value("Filter", BrushingAction::Filter)
        .value("Select", BrushingAction::Select)
        .value("Highlight", BrushingAction::Highlight);

    auto brushingModification = py::enum_<BrushingModification>(m, "BrushingModification")
                                    .value("Filtered", BrushingModification::Filtered)
                                    .value("Selected", BrushingModification::Selected)
                                    .value("Highlighted", BrushingModification::Highlighted);
    brushingModification.def_static("fromAction", &fromAction);
    exposeFlags<BrushingModification>(m, brushingModification, "BrushingModifications");

    py::class_<BrushingTarget>(m, "BrushingTarget")
        .def(py::init<>())
        .def(py::init<std::string_view>())
        .def(py::init<const BrushingTarget&>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def(py::self <= py::self)
        .def("__str__", &BrushingTarget::getString)
        .def_property_readonly_static("Row", [](py::object) { return BrushingTarget::Row; })
        .def_property_readonly_static("Column", [](py::object) { return BrushingTarget::Column; });

    py::class_<IndexList>(m, "IndexList")
        .def(py::init<>())
        .def("empty", &IndexList::empty)
        .def("size", &IndexList::size)
        .def("clear", &IndexList::clear)
        .def("set", &IndexList::set)
        .def("contains", &IndexList::contains)
        .def("getIndices", &IndexList::getIndices)
        .def("removeSource", &IndexList::removeSources);

    py::class_<BrushingTargetsInvalidationLevel, Inport>{m, "BrushingTargetsInvalidationLevel"}
        .def(py::init<BrushingModifications, InvalidationLevel>(), py::arg("mods"),
             py::arg("invalidationLevel"))
        .def(py::init<std::vector<BrushingTarget>, BrushingModifications, InvalidationLevel>(),
             py::arg("targers"), py::arg("mods"), py::arg("invalidationLevel"))
        .def("contains",
             static_cast<bool (BrushingTargetsInvalidationLevel::*)(const BrushingTarget&) const>(
                 &BrushingTargetsInvalidationLevel::contains))
        .def("contains", static_cast<bool (BrushingTargetsInvalidationLevel::*)(
                             const BrushingTarget&, BrushingModifications) const>(
                             &BrushingTargetsInvalidationLevel::contains))
        .def_readwrite("targets", &BrushingTargetsInvalidationLevel::targets)
        .def_readwrite("modifications", &BrushingTargetsInvalidationLevel::modifications)
        .def_readwrite("invalidationLevel", &BrushingTargetsInvalidationLevel::invalidationLevel);

    py::class_<BrushingAndLinkingInport, Inport>{m, "BrushingAndLinkingInport"}
        .def(py::init([](std::string_view identifier) {
                 return std::make_shared<BrushingAndLinkingInport>(identifier);
             }),
             py::arg("identifier"))
        .def(py::init<std::string_view, std::vector<BrushingTargetsInvalidationLevel>>(),
             py::arg("identifier"), py::arg("invalidationLevels"))
        .def(py::init([](std::string_view identifier, Document help) {
                 return std::make_shared<BrushingAndLinkingInport>(identifier, help);
             }),
             py::arg("identifier"), py::arg("help"))
        .def(py::init<std::string_view, Document, std::vector<BrushingTargetsInvalidationLevel>>(),
             py::arg("identifier"), py::arg("help"), py::arg("invalidationLevels"))

        .def("getModifiedActions", &BrushingAndLinkingInport::getModifiedActions)
        .def("isFilteringModified", &BrushingAndLinkingInport::isFilteringModified)
        .def("isSelectionModified", &BrushingAndLinkingInport::isSelectionModified)
        .def("isHighlightModified", &BrushingAndLinkingInport::isHighlightModified)
        .def("brush", &BrushingAndLinkingInport::brush, py::arg("action"), py::arg("target"),
             py::arg("indices"), py::arg("source") = std::string_view{})
        .def("filter", &BrushingAndLinkingInport::filter, py::arg("source"), py::arg("indices"),
             py::arg("target") = BrushingTarget::Row)
        .def("select", &BrushingAndLinkingInport::select, py::arg("indices"),
             py::arg("target") = BrushingTarget::Row)
        .def("highlight", &BrushingAndLinkingInport::highlight, py::arg("indices"),
             py::arg("target") = BrushingTarget::Row)
        .def("isFiltered", &BrushingAndLinkingInport::isFiltered, py::arg("index"),
             py::arg("target") = BrushingTarget::Row)
        .def("isSelected", &BrushingAndLinkingInport::isSelected, py::arg("index"),
             py::arg("target") = BrushingTarget::Row)
        .def("isHighlighted", &BrushingAndLinkingInport::isHighlighted, py::arg("index"),
             py::arg("target") = BrushingTarget::Row)
        .def("getIndices", &BrushingAndLinkingInport::getIndices, py::return_value_policy::copy,
             py::arg("action"), py::arg("target") = BrushingTarget::Row)
        .def("getFilteredIndices", &BrushingAndLinkingInport::getFilteredIndices,
             py::return_value_policy::copy, py::arg("target") = BrushingTarget::Row)
        .def("getSelectedIndices", &BrushingAndLinkingInport::getSelectedIndices,
             py::return_value_policy::copy, py::arg("target") = BrushingTarget::Row)
        .def("getHighlightedIndices", &BrushingAndLinkingInport::getHighlightedIndices,
             py::return_value_policy::copy, py::arg("target") = BrushingTarget::Row)
        .def("getInvalidationLevels", &BrushingAndLinkingInport::getInvalidationLevels)
        .def("setInvalidationLevels", &BrushingAndLinkingInport::setInvalidationLevels)
        .def("getManager",
             static_cast<BrushingAndLinkingManager& (BrushingAndLinkingInport::*)()>(
                 &BrushingAndLinkingInport::getManager),
             py::return_value_policy::reference);

    py::class_<BrushingAndLinkingOutport, Outport>{m, "BrushingAndLinkingOutport"}

        .def(py::init<std::string_view, Document>(), py::arg("identifier"), py::arg("help"))
        .def("getManager",
             static_cast<BrushingAndLinkingManager& (BrushingAndLinkingOutport::*)()>(
                 &BrushingAndLinkingOutport::getManager),
             py::return_value_policy::reference)
        .def("getInvalidationLevels", &BrushingAndLinkingOutport::getInvalidationLevels)
        .def("setInvalidationLevels", &BrushingAndLinkingOutport::setInvalidationLevels);

    py::class_<BrushingAndLinkingManager>{m, "BrushingAndLinkingManager"}
        .def(py::init([](BrushingAndLinkingInport* inport) {
                 return std::make_shared<BrushingAndLinkingManager>(inport);
             }),
             py::arg("inport"))
        .def(py::init<BrushingAndLinkingInport*, std::vector<BrushingTargetsInvalidationLevel>>(),
             py::arg("inport"), py::arg("invalidationLevels"))
        .def(py::init([](BrushingAndLinkingOutport* outport) {
                 return std::make_shared<BrushingAndLinkingManager>(outport);
             }),
             py::arg("outport"))
        .def(py::init<BrushingAndLinkingOutport*, std::vector<BrushingTargetsInvalidationLevel>>(),
             py::arg("outport"), py::arg("invalidationLevels"))
        .def("brush", &BrushingAndLinkingManager::brush, py::arg("action"), py::arg("target"),
             py::arg("indices"), py::arg("source") = std::string_view{})
        .def("filter", &BrushingAndLinkingManager::filter, py::arg("source"), py::arg("indices"),
             py::arg("target") = BrushingTarget::Row)
        .def("select", &BrushingAndLinkingManager::select, py::arg("indices"),
             py::arg("target") = BrushingTarget::Row)
        .def("highlight", &BrushingAndLinkingManager::highlight, py::arg("indices"),
             py::arg("target") = BrushingTarget::Row)
        .def("isModified", &BrushingAndLinkingManager::isModified)
        .def("getModifiedActions", &BrushingAndLinkingManager::getModifiedActions)
        .def("isFilteringModified", &BrushingAndLinkingManager::isFilteringModified,
             py::arg("target") = BrushingTarget::Row)
        .def("isSelectionModified", &BrushingAndLinkingManager::isSelectionModified,
             py::arg("target") = BrushingTarget::Row)
        .def("isHighlightModified", &BrushingAndLinkingManager::isHighlightModified,
             py::arg("target") = BrushingTarget::Row)
        .def("getModifiedTargets", &BrushingAndLinkingManager::getModifiedTargets)
        .def("isTargetModified",
             static_cast<bool (BrushingAndLinkingManager::*)(BrushingTarget, BrushingModifications)
                             const>(&BrushingAndLinkingManager::isTargetModified),
             py::arg("target"), py::arg("modifications"))
        .def("isTargetModified",
             static_cast<bool (BrushingAndLinkingManager::*)(BrushingTarget, BrushingAction) const>(
                 &BrushingAndLinkingManager::isTargetModified),
             py::arg("target"), py::arg("action"))
        .def("hasIndices", &BrushingAndLinkingManager::hasIndices, py::arg("action"),
             py::arg("target") = BrushingTarget::Row)
        .def("getIndices", &BrushingAndLinkingManager::getIndices, py::return_value_policy::copy,
             py::arg("action"), py::arg("target") = BrushingTarget::Row)
        .def("getNumber", &BrushingAndLinkingManager::getNumber, py::arg("action"),
             py::arg("target") = BrushingTarget::Row)
        .def("getNumberOfFiltered", &BrushingAndLinkingManager::getNumberOfFiltered,
             py::arg("target") = BrushingTarget::Row)
        .def("getNumberOfSelected", &BrushingAndLinkingManager::getNumberOfSelected,
             py::arg("target") = BrushingTarget::Row)
        .def("getNumberOfHighlighted", &BrushingAndLinkingManager::getNumberOfHighlighted,
             py::arg("target") = BrushingTarget::Row)
        .def("clearIndices", &BrushingAndLinkingManager::clearIndices, py::arg("action"),
             py::arg("target"))
        .def("clearSelected", &BrushingAndLinkingManager::clearSelected,
             py::arg("target") = BrushingTarget::Row)
        .def("clearHighlighted", &BrushingAndLinkingManager::clearHighlighted,
             py::arg("target") = BrushingTarget::Row)
        .def("contains", &BrushingAndLinkingManager::contains, py::arg("index"), py::arg("action"),
             py::arg("target") = BrushingTarget::Row)
        .def("isFiltered", &BrushingAndLinkingManager::isFiltered, py::arg("index"),
             py::arg("target") = BrushingTarget::Row)
        .def("isSelected", &BrushingAndLinkingManager::isSelected, py::arg("index"),
             py::arg("target") = BrushingTarget::Row)
        .def("isHighlighted", &BrushingAndLinkingManager::isHighlighted, py::arg("index"),
             py::arg("target") = BrushingTarget::Row)
        .def("getTargets",
             static_cast<std::vector<std::pair<BrushingAction, std::vector<BrushingTarget>>> (
                 BrushingAndLinkingManager::*)() const>(&BrushingAndLinkingManager::getTargets))
        .def("getTargets",
             static_cast<std::vector<BrushingTarget> (BrushingAndLinkingManager::*)(BrushingAction)
                             const>(&BrushingAndLinkingManager::getTargets),
             py::arg("action"))
        .def("getFilteredIndices", &BrushingAndLinkingManager::getFilteredIndices,
             py::return_value_policy::copy, py::arg("target") = BrushingTarget::Row)
        .def("getSelectedIndices", &BrushingAndLinkingManager::getSelectedIndices,
             py::return_value_policy::copy, py::arg("target") = BrushingTarget::Row)
        .def("getHighlightedIndices", &BrushingAndLinkingManager::getHighlightedIndices,
             py::return_value_policy::copy, py::arg("target") = BrushingTarget::Row)
        .def("setParent", &BrushingAndLinkingManager::setParent, py::arg("parent"))
        .def("onBrush", &BrushingAndLinkingManager::onBrush, py::arg("callback"))
        .def("getInvalidationLevel",
             static_cast<InvalidationLevel (BrushingAndLinkingManager::*)() const>(
                 &BrushingAndLinkingManager::getInvalidationLevel))
        .def("getInvalidationLevels", &BrushingAndLinkingManager::getInvalidationLevels)
        .def("setInvalidationLevels", &BrushingAndLinkingManager::setInvalidationLevels)
        .def("propagateModifications", &BrushingAndLinkingManager::propagateModifications)
        .def("clearModifications", &BrushingAndLinkingManager::clearModifications);
}

}  // namespace inviwo

#ifdef INVIWO_ALL_DYN_LINK
PYBIND11_MODULE(ivwbnl, m) { inviwo::exposeBnL(m); }
#else
PYBIND11_EMBEDDED_MODULE(ivwbnl, m) { inviwo::exposeBnL(m); }
#endif
