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

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CompositeProcessor::processorInfo_{
    "org.inviwo.CompositeProcessor",  // Class identifier
    "Composite",                      // Display name
    "Meta",                           // Category
    CodeState::Stable,                // Code state
    "Composites",                     // Tags
    false};
const ProcessorInfo CompositeProcessor::getProcessorInfo() const { return processorInfo_; }

CompositeProcessor::CompositeProcessor(const std::string& identifier,
                                       const std::string& displayName, InviwoApplication* app,
                                       const std::string& file)
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

void CompositeProcessor::saveSubNetwork(const std::string& file) {
    Serializer s(app_->getPath(PathType::Workspaces));

    Tags tags;
    subNetwork_->forEachProcessor([&](auto p) { tags.addTags(p->getTags()); });

    // The CompositeProcessorFactoryObject will deserialize the DisplayName and Tags to use in the
    // ProcessorInfo which will be displayed in the processor list
    InviwoSetupInfo info(app_);
    s.serialize("InviwoSetup", info);
    s.serialize("DisplayName", getDisplayName());
    s.serialize("Tags", tags.getString());
    s.serialize("ProcessorNetwork", *subNetwork_);

    auto ofs = filesystem::ofstream(file);
    s.writeFile(ofs, true);
}

ProcessorNetwork& CompositeProcessor::getSubNetwork() { return *subNetwork_; }

void CompositeProcessor::loadSubNetwork(const std::string& file) {
    if (filesystem::fileExists(file)) {
        subNetwork_->clear();
        auto wm = app_->getWorkspaceManager();
        auto ifs = filesystem::ifstream(file);
        auto d = wm->createWorkspaceDeserializer(ifs, app_->getPath(PathType::Workspaces));
        auto name = getDisplayName();
        d.deserialize("DisplayName", name);
        setDisplayName(name);
        d.deserialize("ProcessorNetwork", *subNetwork_);
    }
}

void CompositeProcessor::registerProperty(Property* orgProp) {
    orgProp->addObserver(this);
    if (orgProp->getUsageMode() == UsageMode::Application) {
        addSuperProperty(orgProp);
    } else if (orgProp->getMetaData<BoolMetaData>("CompositeProcessorExposed", false)) {
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
        return it->second->superProperty;
    } else {
        if (orgProp->getOwner()->getProcessor()->getNetwork() == subNetwork_.get()) {
            handlers_[orgProp] = std::make_unique<PropertyHandler>(*this, orgProp);
            orgProp->setMetaData<BoolMetaData>("CompositeProcessorExposed", true);
            return handlers_[orgProp]->superProperty;
        } else {
            throw Exception("Could not find property " + joinString(orgProp->getPath(), "."),
                            IVW_CONTEXT);
        }
    }
}

Property* CompositeProcessor::getSuperProperty(Property* orgProp) {
    auto it = handlers_.find(orgProp);
    if (it != handlers_.end()) {
        return it->second->superProperty;
    } else {
        return nullptr;
    }
}

void CompositeProcessor::removeSuperProperty(Property* orgProp) {
    orgProp->unsetMetaData<BoolMetaData>("CompositeProcessorExposed");
    handlers_.erase(orgProp);
}

Property* CompositeProcessor::getSubProperty(Property* superProperty) {
    auto it = util::find_if(
        handlers_, [&](auto& handler) { return handler.second->superProperty == superProperty; });
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
    std::function<void(PropertyOwner*)> observe = [&](PropertyOwner* po) {
        po->addObserver(this);
        for (auto prop : po->getProperties()) {
            registerProperty(prop);
        }
        for (auto child : po->getCompositeProperties()) {
            observe(child);
        }
    };
    observe(p);

    if (auto sink = dynamic_cast<CompositeSinkBase*>(p)) {
        auto& port = sink->getSuperOutport();
        port.setIdentifier(util::findUniqueIdentifier(
            port.getIdentifier(), [&](const std::string& id) { return getPort(id) == nullptr; },
            ""));
        addPort(port);
        sinks_.push_back(sink);
    } else if (auto source = dynamic_cast<CompositeSourceBase*>(p)) {
        auto& port = source->getSuperInport();
        port.setIdentifier(util::findUniqueIdentifier(
            port.getIdentifier(), [&](const std::string& id) { return getPort(id) == nullptr; },
            ""));
        addPort(port);
        sources_.push_back(source);
    }
}

void CompositeProcessor::onProcessorNetworkWillRemoveProcessor(Processor* p) {
    std::function<void(PropertyOwner*)> unObserve = [&](PropertyOwner* po) {
        po->removeObserver(this);
        for (auto prop : po->getProperties()) {
            unregisterProperty(prop);
        }
        for (auto child : po->getCompositeProperties()) {
            unObserve(child);
        }
    };
    unObserve(p);

    if (auto sink = dynamic_cast<CompositeSinkBase*>(p)) {
        removePort(&sink->getSuperOutport());
        util::erase_remove(sinks_, sink);
    } else if (auto source = dynamic_cast<CompositeSourceBase*>(p)) {
        removePort(&source->getSuperInport());
        util::erase_remove(sources_, source);
    }
}

void CompositeProcessor::onDidAddProperty(Property* prop, size_t) { registerProperty(prop); }

void CompositeProcessor::onWillRemoveProperty(Property* prop, size_t) { unregisterProperty(prop); }

CompositeProcessor::PropertyHandler::PropertyHandler(CompositeProcessor& composite,
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
    superProperty->setIdentifier(joinString(subProperty->getPath(), "-"));
    superProperty->setDisplayName(subProperty->getOwner()->getProcessor()->getDisplayName() + " " +
                                  subProperty->getDisplayName());
    superProperty->setSerializationMode(PropertySerializationMode::All);
    comp.addProperty(superProperty, false);
}

CompositeProcessor::PropertyHandler::~PropertyHandler() {
    superProperty->removeOnChange(superCallback);
    subProperty->removeOnChange(subCallback);
    delete comp.removeProperty(superProperty);
}

void CompositeProcessor::onSetIdentifier(Property* orgProp, const std::string&) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setIdentifier(joinString(orgProp->getPath(), "-"));
    }
}

void CompositeProcessor::onSetDisplayName(Property* orgProp, const std::string& displayName) {
    if (auto superProperty = getSuperProperty(orgProp)) {
        superProperty->setSemantics(displayName);
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

void CompositeProcessor::onSetUsageMode(Property* orgProp, UsageMode) {
    if (orgProp->getUsageMode() == UsageMode::Application) {
        addSuperProperty(orgProp);
    } else {
        removeSuperProperty(orgProp);
    }
}

}  // namespace inviwo
