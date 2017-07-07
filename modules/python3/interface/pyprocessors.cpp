/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/python3/interface/pyprocessors.h>

#include <modules/python3/interface/inviwopy.h>

#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/metadata/processormetadata.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>

namespace py = pybind11;

namespace inviwo {

void exposeProcessors(py::module &m) {

    py::class_<ProcessorFactory>(m, "ProcessorFactory")
        .def("hasKey", [](ProcessorFactory *pf, std::string key) { return pf->hasKey(key); })
        .def_property_readonly("keys", [](ProcessorFactory *pf) { return pf->getKeys(); })
        .def("create",
             [](ProcessorFactory *pf, std::string key) { return pf->create(key).release(); })
        .def("create", [](ProcessorFactory *pf, std::string key, ivec2 pos) {
            auto p = pf->create(key);
            if (!p)
                throw Exception("failed to create processor of type '" + key + "'",
                                IvwContextCustom("inviwopy"));
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

    py::class_<Processor, PropertyOwner, ProcessorPtr<Processor>>(m, "Processor")
        .def("__getattr__", &getPropertyById<Processor>, py::return_value_policy::reference)
        .def_property_readonly("classIdentifier", &Processor::getClassIdentifier)
        .def_property_readonly("displayName", &Processor::getDisplayName)
        .def_property_readonly("category", &Processor::getCategory)
        .def_property_readonly("codeState", &Processor::getCodeState)  // TODO expose states
        .def_property_readonly("tags", &Processor::getTags)            // TODO expose tags
        .def_property("identifier", &Processor::getIdentifier, &Processor::setIdentifier)
        .def("hasProcessorWidget", &Processor::hasProcessorWidget)
        .def_property_readonly("widget", &Processor::getProcessorWidget)
        .def_property_readonly("network", &Processor::getNetwork,
                               py::return_value_policy::reference)
        .def_property_readonly("inports", &Processor::getInports,
                               py::return_value_policy::reference)
        .def_property_readonly("outports", &Processor::getOutports,
                               py::return_value_policy::reference)
        .def("getPort", &Processor::getPort, py::return_value_policy::reference)
        .def("getInport", &Processor::getInport, py::return_value_policy::reference)
        .def("getOutport", &Processor::getOutport, py::return_value_policy::reference)
        .def_property("position",
                      [](Processor *p) {
                          return p
                              ->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->getPosition();
                      },
                      [](Processor *p, ivec2 pos) {
                          p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->setPosition(pos);
                      })
        .def_property("selected",
                      [](Processor *p) {
                          return p
                              ->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->isSelected();
                      },
                      [](Processor *p, bool selected) {
                          p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->setSelected(selected);
                      })
        .def_property("visible",
                      [](Processor *p) {
                          return p
                              ->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->isVisible();
                      },
                      [](Processor *p, bool selected) {
                          p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->setVisible(selected);
                      });

    py::class_<CanvasProcessor, Processor, ProcessorPtr<CanvasProcessor>> canvasPorcessor(
        m, "CanvasProcessor");
    canvasPorcessor
        .def_property("size", &CanvasProcessor::getCanvasSize, &CanvasProcessor::setCanvasSize)
        .def("getUseCustomDimensions", &CanvasProcessor::getUseCustomDimensions)
        .def_property_readonly("customDimensions", &CanvasProcessor::getCustomDimensions)
        .def_property_readonly("image", [](CanvasProcessor *cp) { return cp->getImage().get(); },
                               py::return_value_policy::reference)
        .def_property_readonly("ready", &CanvasProcessor::isReady)
        .def_property("fullScreen", &CanvasProcessor::isFullScreen, &CanvasProcessor::setFullScreen)
        .def("snapshot", [](CanvasProcessor *canvas, std::string filepath) {
            auto ext = filesystem::getFileExtension(filepath);

            auto writer = canvas->getNetwork()
                              ->getApplication()
                              ->getDataWriterFactory()
                              ->getWriterForTypeAndExtension<Layer>(ext);
            if (!writer) {
                std::stringstream ss;
                ss << "No write for extension " << ext;
                throw Exception(ss.str().c_str());
            }

            auto layer = canvas->getVisibleLayer();
            writer->writeData(layer, filepath);
        });
}
}  // namespace inviwo
