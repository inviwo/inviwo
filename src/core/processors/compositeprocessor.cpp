/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>

#include <inviwo/core/processors/compositesource.h>
#include <inviwo/core/processors/compositesink.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/network/workspacemanager.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/inviwosetupinfo.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/core/network/lambdanetworkvisitor.h>

#include <algorithm>

namespace inviwo {

namespace meta {
constexpr std::string_view exposed = "CompositeProcessorExposed";
constexpr std::string_view index = "CompositeProcessorIndex";
constexpr std::string_view visible = "CompositeProcessorVisible";
constexpr std::string_view readOnly = "CompositeProcessorReadOnly";
constexpr std::string_view displayName = "CompositeProcessorDisplayName";
}  // namespace meta

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CompositeProcessor::processorInfo_{
    "org.inviwo.CompositeProcessor",  // Class identifier
    "Composite",                      // Display name
    "Meta",                           // Category
    CodeState::Stable,                // Code state
    "Composites",                     // Tags
    "A processor for wrapping ProcessorNetworks"_help,
    false};

const ProcessorInfo CompositeProcessor::getProcessorInfo() const { return processorInfo_; }

CompositeProcessor::CompositeProcessor(std::string_view identifier, std::string_view displayName,
                                       InviwoApplication* app, const std::filesystem::path& file)
    : Processor(identifier, displayName)
    , app_{app}
    , subNetwork_{std::make_unique<ProcessorNetwork>(app)}
    , evaluator_{std::make_unique<ProcessorNetworkEvaluator>(subNetwork_.get())} {

    subNetwork_->addObserver(this);

    // keep the network locked, only unlock in the process function.
    subNetwork_->lock();

    loadSubNetwork(file);
}

CompositeProcessor::~CompositeProcessor() = default;

void CompositeProcessor::process() {
    util::KeepTrueWhileInScope processing{&isProcessing_};
    for (auto& source : sources_) {
        if (source->getSuperInport().isChanged()) {
            source->invalidate(InvalidationLevel::InvalidOutput);
        }
    }
    util::OnScopeExit lock{[this]() { subNetwork_->lock(); }};
    subNetwork_->unlock();  // This will trigger an evaluation of the sub network.
}

void CompositeProcessor::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    for (auto& sinkProcessor : sinks_) {
        if (&sinkProcessor->getSuperOutport() == source) {
            sinkProcessor->propagateEvent(event, source);
        }
    }
}

void CompositeProcessor::serialize(Serializer& s) const {
    s.serialize("ProcessorNetwork", *subNetwork_);
    Processor::serialize(s);
}

void CompositeProcessor::deserialize(Deserializer& d) {
    d.deserialize("ProcessorNetwork", *subNetwork_);
    Processor::deserialize(d);
}

void CompositeProcessor::saveSubNetwork(const std::filesystem::path& file) {
    Serializer s(app_->getPath(PathType::Workspaces));

    Tags tags;
    subNetwork_->forEachProcessor([&](auto p) { tags.addTags(p->getTags()); });

    // The CompositeProcessorFactoryObject will deserialize the DisplayName and Tags to use in the
    // ProcessorInfo which will be displayed in the processor list
    InviwoSetupInfo info(*app_, *subNetwork_);
    s.serialize("InviwoSetup", info);
    s.serialize("DisplayName", getDisplayName());
    s.serialize("Tags", tags.getString());
    s.serialize("ProcessorNetwork", *subNetwork_);

    auto ofs = std::ofstream(file);
    s.writeFile(ofs, true);
}

ProcessorNetwork& CompositeProcessor::getSubNetwork() { return *subNetwork_; }

void CompositeProcessor::loadSubNetwork(const std::filesystem::path& file) {
    if (std::filesystem::is_regular_file(file)) {
        subNetwork_->clear();
        auto wm = app_->getWorkspaceManager();
        auto ifs = std::ifstream(file);
        auto d = wm->createWorkspaceDeserializer(ifs, app_->getPath(PathType::Workspaces));
        auto name = getDisplayName();
        d.deserialize("DisplayName", name);
        setDisplayName(name);
        d.deserialize("ProcessorNetwork", *subNetwork_);
    }
}

void CompositeProcessor::registerProperty(Property* orgProp) {
    orgProp->addObserver(this);
    if (orgProp->getMetaData<BoolMetaData>(meta::exposed, false)) {
        addSuperProperty(orgProp);
    }
}

void CompositeProcessor::unregisterProperty(Property* orgProp) {
    orgProp->removeObserver(this);
    handlers_.erase(orgProp);
}

Property* CompositeProcessor::addSuperProperty(Property* orgProp) {
    auto it = handlers_.find(orgProp);
    if (it != handlers_.end()) {
        return it->second->superProperty.get();
    } else {
        if (orgProp->getOwner()->getProcessor()->getNetwork() == subNetwork_.get()) {
            handlers_[orgProp] = std::make_unique<PropertyHandler>(*this, orgProp);
            return handlers_[orgProp]->superProperty.get();
        } else {
            throw Exception("Could not find property " + orgProp->getPath(), IVW_CONTEXT);
        }
    }
}

void CompositeProcessor::removeSuperProperty(Property* orgProp) {
    orgProp->unsetMetaData<BoolMetaData>(meta::exposed);
    handlers_.erase(orgProp);

    orgProp->unsetMetaData<IntMetaData>(meta::index);
    for (auto&& [index, superProp] : util::enumerate<int>(*this)) {
        superProp->setMetaData<IntMetaData>(meta::index, index);
        getSubProperty(superProp)->setMetaData<IntMetaData>(meta::index, index);
    }
}

Property* CompositeProcessor::getSuperProperty(Property* orgProp) {
    auto it = handlers_.find(orgProp);
    if (it != handlers_.end()) {
        return it->second->superProperty.get();
    } else {
        return nullptr;
    }
}

Property* CompositeProcessor::getSubProperty(Property* superProperty) {
    auto it = util::find_if(handlers_, [&](auto& handler) {
        return handler.second->superProperty.get() == superProperty;
    });
    if (it != handlers_.end()) {
        return it->second->subProperty;
    } else {
        return nullptr;
    }
}

void CompositeProcessor::onProcessorNetworkEvaluateRequest() {
    if (!isProcessing_) {
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void CompositeProcessor::onProcessorNetworkDidAddProcessor(Processor* p) {
    LambdaNetworkVisitor visitor{[this](CompositeProperty& prop) {
                                     prop.PropertyOwner::addObserver(this);
                                     registerProperty(&prop);
                                 },
                                 [this](Property& prop) { registerProperty(&prop); }};
    p->accept(visitor);

    if (auto sink = dynamic_cast<CompositeSinkBase*>(p)) {
        auto& port = sink->getSuperOutport();
        const auto id = util::findUniqueIdentifier(
            port.getIdentifier(), [&](std::string_view id) { return getPort(id) == nullptr; }, "");
        port.setIdentifier(id);
        addPort(port);
        sinks_.push_back(sink);
    } else if (auto source = dynamic_cast<CompositeSourceBase*>(p)) {
        auto& port = source->getSuperInport();
        const auto id = util::findUniqueIdentifier(
            port.getIdentifier(), [&](std::string_view id) { return getPort(id) == nullptr; }, "");
        port.setIdentifier(id);
        addPort(port);
        sources_.push_back(source);
    }
}

void CompositeProcessor::onProcessorNetworkWillRemoveProcessor(Processor* p) {
    LambdaNetworkVisitor visitor{[this](CompositeProperty& prop) {
                                     prop.PropertyOwner::removeObserver(this);
                                     unregisterProperty(&prop);
                                 },
                                 [this](Property& prop) { unregisterProperty(&prop); }};
    p->accept(visitor);

    if (auto sink = dynamic_cast<CompositeSinkBase*>(p)) {
        removePort(&sink->getSuperOutport());
        std::erase(sinks_, sink);
    } else if (auto source = dynamic_cast<CompositeSourceBase*>(p)) {
        removePort(&source->getSuperInport());
        std::erase(sources_, source);
    }
}

void CompositeProcessor::onProcessorBackgroundJobsChanged(Processor*, int diff, int total) {
    if (diff > 0) {
        notifyObserversStartBackgroundWork(this, diff);
    } else {
        notifyObserversFinishBackgroundWork(this, -diff);
    }

    getProgressBar().setActive(total > 0);
}

void CompositeProcessor::onDidAddProperty(Property* prop, size_t) { registerProperty(prop); }

void CompositeProcessor::onWillRemoveProperty(Property* prop, size_t) { unregisterProperty(prop); }

CompositeProcessor::PropertyHandler::PropertyHandler(CompositeProcessor& composite,
                                                     Property* aSubProperty)
    : comp{composite}
    , subProperty{aSubProperty}
    , superProperty{subProperty->clone()}
    , subCallback{subProperty->onChangeScoped([this]() {
        if (onChangeActive) return;
        util::KeepTrueWhileInScope active{&onChangeActive};
        superProperty->set(subProperty);
    })}
    , superCallback{superProperty->onChangeScoped([this]() {
        if (onChangeActive) return;
        util::KeepTrueWhileInScope active{&onChangeActive};
        subProperty->set(superProperty.get());
    })} {

    subProperty->setMetaData<BoolMetaData>(meta::exposed, true);

    const auto index = [&]() {
        if (auto meta = subProperty->getMetaData<IntMetaData>(meta::index)) {
            return meta->get();
        } else {
            subProperty->setMetaData<IntMetaData>(meta::index, static_cast<int>(comp.size()));
            return static_cast<int>(comp.size());
        }
    }();

    auto superId = subProperty->getPath();
    replaceInString(superId, ".", "-");
    superProperty->setIdentifier(superId);
    superProperty->setSerializationMode(PropertySerializationMode::All);
    superProperty->setMetaData<IntMetaData>(meta::index, index);

    auto it = std::lower_bound(comp.begin(), comp.end(), index, [&](Property* prop, int b) {
        const auto a = prop->getMetaData<IntMetaData>(meta::index, 0);
        return a < b;
    });
    comp.insertProperty(std::distance(comp.begin(), it), superProperty.get(), false);

    LambdaNetworkVisitor visitor{[&](Property& superProp) {
        auto subProp = findSubProperty(&superProp);
        if (!subProp) return;

        if (auto meta = subProp->getMetaData<StringMetaData>(meta::displayName)) {
            superProp.setDisplayName(meta->get());
        }
        if (auto meta = subProp->getMetaData<BoolMetaData>(meta::visible)) {
            superProp.setVisible(meta->get());
        }
        if (auto meta = subProp->getMetaData<BoolMetaData>(meta::readOnly)) {
            superProp.setReadOnly(meta->get());
        }
        superProp.addObserver(&superObserver);
    }};
    superProperty->accept(visitor);

    superObserver.onDisplayNameChange = [this](Property* superProp, std::string_view name) {
        if (auto subProp = findSubProperty(superProp)) {
            subProp->setMetaData<StringMetaData>(meta::displayName, std::string{name});
        }
    };
    superObserver.onVisibleChange = [this](Property* superProp, bool visible) {
        if (auto subProp = findSubProperty(superProp)) {
            subProp->setMetaData<BoolMetaData>(meta::visible, visible);
        }
    };
    superObserver.onReadOnlyChange = [this](Property* superProp, bool readOnly) {
        if (auto subProp = findSubProperty(superProp)) {
            subProp->setMetaData<BoolMetaData>(meta::readOnly, readOnly);
        }
    };
}

Property* CompositeProcessor::PropertyHandler::findSubProperty(Property* superProp) const {
    auto imp = [&](auto self, Property* superProp) -> Property* {
        if (superProp == superProperty.get()) {
            return subProperty;
        } else {
            auto* superOwner = dynamic_cast<CompositeProperty*>(superProp->getOwner());
            if (!superOwner) {
                util::logError(IVW_CONTEXT, "Could not find composite owner of property {}",
                               superProp->getPath());
                return nullptr;
            }
            auto* subOwner = dynamic_cast<CompositeProperty*>(self(self, superOwner));
            if (!subOwner) {
                util::logError(IVW_CONTEXT, "Could not find sub Owner {}", superOwner->getPath());
                return nullptr;
            }
            auto it = superOwner->find(superProp);
            if (it == superOwner->cend()) {
                util::logError(IVW_CONTEXT, "Could not find superProp {} in  {}",
                               superProp->getPath(), superOwner->getPath());
                return nullptr;
            }
            auto pos = std::distance(superOwner->cbegin(), superOwner->find(superProp));
            if (static_cast<size_t>(pos) >= subOwner->size()) {
                util::logError(IVW_CONTEXT, "Size missmatch {} {} less than {}",
                               subOwner->getPath(), subOwner->size(), pos);
                return nullptr;
            }

            return (*subOwner)[pos];
        }
    };
    return imp(imp, superProp);
}

CompositeProcessor::PropertyHandler::~PropertyHandler() {
    comp.removeProperty(superProperty.get());
}

void CompositeProcessor::onSetIdentifier(Property* orgProp, const std::string&) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        auto superId = orgProp->getPath();
        replaceInString(superId, ".", "-");
        superProperty->setIdentifier(superId);
    }
}

void CompositeProcessor::onSetDisplayName(Property* orgProp, const std::string& displayName) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setDisplayName(displayName);
    }
}

void CompositeProcessor::onSetSemantics(Property* orgProp, const PropertySemantics& semantics) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setSemantics(semantics);
    }
}

void CompositeProcessor::onSetReadOnly(Property* orgProp, bool readonly) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setReadOnly(readonly);
    }
}

void CompositeProcessor::onSetVisible(Property* orgProp, bool visible) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setVisible(visible);
    }
}

}  // namespace inviwo
