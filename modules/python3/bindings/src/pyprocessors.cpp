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
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/processors/processorwidgetfactoryobject.h>
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
        PYBIND11_OVERLOAD(void, Processor, initializeResources, );
    }
    virtual void process() override { PYBIND11_OVERLOAD(void, Processor, process, ); }
    virtual void doIfNotReady() override { PYBIND11_OVERLOAD(void, Processor, doIfNotReady, ); }
    virtual void setValid() override { PYBIND11_OVERLOAD(void, Processor, setValid, ); }
    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property *modifiedProperty = nullptr) override {
        PYBIND11_OVERLOAD(void, Processor, invalidate, invalidationLevel, modifiedProperty);
    }
    virtual const ProcessorInfo getProcessorInfo() const override {
        PYBIND11_OVERLOAD_PURE(const ProcessorInfo, Processor, getProcessorInfo, );
    }

    virtual void invokeEvent(Event *event) override {
        PYBIND11_OVERLOAD(void, Processor, invokeEvent, event);
    }
    virtual void propagateEvent(Event *event, Outport *source) override {
        PYBIND11_OVERLOAD(void, Processor, propagateEvent, event, source);
    }
};

class ProcessorFactoryObjectTrampoline : public ProcessorFactoryObject {
public:
    using ProcessorFactoryObject::ProcessorFactoryObject;

    virtual pybind11::object createProcessor(InviwoApplication *app) {
        PYBIND11_OVERLOAD(pybind11::object, ProcessorFactoryObjectTrampoline, createProcessor, app);
    }

    virtual std::unique_ptr<Processor> create(InviwoApplication *app) override {
        auto proc = createProcessor(app);
        auto p = std::unique_ptr<Processor>(proc.cast<Processor *>());
        proc.release();
        return p;
    }
};

class ProcessorWidgetFactoryObjectTrampoline : public ProcessorWidgetFactoryObject {
public:
    using ProcessorWidgetFactoryObject::ProcessorWidgetFactoryObject;

    virtual pybind11::object createWidget(Processor *processor) {
        PYBIND11_OVERLOAD(pybind11::object, ProcessorWidgetFactoryObjectTrampoline, createWidget,
                          processor);
    }

    virtual std::unique_ptr<ProcessorWidget> create(Processor *processor) override {
        auto proc = createWidget(processor);
        auto p = std::unique_ptr<ProcessorWidget>(proc.cast<ProcessorWidget *>());
        proc.release();
        return p;
    }
};

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
        .def("getString", &Tag::getString)
        .def_readonly_static("CPU", &Tag::CPU)
        .def_readonly_static("GL", &Tag::GL)
        .def_readonly_static("CL", &Tag::CL)
        .def_readonly_static("PY", &Tag::PY);

    py::class_<Tags>(m, "Tags")
        .def(py::init())
        .def(py::init<Tag>())
        .def(py::init<std::vector<Tag>>())
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
        .def(py::self < py::self)
        .def_readonly_static("None", &Tags::None)
        .def_readonly_static("CPU", &Tags::CPU)
        .def_readonly_static("GL", &Tags::GL)
        .def_readonly_static("CL", &Tags::CL)
        .def_readonly_static("PY", &Tags::PY);

    py::class_<ProcessorInfo>(m, "ProcessorInfo")
        .def(py::init<std::string, std::string, std::string, CodeState, Tags, bool>(),
             py::arg("classIdentifier"), py::arg("displayName"), py::arg("category") = "Python",
             py::arg("codeState") = CodeState::Stable, py::arg("tags") = Tags::PY,
             py::arg("visible") = true)
        .def_readonly("classIdentifier", &ProcessorInfo::classIdentifier)
        .def_readonly("displayName", &ProcessorInfo::displayName)
        .def_readonly("category", &ProcessorInfo::category)
        .def_readonly("codeState", &ProcessorInfo::codeState)
        .def_readonly("tags", &ProcessorInfo::tags)
        .def_readonly("visible", &ProcessorInfo::visible);

    py::class_<ProcessorFactoryObject, ProcessorFactoryObjectTrampoline>(m,
                                                                         "ProcessorFactoryObject")
        .def(py::init<ProcessorInfo>())
        .def("getProcessorInfo", &ProcessorFactoryObject::getProcessorInfo);

    py::class_<ProcessorFactory>(m, "ProcessorFactory")
        .def("hasKey", &ProcessorFactory::hasKey)
        .def_property_readonly("keys", &ProcessorFactory::getKeys)
        .def("create",
             [](ProcessorFactory *pf, std::string key) { return pf->create(key).release(); })
        .def("create",
             [](ProcessorFactory *pf, std::string key, ivec2 pos) {
                 auto p = pf->create(key);
                 if (!p) {
                     throw py::key_error("failed to create processor of type '" + key + "'");
                 }
                 p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                     ->setPosition(pos);

                 return p.release();
             })
        .def("registerObject", &ProcessorFactory::registerObject)
        .def("unRegisterObject", &ProcessorFactory::unRegisterObject);

    py::class_<ProcessorWidget>(m, "ProcessorWidget")
        .def_property("visibility", &ProcessorWidget::isVisible, &ProcessorWidget::setVisible)
        .def_property("dimensions", &ProcessorWidget::getDimensions,
                      &ProcessorWidget::setDimensions)
        .def_property("position", &ProcessorWidget::getPosition, &ProcessorWidget::setPosition)
        .def("show", &ProcessorWidget::show)
        .def("hide", &ProcessorWidget::hide);

    py::class_<ProcessorWidgetFactory>(m, "ProcessorWidgetFactory")
        .def("registerObject", &ProcessorWidgetFactory::registerObject)
        .def("unRegisterObject", &ProcessorWidgetFactory::unRegisterObject)
        .def("create", [](ProcessorWidgetFactory *pf, Processor *p) { return pf->create(p); })
        .def("hasKey", &ProcessorWidgetFactory::hasKey)
        .def("getkeys", &ProcessorWidgetFactory::getKeys);

    py::class_<ProcessorWidgetFactoryObject, ProcessorWidgetFactoryObjectTrampoline>(
        m, "ProcessorWidgetFactoryObject")
        .def(py::init<const std::string &>())
        .def("getClassIdentifier", &ProcessorWidgetFactoryObject::getClassIdentifier);

    py::class_<ProcessorMetaData>(m, "ProcessorMetaData")
        .def_property("position", &ProcessorMetaData::getPosition, &ProcessorMetaData::setPosition)
        .def_property("selected", &ProcessorMetaData::isSelected, &ProcessorMetaData::setSelected)
        .def_property("visible", &ProcessorMetaData::isVisible, &ProcessorMetaData::setVisible);

    using InportVecWrapper = VectorIdentifierWrapper<std::vector<Inport *>>;
    exposeVectorIdentifierWrapper<std::vector<Inport *>>(m, "InportVectorWrapper");

    using OutportVecWrapper = VectorIdentifierWrapper<std::vector<Outport *>>;
    exposeVectorIdentifierWrapper<std::vector<Outport *>>(m, "OutportVectorWrapper");

    py::class_<Processor, ProcessorTrampoline, PropertyOwner, ProcessorPtr<Processor>>(
        m, "Processor", py::dynamic_attr{}, py::multiple_inheritance{})
        .def(py::init<const std::string &, const std::string &>())
        .def("__repr__", &Processor::getIdentifier)
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
        .def("getPortGroup", &Processor::getPortGroup)
        .def("getPortGroups", &Processor::getPortGroups)
        .def("getPortsInGroup", &Processor::getPortsInGroup)
        .def("getPortsInSameGroup", &Processor::getPortsInSameGroup)
        .def("allInportsConnected", &Processor::allInportsConnected)
        .def("allInportsAreReady", &Processor::allInportsAreReady)

        .def("isSource", &Processor::isSource)
        .def("isSink", &Processor::isSink)
        .def("isReady", &Processor::isReady)

        .def("initializeResources", &Processor::initializeResources)
        .def("process", &Processor::process)
        .def("doIfNotReady", &Processor::doIfNotReady)
        .def("setValid", &Processor::setValid)
        .def("invalidate", &Processor::invalidate, py::arg("invalidationLevel"),
             py::arg("modifiedProperty") = nullptr)
        .def("invokeEvent", &Processor::invokeEvent)
        .def("propagateEvent", &Processor::propagateEvent)
        .def_property_readonly(
            "meta",
            [](Processor *p) {
                return p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            },
            py::return_value_policy::reference);

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
                throw Exception("No writer for extension " + ext);
            }

            if (auto layer = canvas->getVisibleLayer()) {
                writer->writeData(layer, filepath);
            } else {
                throw Exception("No image in canvas " + canvas->getIdentifier());
            }
        });

    py::class_<PythonScriptProcessor, Processor, ProcessorPtr<PythonScriptProcessor>>(
        m, "PythonScriptProcessor", py::dynamic_attr{})
        .def("setInitializeResources", &PythonScriptProcessor::setInitializeResources)
        .def("setProcess", &PythonScriptProcessor::setProcess);
}
}  // namespace inviwo
