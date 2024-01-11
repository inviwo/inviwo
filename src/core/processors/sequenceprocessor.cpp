/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <inviwo/core/processors/sequenceprocessor.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>

#include <inviwo/core/processors/sequencecompositesource.h>
#include <inviwo/core/processors/sequencecompositesink.h>
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
constexpr std::string_view exposed = "SequenceProcessorExposed";
constexpr std::string_view index = "SequenceProcessorIndex";
constexpr std::string_view visible = "SequenceProcessorVisible";
constexpr std::string_view readOnly = "SequenceProcessorReadOnly";
constexpr std::string_view displayName = "SequenceProcessorDisplayName";
}  // namespace meta

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SequenceProcessor::processorInfo_{
    "org.inviwo.SequenceProcessor",  // Class identifier
    "Composite",                     // Display name
    "Meta",                          // Category
    CodeState::Stable,               // Code state
    "Composites",                    // Tags
    "A processor for wrapping ProcessorNetworks"_help,
    false};

const ProcessorInfo SequenceProcessor::getProcessorInfo() const { return processorInfo_; }

SequenceProcessor::SequenceProcessor(std::string_view identifier, std::string_view displayName,
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

SequenceProcessor::~SequenceProcessor() = default;

void SequenceProcessor::process() {
    util::KeepTrueWhileInScope processing{&isProcessing_};

    static constexpr auto deref = [](auto&& item) -> decltype(auto) { return *item; }; 

    std::ranges::for_each(sinks_, &SequenceCompositeSinkBase::superProcessStart, deref);

    if (!sources_.empty()) {
        const auto shortest = std::ranges::min(sources_, std::less<>{},
                                               [](auto& source) { return source->sequenceSize(); });
        const auto size = shortest->sequenceSize();

        for (size_t i = 0; i < size; ++i) {
            std::ranges::for_each(sources_, [&](auto& source) { source->setSequenceIndex(i); });
            util::OnScopeExit lock{[this]() { subNetwork_->lock(); }};
            subNetwork_->unlock();  // This will trigger an evaluation of the sub network.
        }
    }

    std::ranges::for_each(sinks_, &SequenceCompositeSinkBase::superProcessEnd, deref);
}

void SequenceProcessor::propagateEvent(Event* event, Outport* source) {
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

void SequenceProcessor::serialize(Serializer& s) const {
    s.serialize("ProcessorNetwork", *subNetwork_);
    Processor::serialize(s);
}

void SequenceProcessor::deserialize(Deserializer& d) {
    d.deserialize("ProcessorNetwork", *subNetwork_);
    Processor::deserialize(d);
}

void SequenceProcessor::saveSubNetwork(const std::filesystem::path& file) {
    Serializer s(app_->getPath(PathType::Workspaces));

    Tags tags;
    subNetwork_->forEachProcessor([&](auto p) { tags.addTags(p->getTags()); });

    // The SequenceProcessorFactoryObject will deserialize the DisplayName and Tags to use in the
    // ProcessorInfo which will be displayed in the processor list
    InviwoSetupInfo info(*app_, *subNetwork_);
    s.serialize("InviwoSetup", info);
    s.serialize("DisplayName", getDisplayName());
    s.serialize("Tags", tags.getString());
    s.serialize("ProcessorNetwork", *subNetwork_);

    auto ofs = std::ofstream(file);
    s.writeFile(ofs, true);
}

ProcessorNetwork& SequenceProcessor::getSubNetwork() { return *subNetwork_; }

void SequenceProcessor::loadSubNetwork(const std::filesystem::path& file) {
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

void SequenceProcessor::registerProperty(Property* orgProp) {
    orgProp->addObserver(this);
    if (orgProp->getMetaData<BoolMetaData>(meta::exposed, false)) {
        addSuperProperty(orgProp);
    }
}

void SequenceProcessor::unregisterProperty(Property* orgProp) {
    orgProp->removeObserver(this);
    handlers_.erase(orgProp);
}

Property* SequenceProcessor::addSuperProperty(Property* orgProp) {
    auto it = handlers_.find(orgProp);
    if (it != handlers_.end()) {
        return it->second->superProperty;
    } else {
        if (orgProp->getOwner()->getProcessor()->getNetwork() == subNetwork_.get()) {
            handlers_[orgProp] = std::make_unique<PropertyHandler>(*this, orgProp);
            return handlers_[orgProp]->superProperty;
        } else {
            throw Exception("Could not find property " + orgProp->getPath(), IVW_CONTEXT);
        }
    }
}

void SequenceProcessor::removeSuperProperty(Property* orgProp) {
    orgProp->unsetMetaData<BoolMetaData>(meta::exposed);
    handlers_.erase(orgProp);

    orgProp->unsetMetaData<IntMetaData>(meta::index);
    for (auto&& [index, superProp] : util::enumerate<int>(*this)) {
        superProp->setMetaData<IntMetaData>(meta::index, index);
        getSubProperty(superProp)->setMetaData<IntMetaData>(meta::index, index);
    }
}

Property* SequenceProcessor::getSuperProperty(Property* orgProp) {
    auto it = handlers_.find(orgProp);
    if (it != handlers_.end()) {
        return it->second->superProperty;
    } else {
        return nullptr;
    }
}

Property* SequenceProcessor::getSubProperty(Property* superProperty) {
    auto it = util::find_if(
        handlers_, [&](auto& handler) { return handler.second->superProperty == superProperty; });
    if (it != handlers_.end()) {
        return it->second->subProperty;
    } else {
        return nullptr;
    }
}

void SequenceProcessor::onProcessorNetworkEvaluateRequest() {
    if (!isProcessing_) {
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void SequenceProcessor::onProcessorNetworkDidAddProcessor(Processor* p) {
    LambdaNetworkVisitor visitor{[this](CompositeProperty& prop) {
                                     prop.PropertyOwner::addObserver(this);
                                     registerProperty(&prop);
                                 },
                                 [this](Property& prop) { registerProperty(&prop); }};
    p->accept(visitor);

    if (auto sink = dynamic_cast<SequenceCompositeSinkBase*>(p)) {
        auto& port = sink->getSuperOutport();
        const auto id = util::findUniqueIdentifier(
            port.getIdentifier(), [&](std::string_view id) { return getPort(id) == nullptr; }, "");
        port.setIdentifier(id);
        addPort(port);
        sinks_.push_back(sink);
    } else if (auto source = dynamic_cast<SequenceCompositeSourceBase*>(p)) {
        auto& port = source->getSuperInport();
        const auto id = util::findUniqueIdentifier(
            port.getIdentifier(), [&](std::string_view id) { return getPort(id) == nullptr; }, "");
        port.setIdentifier(id);
        addPort(port);
        sources_.push_back(source);
    }
}

void SequenceProcessor::onProcessorNetworkWillRemoveProcessor(Processor* p) {
    LambdaNetworkVisitor visitor{[this](CompositeProperty& prop) {
                                     prop.PropertyOwner::removeObserver(this);
                                     unregisterProperty(&prop);
                                 },
                                 [this](Property& prop) { unregisterProperty(&prop); }};
    p->accept(visitor);

    if (auto sink = dynamic_cast<SequenceCompositeSinkBase*>(p)) {
        removePort(&sink->getSuperOutport());
        std::erase(sinks_, sink);
    } else if (auto source = dynamic_cast<SequenceCompositeSourceBase*>(p)) {
        removePort(&source->getSuperInport());
        std::erase(sources_, source);
    }
}

void SequenceProcessor::onProcessorBackgroundJobsChanged(Processor*, int diff, int total) {
    if (diff > 0) {
        notifyObserversStartBackgroundWork(this, diff);
    } else {
        notifyObserversFinishBackgroundWork(this, -diff);
    }

    getProgressBar().setActive(total > 0);
}

void SequenceProcessor::onDidAddProperty(Property* prop, size_t) { registerProperty(prop); }

void SequenceProcessor::onWillRemoveProperty(Property* prop, size_t) { unregisterProperty(prop); }

SequenceProcessor::PropertyHandler::PropertyHandler(SequenceProcessor& composite,
                                                    Property* aSubProperty)
    : comp{composite}
    , subProperty{aSubProperty}
    , superProperty{subProperty->clone()}
    , subCallback{subProperty->onChange([this]() {
        if (onChangeActive) return;
        util::KeepTrueWhileInScope active{&onChangeActive};
        superProperty->set(subProperty);
    })}
    , superCallback{superProperty->onChange([this]() {
        if (onChangeActive) return;
        util::KeepTrueWhileInScope active{&onChangeActive};
        subProperty->set(superProperty);
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

    comp.insertProperty(std::distance(comp.begin(), it), superProperty, false);

    auto findSub = [this](Property* superProp) {
        auto imp = [&](auto self, Property* superProp) -> Property* {
            if (superProp == superProperty) {
                return subProperty;
            } else {
                auto superOwner = dynamic_cast<CompositeProperty*>(superProp->getOwner());
                auto subOwner = dynamic_cast<CompositeProperty*>(self(self, superOwner));
                auto pos = std::distance(superOwner->cbegin(), superOwner->find(superProp));
                return (*subOwner)[pos];
            }
        };
        return imp(imp, superProp);
    };

    LambdaNetworkVisitor visitor{[&](Property& superProp) {
        auto subProp = findSub(&superProp);
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

    superObserver.onDisplayNameChange = [findSub](Property* superProp, std::string_view name) {
        auto subProp = findSub(superProp);
        subProp->setMetaData<StringMetaData>(meta::displayName, std::string{name});
    };
    superObserver.onVisibleChange = [findSub](Property* superProp, bool visible) {
        auto subProp = findSub(superProp);
        subProp->setMetaData<BoolMetaData>(meta::visible, visible);
    };
    superObserver.onReadOnlyChange = [findSub](Property* superProp, bool readOnly) {
        auto subProp = findSub(superProp);
        subProp->setMetaData<BoolMetaData>(meta::readOnly, readOnly);
    };
}

SequenceProcessor::PropertyHandler::~PropertyHandler() {
    superProperty->removeOnChange(superCallback);
    subProperty->removeOnChange(subCallback);
    delete comp.removeProperty(superProperty);
}

void SequenceProcessor::onSetIdentifier(Property* orgProp, const std::string&) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        auto superId = orgProp->getPath();
        replaceInString(superId, ".", "-");
        superProperty->setIdentifier(superId);
    }
}

void SequenceProcessor::onSetDisplayName(Property* orgProp, const std::string& displayName) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setDisplayName(displayName);
    }
}

void SequenceProcessor::onSetSemantics(Property* orgProp, const PropertySemantics& semantics) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setSemantics(semantics);
    }
}

void SequenceProcessor::onSetReadOnly(Property* orgProp, bool readonly) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setReadOnly(readonly);
    }
}

void SequenceProcessor::onSetVisible(Property* orgProp, bool visible) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setVisible(visible);
    }
}

}  // namespace inviwo
