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

#include <inviwo/core/ports/portinspectormanager.h>

#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/ports/portinspector.h>
#include <inviwo/core/ports/portinspectorfactory.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/autolinker.h>

namespace inviwo {

PortInspector* PortInspectorManager::borrow(Outport* outport) {
    auto it = std::find_if(unUsedInspectors_.begin(), unUsedInspectors_.end(), [&](const auto& pi) {
        return pi->getPortClassName() == outport->getClassIdentifier();
    });
    if (it != unUsedInspectors_.end()) {
        return it->get();
    } else {
        auto factory = app_->getPortInspectorFactory();
        if (auto portInspector = factory->create(outport->getClassIdentifier())) {
            unUsedInspectors_.push_back(std::move(portInspector));
            return unUsedInspectors_.back().get();
        } else {
            return nullptr;
        }
    }
}
std::unique_ptr<PortInspector> PortInspectorManager::getPortInspector(Outport* outport) {
    auto it = std::find_if(unUsedInspectors_.begin(), unUsedInspectors_.end(), [&](const auto& pi) {
        return pi->getPortClassName() == outport->getClassIdentifier();
    });
    if (it != unUsedInspectors_.end()) {
        auto pi = std::move(*it);
        unUsedInspectors_.erase(it);
        return pi;
    } else {
        auto factory = app_->getPortInspectorFactory();
        return factory->create(outport->getClassIdentifier());
    }
}

bool PortInspectorManager::isPortInspectorSupported(const Outport* outport) {
    auto factory = app_->getPortInspectorFactory();
    return factory->hasKey(outport->getClassIdentifier());
}

void PortInspectorManager::returnPortInspector(std::unique_ptr<PortInspector> pi) {
    unUsedInspectors_.push_back(std::move(pi));
}

void PortInspectorManager::insertNetwork(PortInspector* portInspector, ProcessorNetwork* network,
                                         Outport* outport, bool bidirectionalAutoLinks) {
    for (auto& processor : portInspector->getProcessors()) {
        network->addProcessor(processor);
    }

    // Connect the port to inspect to the inports of the inspector network
    for (auto& inport : portInspector->getInports()) {
        network->addConnection(outport, inport);
    }

    // Add connections to the network
    for (auto& connection : portInspector->getConnections()) {
        network->addConnection(connection);
    }

    // Add links to the network
    for (auto& link : portInspector->getPropertyLinks()) {
        network->addLink(link);
    }

    // Do auto-linking.
    for (auto& processor : portInspector->getProcessors()) {
        AutoLinker al(network, processor, outport->getProcessor());
        al.addLinksToClosestCandidates(bidirectionalAutoLinks);
    }
}

void PortInspectorManager::removeNetwork(PortInspector* portInspector, ProcessorNetwork* network) {
    auto processors = portInspector->getProcessors();
    for (auto& processor : processors) {
        network->removeProcessor(processor);
    }
}

PortInspectorManager::PortInspectorManager(InviwoApplication* app) : app_(app) {
    app_->getProcessorNetwork()->addObserver(this);
}

PortInspectorManager::~PortInspectorManager() { clear(); }

bool PortInspectorManager::hasPortInspector(Outport* outport) const {

    return std::find_if(portInspectors_.begin(), portInspectors_.end(), [&](const auto& item) {
               return item.first == getPortId(outport);
           }) != portInspectors_.end();
}

ProcessorWidget* PortInspectorManager::addPortInspector(Outport* outport, ivec2 pos) {
    if (!outport) return nullptr;

    try {
        if (auto portInspector = getPortInspector(outport)) {
            auto network = app_->getProcessorNetwork();
            NetworkLock lock(network);

            // Setup the widget
            CanvasProcessor* canvasProcessor = portInspector->getCanvasProcessor();
            canvasProcessor->createMetaData<BoolMetaData>("PortInspector")->set(true);
            auto widgetMeta = canvasProcessor->getMetaData<ProcessorWidgetMetaData>(
                ProcessorWidgetMetaData::CLASS_IDENTIFIER);
            auto size = app_->getSettingsByType<SystemSettings>()->portInspectorSize_.get();
            widgetMeta->setDimensions(ivec2(size, size));
            widgetMeta->setPosition(pos);
            widgetMeta->setVisibile(false);

            // Add processors to the network
            insertNetwork(portInspector.get(), network, outport, true);

            auto processorWidget = canvasProcessor->getProcessorWidget();
            if (!processorWidget) {
                util::log(IVW_CONTEXT_CUSTOM("PortInspector"), "Problem using port inspector",
                          LogLevel::Error);
                return nullptr;
            }
            portInspectors_[getPortId(outport)] = std::move(portInspector);
            return processorWidget;
        }
    } catch (Exception& exception) {
        util::log(exception.getContext(), exception.getMessage(), LogLevel::Error);
    } catch (...) {
        util::log(IVW_CONTEXT_CUSTOM("PortInspector"), "Problem using port inspector",
                  LogLevel::Error);
    }
    return nullptr;
}

void PortInspectorManager::removePortInspector(Outport* outport) {

    auto it = std::find_if(portInspectors_.begin(), portInspectors_.end(),
                           [&](const auto& item) { return item.first == getPortId(outport); });
    if (it != portInspectors_.end()) {
        auto network = app_->getProcessorNetwork();
        NetworkLock lock(network);
        removePortInspector(it);
        portInspectors_.erase(it);
    }
}

void PortInspectorManager::removePortInspector(PortInspectorMap::iterator it) {
    auto portInspector = std::move(it->second);
    portInspector->getCanvasProcessor()->getProcessorWidget()->setVisible(false);

    // Remove processors from the network
    removeNetwork(portInspector.get(), app_->getProcessorNetwork());
    returnPortInspector(std::move(portInspector));
}

std::shared_ptr<const Image> PortInspectorManager::renderPortInspectorImage(Outport* outport) {
    std::shared_ptr<const Image> image;

    try {
        if (auto portInspector = borrow(outport)) {
            if (auto canvasProcessor = portInspector->getCanvasProcessor()) {
                canvasProcessor->setEvaluateWhenHidden(true);
                canvasProcessor->createMetaData<BoolMetaData>("PortInspector")->set(false);
                auto widgetMeta = canvasProcessor->getMetaData<ProcessorWidgetMetaData>(
                    ProcessorWidgetMetaData::CLASS_IDENTIFIER);
                auto size = app_->getSystemSettings().portInspectorSize_.get();
                widgetMeta->setDimensions(ivec2(size, size));
                widgetMeta->setVisibile(false);

                auto network = app_->getProcessorNetwork();
                {
                    // Add processors to the network
                    NetworkLock lock(network);
                    insertNetwork(portInspector, network, outport, false);
                }  // Network will unlock and evaluate here.

                // clone the image since removing the port inspector processors from the network
                // will cause a re-evaluation and thus resize this image to 8x8 pixel
                if (auto img = canvasProcessor->getImage()) {
                    image.reset(img->clone());
                }

                {
                    // remove the network...
                    NetworkLock lock(network);
                    removeNetwork(portInspector, network);
                    canvasProcessor->setEvaluateWhenHidden(false);
                }
            }
        }
    } catch (Exception& exception) {
        util::log(exception.getContext(), exception.getMessage(), LogLevel::Error);
    } catch (...) {
        util::log(IVW_CONTEXT_CUSTOM("PortInspector"), "Problem using port inspector",
                  LogLevel::Error);
    }
    return image;
}

void PortInspectorManager::clear() {
    auto network = app_->getProcessorNetwork();
    NetworkLock lock(network);
    for (auto it = portInspectors_.begin(); it != portInspectors_.end(); ++it) {
        removePortInspector(it);
    }
    portInspectors_.clear();
    unUsedInspectors_.clear();
}

void PortInspectorManager::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    for (auto& outport : processor->getOutports()) removePortInspector(outport);
}

void PortInspectorManager::serialize(Serializer& s) const {
    s.serialize("PortInsectors", portInspectors_, "PortInspector");
}

void PortInspectorManager::deserialize(Deserializer& d) {
    d.deserialize("PortInsectors", portInspectors_, "PortInspector");
}

std::string PortInspectorManager::getPortId(Outport* outport) {
    if (!outport) return "";
    if (!outport->getProcessor()) return outport->getIdentifier();
    return outport->getProcessor()->getIdentifier() + "." + outport->getIdentifier();
}

}  // namespace inviwo
