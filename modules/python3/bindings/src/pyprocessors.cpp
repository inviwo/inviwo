/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwopy/pyprocessors.h>

#include <inviwopy/inviwopy.h>
#include <inviwopy/vectoridentifierwrapper.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/metadata/processormetadata.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>

#include <modules/python3/processors/pythonscriptprocessor.h>

namespace inviwo {

class ProcessorTrampoline : public Processor {
public:
    /* Inherit the constructors */
    using Processor::Processor;

    /* Trampoline (need one for each virtual function) */
    virtual void initializeResources() override {
        pybind11::gil_scoped_acquire gil;
        if (auto overload = pybind11::get_overload(static_cast<const Processor *>(this),
                                                   "initializeResources")) {
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::overload_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(overload(), caster);
            } else {
                return pybind11::detail::cast_safe<void>(overload());
            }
        }
        Processor::initializeResources();
    }
    virtual void process() override {
        pybind11::gil_scoped_acquire gil;
        if (auto overload =
                pybind11::get_overload(static_cast<const Processor *>(this), "process")) {
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::overload_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(overload(), caster);
            } else {
                return pybind11::detail::cast_safe<void>(overload());
            }
        }
        Processor::process();
    }
    virtual const ProcessorInfo getProcessorInfo() const override {
        pybind11::gil_scoped_acquire gil;
        if (auto overload =
                pybind11::get_overload(static_cast<const Processor *>(this), "getProcessorInfo")) {
            if (pybind11::detail::cast_is_temporary_value_reference<const ProcessorInfo>::value) {
                static pybind11::detail::overload_caster_t<const ProcessorInfo> caster;
                return pybind11::detail::cast_ref<const ProcessorInfo>(overload(), caster);
            } else {
                return pybind11::detail::cast_safe<const ProcessorInfo>(overload());
            }
        }
        pybind11::pybind11_fail("Tried to call pure virtual function Processor::getProcessorInfo");
    }
};

/*
    template <>
      struct ProcessorTraits<ProcessorTrampoline> {
         static ProcessorInfo getProcessorInfo() {
            return generateMyProcessorInfo<T>();
         }
      };
      */

void exposeProcessors(pybind11::module &m) {
    namespace py = pybind11;

    py::enum_<CodeState>(m, "CodeState")
        .value("Broken", CodeState::Broken)
        .value("Experimental", CodeState::Experimental)
        .value("Stable", CodeState::Stable);

    py::class_<Tag>(m, "Tag")
        .def(py::init())
        .def(py::init<std::string>())
        .def(py::init<Tag>())
        .def("getString", &Tag::getString);

    py::class_<Tags>(m, "Tags")
        .def(py::init())
        .def(py::init<std::string>())
        .def(py::init<Tags>())
        .def("addTag", &Tags::addTag)
        .def("addTags", &Tags::addTags)
        .def("size", &Tags::size)
        .def("empty", &Tags::empty)
        .def("getString", &Tags::getString)
        .def("getMatches", &Tags::getMatches)
        .def_readwrite("tags", &Tags::tags_)
        .def(py::self == py::self)
        .def(py::self < py::self);

    py::class_<ProcessorInfo>(m, "ProcessorInfo")
        .def(py::init<std::string, std::string, std::string, CodeState, Tags, bool>())
        .def_readonly("classIdentifier", &ProcessorInfo::classIdentifier)
        .def_readonly("displayName", &ProcessorInfo::displayName)
        .def_readonly("category", &ProcessorInfo::category)
        .def_readonly("codeState", &ProcessorInfo::codeState)
        .def_readonly("tags", &ProcessorInfo::tags)
        .def_readonly("visible", &ProcessorInfo::visible);

    py::class_<ProcessorFactory>(m, "ProcessorFactory")
        .def("hasKey", [](ProcessorFactory *pf, std::string key) { return pf->hasKey(key); })
        .def_property_readonly("keys", [](ProcessorFactory *pf) { return pf->getKeys(); })
        .def("create",
             [](ProcessorFactory *pf, std::string key) { return pf->create(key).release(); })
        .def("create", [](ProcessorFactory *pf, std::string key, ivec2 pos) {
            auto p = pf->create(key);
            if (!p) {
                throw py::key_error("failed to create processor of type '" + key + "'");
            }
            p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                ->setPosition(pos);

            return p.release();
        });

    py::class_<ProcessorWidget>(m, "ProcessorWidget")
        .def_property("visibility", &ProcessorWidget::isVisible, &ProcessorWidget::setVisible)
        .def_property("dimensions", &ProcessorWidget::getDimensions,
                      &ProcessorWidget::setDimensions)
        .def_property("position", &ProcessorWidget::getPosition, &ProcessorWidget::setPosition)
        .def("show", &ProcessorWidget::show)
        .def("hide", &ProcessorWidget::hide);

    py::class_<ProcessorMetaData>(m, "ProcessorMetaData")
        .def_property("position", &ProcessorMetaData::getPosition, &ProcessorMetaData::setPosition)
        .def_property("selected", &ProcessorMetaData::isSelected, &ProcessorMetaData::setSelected)
        .def_property("visible", &ProcessorMetaData::isVisible, &ProcessorMetaData::setVisible);

    using InportVecWrapper = VectorIdentifierWrapper<std::vector<Inport *>>;
    exposeVectorIdentifierWrapper<std::vector<Inport *>>(m, "InportVectorWrapper");

    using OutportVecWrapper = VectorIdentifierWrapper<std::vector<Outport *>>;
    exposeVectorIdentifierWrapper<std::vector<Outport *>>(m, "OutportVectorWrapper");

    py::class_<Processor, ProcessorTrampoline, PropertyOwner, ProcessorPtr<Processor>>(
        m, "Processor", py::dynamic_attr{})
        .def(py::init<const std::string &, const std::string &>())
        .def_property_readonly("classIdentifier", &Processor::getClassIdentifier)
        .def_property("displayName", &Processor::getDisplayName, &Processor::setDisplayName)
        .def("getProcessorInfo", &Processor::getProcessorInfo)
        .def_property_readonly("category", &Processor::getCategory)
        .def_property_readonly("codeState", &Processor::getCodeState)
        .def_property_readonly("tags", &Processor::getTags)
        .def_property("identifier", &Processor::getIdentifier, &Processor::setIdentifier)
        .def("hasProcessorWidget", &Processor::hasProcessorWidget)
        .def_property_readonly("widget", &Processor::getProcessorWidget)
        .def_property_readonly("network", &Processor::getNetwork,
                               py::return_value_policy::reference)
        .def_property_readonly("inports",
                               [](Processor *p) { return InportVecWrapper(p->getInports()); })
        .def_property_readonly("outports",
                               [](Processor *p) { return OutportVecWrapper(p->getOutports()); })
        .def("getPort", &Processor::getPort, py::return_value_policy::reference)
        .def("getInport", &Processor::getInport, py::return_value_policy::reference)
        .def("getOutport", &Processor::getOutport, py::return_value_policy::reference)
        .def("addInport",
             [](Processor &p, Inport *port, const std::string &group, bool owner) {
                 if (owner) {
                     p.addPort(std::unique_ptr<Inport>(port), group);
                 } else {
                     p.addPort(*port, group);
                 }
             },
             py::arg("inport"), py::arg("group") = "default", py::arg("owner") = true,
             py::keep_alive<1, 2>{})
        .def("addOutport",
             [](Processor &p, Outport *port, const std::string &group, bool owner) {
                 if (owner) {
                     p.addPort(std::unique_ptr<Outport>(port), group);
                 } else {
                     p.addPort(*port, group);
                 }
             },
             py::arg("outport"), py::arg("group") = "default", py::arg("owner") = true,
             py::keep_alive<1, 2>{})
        .def("removeInport", [](Processor &p, Inport *port) { return p.removePort(port); })
        .def("removeOutport", [](Processor &p, Outport *port) { return p.removePort(port); })

        .def_property_readonly(
            "meta",
            [](Processor *p) {
                return p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            },
            py::return_value_policy::reference)
        .def("initializeResources", &Processor::initializeResources)
        .def("process", &Processor::process);

    py::class_<CanvasProcessor, Processor, ProcessorPtr<CanvasProcessor>>(m, "CanvasProcessor")
        .def_property("size", &CanvasProcessor::getCanvasSize, &CanvasProcessor::setCanvasSize)
        .def("getUseCustomDimensions", &CanvasProcessor::getUseCustomDimensions)
        .def_property_readonly("customDimensions", &CanvasProcessor::getCustomDimensions)
        .def_property_readonly("image", [](CanvasProcessor *cp) { return cp->getImage().get(); },
                               py::return_value_policy::reference)
        .def_property_readonly("ready", &CanvasProcessor::isReady)
        .def("snapshot", [](CanvasProcessor *canvas, std::string filepath) {
            auto ext = filesystem::getFileExtension(filepath);

            auto writer = canvas->getNetwork()
                              ->getApplication()
                              ->getDataWriterFactory()
                              ->getWriterForTypeAndExtension<Layer>(ext);
            if (!writer) {
                throw Exception("No write for extension " + ext);
            }

            auto layer = canvas->getVisibleLayer();
            writer->writeData(layer, filepath);
        });

    py::class_<PythonScriptProcessor, Processor, ProcessorPtr<PythonScriptProcessor>>(
        m, "PythonScriptProcessor", py::dynamic_attr{})
        .def("setInitializeResources", &PythonScriptProcessor::setInitializeResources)
        .def("setProcess", &PythonScriptProcessor::setProcess);
}
}  // namespace inviwo
