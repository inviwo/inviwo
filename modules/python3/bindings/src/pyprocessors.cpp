/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/trampoline_self_life_support.h>  // for trampoline_self_life_support
#include <pybind11/functional.h>
#include <pybind11/numpy.h>

#include <inviwopy/vectoridentifierwrapper.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/processors/processorwidgetfactoryobject.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/rendercontext.h>

#include <modules/python3/processortrampoline.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>
#include <fmt/std.h>

namespace inviwo {

#include <warn/push>
#include <warn/ignore/attributes>
class ProcessorFactoryObjectTrampoline : public ProcessorFactoryObject,
                                         public pybind11::trampoline_self_life_support {
public:
    using ProcessorFactoryObject::ProcessorFactoryObject;

    virtual pybind11::object createProcessor(InviwoApplication* app) const {
        PYBIND11_OVERLOAD(pybind11::object, ProcessorFactoryObjectTrampoline, createProcessor, app);
    }

    virtual std::shared_ptr<Processor> create(InviwoApplication* app) const override {
        auto proc = createProcessor(app);
        auto p = proc.cast<std::shared_ptr<Processor>>();
        return p;
    }
};

class ProcessorWidgetFactoryObjectTrampoline : public ProcessorWidgetFactoryObject,
                                               public pybind11::trampoline_self_life_support {
public:
    using ProcessorWidgetFactoryObject::ProcessorWidgetFactoryObject;

    virtual pybind11::object createWidget(Processor* processor) {
        PYBIND11_OVERLOAD(pybind11::object, ProcessorWidgetFactoryObjectTrampoline, createWidget,
                          processor);
    }

    virtual std::unique_ptr<ProcessorWidget> create(Processor* processor) override {
        auto proc = createWidget(processor);
        auto p = proc.cast<std::unique_ptr<ProcessorWidget>>();
        return p;
    }
};
#include <warn/pop>

void exposeProcessors(pybind11::module& m) {
    namespace py = pybind11;

    py::enum_<CodeState>(m, "CodeState")
        .value("Broken", CodeState::Broken)
        .value("Experimental", CodeState::Experimental)
        .value("Stable", CodeState::Stable);

    py::classh<Tag>(m, "Tag")
        .def(py::init())
        .def(py::init<std::string_view>())
        .def(py::init<Tag>())
        .def("getString", &Tag::getString)
        .def(py::self | py::self);

    py::classh<Tags>(m, "Tags")
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
        .def_readonly_static("Empty", &Tags::None)
        .def_readonly_static("CPU", &Tags::CPU)
        .def_readonly_static("GL", &Tags::GL)
        .def_readonly_static("CL", &Tags::CL)
        .def_readonly_static("PY", &Tags::PY)
        .def(py::self | py::self)
        .def(Tag() | py::self)
        .def(py::self | Tag())
        .def(py::self |= Tag())
        .def(py::self |= Tags());

    py::implicitly_convertible<Tag, Tags>();

    py::classh<ProcessorInfo>(m, "ProcessorInfo")
        .def(py::init<std::string, std::string, std::string, CodeState, Tags, Document>(),
             py::arg("classIdentifier"), py::arg("displayName"), py::arg("category") = "Python",
             py::arg("codeState") = CodeState::Stable, py::arg("tags") = Tags::PY,
             py::arg("help") = Document{})
        .def_readonly("classIdentifier", &ProcessorInfo::classIdentifier)
        .def_readonly("displayName", &ProcessorInfo::displayName)
        .def_readonly("category", &ProcessorInfo::category)
        .def_readonly("codeState", &ProcessorInfo::codeState)
        .def_readonly("tags", &ProcessorInfo::tags)
        .def_readonly("visible", &ProcessorInfo::visible);

    py::classh<ProcessorFactoryObject, ProcessorFactoryObjectTrampoline>(m,
                                                                         "ProcessorFactoryObject")
        .def(py::init<ProcessorInfo, std::string_view>())
        .def("getProcessorInfo", &ProcessorFactoryObject::getProcessorInfo);

    py::classh<ProcessorFactory>(m, "ProcessorFactory")
        .def("hasKey", &ProcessorFactory::hasKey)
        .def_property_readonly("keys", &ProcessorFactory::getKeys)
        .def("create", [](ProcessorFactory* pf, std::string key) { return pf->createShared(key); })
        .def("create",
             [](ProcessorFactory* pf, std::string key, ivec2 pos) {
                 auto p = pf->createShared(key);
                 if (!p) {
                     throw py::key_error("failed to create processor of type '" + key + "'");
                 }
                 p->getMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier)
                     ->setPosition(pos);

                 return p;
             })
        .def("registerObject", &ProcessorFactory::registerObject)
        .def("unRegisterObject", &ProcessorFactory::unRegisterObject);

    py::classh<ProcessorWidget>(m, "ProcessorWidget", py::multiple_inheritance{})
        .def_property("visible", &ProcessorWidget::isVisible, &ProcessorWidget::setVisible)
        .def_property("dimensions", &ProcessorWidget::getDimensions,
                      &ProcessorWidget::setDimensions)
        .def_property("position", &ProcessorWidget::getPosition, &ProcessorWidget::setPosition)
        .def_property("onTop", &ProcessorWidget::isOnTop, &ProcessorWidget::setOnTop)
        .def_property("fullScreen", &ProcessorWidget::isFullScreen, &ProcessorWidget::setFullScreen)
        .def("address", [](ProcessorWidget* w) {
            return reinterpret_cast<std::intptr_t>(static_cast<void*>(w));
        });

    py::classh<ProcessorWidgetFactory>(m, "ProcessorWidgetFactory")
        .def("registerObject", &ProcessorWidgetFactory::registerObject)
        .def("unRegisterObject", &ProcessorWidgetFactory::unRegisterObject)
        .def("create", [](ProcessorWidgetFactory* pf, Processor* p) { return pf->create(p); })
        .def("hasKey", [](ProcessorWidgetFactory* pf, Processor* p) { return pf->hasKey(p); })
        .def("hasKey",
             [](ProcessorWidgetFactory* pf, std::string_view id) { return pf->hasKey(id); })
        .def("getkeys", &ProcessorWidgetFactory::getKeys);

    py::classh<ProcessorWidgetFactoryObject, ProcessorWidgetFactoryObjectTrampoline>(
        m, "ProcessorWidgetFactoryObject")
        .def(py::init<const std::string&>())
        .def("getClassIdentifier", &ProcessorWidgetFactoryObject::getClassIdentifier);

    py::classh<ProcessorMetaData>(m, "ProcessorMetaData")
        .def_property("position", &ProcessorMetaData::getPosition, &ProcessorMetaData::setPosition)
        .def_property("selected", &ProcessorMetaData::isSelected, &ProcessorMetaData::setSelected)
        .def_property("visible", &ProcessorMetaData::isVisible, &ProcessorMetaData::setVisible);

    using InportVecWrapper = VectorIdentifierWrapper<typename std::vector<Inport*>::const_iterator>;
    exposeVectorIdentifierWrapper<typename std::vector<Inport*>::const_iterator>(
        m, "InportVectorWrapper");

    using OutportVecWrapper =
        VectorIdentifierWrapper<typename std::vector<Outport*>::const_iterator>;
    exposeVectorIdentifierWrapper<typename std::vector<Outport*>::const_iterator>(
        m, "OutportVectorWrapper");

    py::classh<Processor, PropertyOwner, ProcessorTrampoline>(
        m, "Processor", py::multiple_inheritance{}, py::dynamic_attr{})
        .def(py::init<const std::string&, const std::string&>())
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
                               [](Processor* p) {
                                   return InportVecWrapper(p->getInports().begin(),
                                                           p->getInports().end());
                               })
        .def_property_readonly("outports",
                               [](Processor* p) {
                                   return OutportVecWrapper(p->getOutports().begin(),
                                                            p->getOutports().end());
                               })
        .def("getPort", &Processor::getPort, py::return_value_policy::reference)
        .def("getInport", &Processor::getInport, py::return_value_policy::reference)
        .def("getOutport", &Processor::getOutport, py::return_value_policy::reference)
        .def(
            "addInport",
            [](Processor& p, Inport* port, const std::string& group, bool owner) {
                if (owner) {
                    p.addPort(std::unique_ptr<Inport>(port), group);
                } else {
                    p.addPort(*port, group);
                }
            },
            py::arg("inport"), py::arg("group") = "default", py::arg("owner") = false,
            py::keep_alive<1, 2>{})
        .def(
            "addOutport",
            [](Processor& p, Outport* port, const std::string& group, bool owner) {
                if (owner) {
                    p.addPort(std::unique_ptr<Outport>(port), group);
                } else {
                    p.addPort(*port, group);
                }
            },
            py::arg("outport"), py::arg("group") = "default", py::arg("owner") = false,
            py::keep_alive<1, 2>{})
        .def("removeInport", [](Processor& p, Inport* port) { return p.removePort(port); })
        .def("removeOutport", [](Processor& p, Outport* port) { return p.removePort(port); })
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
            [](Processor* p) {
                return p->getMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
            },
            py::return_value_policy::reference)
        .def("getDirectPredecessors", [](Processor* p) { return util::getDirectPredecessors(p); })
        .def("getDirectSuccessors", [](Processor* p) { return util::getDirectSuccessors(p); })
        .def("getPredecessors", [](Processor* p) { return util::getPredecessors(p); })
        .def("getSuccessors", [](Processor* p) { return util::getSuccessors(p); })
        .def("serialize", &Processor::serialize)
        .def("deserialize", &Processor::deserialize);

    py::classh<CanvasProcessor, Processor>(m, "CanvasProcessor")
        .def_property("size", &CanvasProcessor::getCanvasSize, &CanvasProcessor::setCanvasSize)
        .def("getUseCustomDimensions", &CanvasProcessor::getUseCustomDimensions)
        .def_property_readonly("customDimensions", &CanvasProcessor::getCustomDimensions)
        .def_property_readonly(
            "image", [](CanvasProcessor* cp) { return cp->getImage().get(); },
            py::return_value_policy::reference)
        .def_property_readonly("ready", &CanvasProcessor::isReady)
        .def("snapshot",
             [](CanvasProcessor* canvas, const std::filesystem::path& filePath) {
                 auto writer = canvas->getInviwoApplication()
                                   ->getDataWriterFactory()
                                   ->getWriterForTypeAndExtension<Layer>(filePath);
                 if (!writer) {
                     throw Exception(SourceContext{}, "No writer for {}", filePath);
                 }

                 if (auto layer = canvas->getVisibleLayer()) {
                     rendercontext::activateDefault();
                     writer->writeData(layer, filePath);
                 } else {
                     throw Exception(SourceContext{}, "No image in canvas {}",
                                     canvas->getIdentifier());
                 }
             })

        .def("snapshotAsync", [](CanvasProcessor* canvas, const std::filesystem::path& filePath) {
            auto writer = std::shared_ptr<DataWriterType<Layer>>{
                canvas->getInviwoApplication()
                    ->getDataWriterFactory()
                    ->getWriterForTypeAndExtension<Layer>(filePath)};
            if (!writer) {
                throw Exception(SourceContext{}, "No writer for {}", filePath);
            }

            if (auto layer = canvas->getVisibleLayer()) {
                /* Unfortunately we need to clone the layer here since in most cases the layer comes
                 * from an ImageOutport and that will generally render new data into the layer on
                 * the next evaluation.
                 */
                dispatchPool(
                    [layerClone = std::shared_ptr<Layer>{layer->clone()}, writer, filePath]() {
                        rendercontext::activateLocal();
                        writer->writeData(layerClone.get(), filePath);
                    });
            } else {
                throw Exception(SourceContext{}, "No image in canvas {}", canvas->getIdentifier());
            }
        });
}

}  // namespace inviwo
