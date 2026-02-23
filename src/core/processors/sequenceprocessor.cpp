/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/resizeevent.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/inviwosetupinfo.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/core/network/lambdanetworkvisitor.h>

#include <algorithm>
#include <memory_resource>
#include <ranges>

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
    "Sequence",                      // Display name
    "Meta",                          // Category
    CodeState::Stable,               // Code state
    "Composites",                    // Tags
    "A processor for wrapping ProcessorNetworks"_help,
    false};

const ProcessorInfo& SequenceProcessor::getProcessorInfo() const { return processorInfo_; }

SequenceProcessor::SequenceProcessor(std::string_view identifier, std::string_view displayName,
                                     InviwoApplication* app, const std::filesystem::path& file)
    : Processor(identifier, displayName), app_{app}, sub_{[&] {
        auto net = std::make_unique<ProcessorNetwork>(app);
        auto eval = std::make_unique<ProcessorNetworkEvaluator>(net.get());
        // keep the network locked, only unlock in the process function.
        net->lock();
        net->addObserver(this);
        return NetEval{.net = std::move(net), .eval = std::move(eval)};
    }()} {

    loadSubNetwork(file);
}

SequenceProcessor::~SequenceProcessor() { ProcessorNetworkObserver::removeObservations(); };

void SequenceProcessor::process() {
    const util::KeepTrueWhileInScope processing{&isProcessing_};

    const auto size = std::ranges::fold_left(
        sub_.sources | std::views::transform([](auto* s) { return s->sequenceSize(); }), 0uz,
        std::ranges::max);

    createNetworkCopies(size);

    for (auto&& sink : sub_.sinks) {
        sink->newData();
    }

    if (size > 0) {
        std::ranges::for_each(sub_.sources, [&](auto* source) { source->setSequenceIndex(0); });
        const util::OnScopeExit lock{[this]() { sub_.net->lock(); }};
        sub_.net->unlock();  // This will trigger an evaluation of the sub network.
    }

    for (size_t i = 1; i < size; ++i) {
        std::ranges::for_each(copies_[i - 1].sources,
                              [&](auto& source) { source->setSequenceIndex(i); });
        const util::OnScopeExit lock{[&]() { copies_[i - 1].net->lock(); }};
        copies_[i - 1].net->unlock();  // This will trigger an evaluation of the sub network.
    }

    for (auto&& sink : sub_.sinks) {
        sink->superProcessEnd();
    }
}

void SequenceProcessor::createNetworkCopies(size_t count) {
    if (copies_.size() + 1 == count) return;

    std::pmr::monotonic_buffer_resource mbr{1024 * 32};
    Serializer s{"", SerializeConstants::InviwoWorkspace, &mbr};
    s.serialize("ProcessorNetwork", *sub_.net);
    std::stringstream ss;
    s.writeFile(ss, false);

    auto* wm = app_->getWorkspaceManager();
    auto d = wm->createWorkspaceDeserializer(ss, "");

    while (copies_.size() + 1 < count) {
        auto net = std::make_unique<ProcessorNetwork>(app_);
        auto eval = std::make_unique<ProcessorNetworkEvaluator>(net.get());
        auto& copy = copies_.emplace_back(std::move(net), std::move(eval));

        copy.net->lock();
        copy.net->addObserver(this);
        d.deserialize("ProcessorNetwork", *copy.net);

        for (auto* sinkProcessor : copy.sinks) {
            for (auto* inport : sinkProcessor->getInports()) {
                auto e = std::unique_ptr<Event>{lastResize_->clone()};
                inport->propagateEvent(e.get(), nullptr);
            }
        }
    }
    while (copies_.size() + 1 > count) {
        copies_.pop_back();
    }
}

void SequenceProcessor::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    auto last = std::unique_ptr<Event>{event->clone()};

    if (auto* re = event->getAs<ResizeEvent>()) {
        lastResize_ = std::unique_ptr<ResizeEvent>(re->clone());
    }

    for (auto* sinkProcessor : sub_.sinks) {
        if (&sinkProcessor->getSuperOutport() == source) {
            sinkProcessor->propagateEvent(event, source);
        }
    }
    for (auto& copy : copies_) {
        for (auto* sinkProcessor : copy.sinks) {
            if (&sinkProcessor->getSuperOutport() == source) {
                auto e = std::unique_ptr<Event>{last->clone()};
                for (auto* s : copy.sources) {
                    // mark the event as visited for the source,
                    // we only need this to propagate though the sub network.
                    e->markAsVisited(s);
                }
                sinkProcessor->propagateEvent(e.get(), source);
            }
        }
    }
}

void SequenceProcessor::serialize(Serializer& s) const {
    s.serialize("ProcessorNetwork", *sub_.net);
    Processor::serialize(s);
}

void SequenceProcessor::deserialize(Deserializer& d) {
    d.deserialize("ProcessorNetwork", *sub_.net);
    Processor::deserialize(d);
}

void SequenceProcessor::saveSubNetwork(const std::filesystem::path& file) {
    Serializer s(filesystem::getPath(PathType::Workspaces) / "dummy.inv");

    Tags tags;
    sub_.net->forEachProcessor([&](auto p) { tags.addTags(p->getTags()); });

    // The SequenceProcessorFactoryObject will deserialize the DisplayName and Tags to use in the
    // ProcessorInfo which will be displayed in the processor list
    const InviwoSetupInfo info(*app_, *sub_.net);
    s.serialize("InviwoSetup", info);
    s.serialize("DisplayName", getDisplayName());
    s.serialize("Tags", tags.getString());
    s.serialize("ProcessorNetwork", *sub_.net);

    auto ofs = std::ofstream(file);
    s.writeFile(ofs, true);
}

ProcessorNetwork& SequenceProcessor::getSubNetwork() { return *sub_.net; }

void SequenceProcessor::loadSubNetwork(const std::filesystem::path& file) {
    if (std::filesystem::is_regular_file(file)) {
        sub_.net->clear();
        auto* wm = app_->getWorkspaceManager();
        auto ifs = std::ifstream(file);
        auto d = wm->createWorkspaceDeserializer(
            ifs, filesystem::getPath(PathType::Workspaces) / "dummy.inv");
        auto name = getDisplayName();
        d.deserialize("DisplayName", name);
        setDisplayName(name);
        d.deserialize("ProcessorNetwork", *sub_.net);
    }
}

void SequenceProcessor::registerProperty(Property* subProperty) {
    subProperty->addObserver(this);
    if (subProperty->getMetaData<BoolMetaData>(meta::exposed, false)) {
        addSuperProperty(subProperty);
    }
}

void SequenceProcessor::unregisterProperty(Property* subProperty) {
    subProperty->removeObserver(this);
    handlers_.erase(subProperty);
}

Property* SequenceProcessor::addSuperProperty(Property* subProperty) {
    auto it = handlers_.find(subProperty);
    if (it != handlers_.end()) {
        return it->second->superProperty;
    } else {
        if (subProperty->getOwner()->getProcessor()->getNetwork() == sub_.net.get()) {
            handlers_[subProperty] = std::make_unique<PropertyHandler>(*this, subProperty);
            return handlers_[subProperty]->superProperty;
        } else {
            throw Exception(SourceContext{}, "Could not find property {}", subProperty->getPath());
        }
    }
}

void SequenceProcessor::removeSuperProperty(Property* subProperty) {
    subProperty->unsetMetaData<BoolMetaData>(meta::exposed);
    handlers_.erase(subProperty);

    subProperty->unsetMetaData<IntMetaData>(meta::index);
    for (auto&& [index, superProp] : util::enumerate<int>(*this)) {
        superProp->setMetaData<IntMetaData>(meta::index, index);
        getSubProperty(superProp)->setMetaData<IntMetaData>(meta::index, index);
    }
}

Property* SequenceProcessor::getSuperProperty(Property* subProperty) {
    auto it = handlers_.find(subProperty);
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
    if (p->getNetwork() == sub_.net.get()) {
        LambdaNetworkVisitor visitor{[this](CompositeProperty& prop) {
                                         prop.PropertyOwner::addObserver(this);
                                         registerProperty(&prop);
                                     },
                                     [this](Property& prop) { registerProperty(&prop); }};
        p->accept(visitor);

        if (auto* sink = dynamic_cast<SequenceCompositeSinkBase*>(p)) {
            auto& port = sink->getSuperOutport();
            const auto id = util::findUniqueIdentifier(
                port.getIdentifier(), [&](std::string_view id) { return getPort(id) == nullptr; },
                "");
            port.setIdentifier(id);
            addPort(port);
            sub_.sinks.push_back(sink);
        } else if (auto* source = dynamic_cast<SequenceCompositeSourceBase*>(p)) {
            auto& port = source->getSuperInport();
            const auto id = util::findUniqueIdentifier(
                port.getIdentifier(), [&](std::string_view id) { return getPort(id) == nullptr; },
                "");
            port.setIdentifier(id);
            addPort(port);
            sub_.sources.push_back(source);
        }
    } else if (auto it = std::ranges::find_if(
                   copies_, [&](auto& copy) { return copy.net.get() == p->getNetwork(); });
               it != copies_.end()) {
        auto& copy = *it;

        if (auto* sink = dynamic_cast<SequenceCompositeSinkBase*>(p)) {
            if (auto* org = dynamic_cast<SequenceCompositeSinkBase*>(
                    sub_.net->getProcessorByIdentifier(sink->getIdentifier()))) {

                sink->setSuperOutport(org->getSuperOutportShared());
                sink->setData(org->getData());

                copy.sinks.push_back(sink);
            } else {
                throw Exception(SourceContext{}, "Could not find original sink for {}",
                                sink->getIdentifier());
            }

        } else if (auto* source = dynamic_cast<SequenceCompositeSourceBase*>(p)) {
            if (auto* org = dynamic_cast<SequenceCompositeSourceBase*>(
                    sub_.net->getProcessorByIdentifier(source->getIdentifier()))) {

                source->setSuperInport(org->getSuperInportShared());
                copy.sources.push_back(source);
            } else {
                throw Exception(SourceContext{}, "Could not find original source for {}",
                                source->getIdentifier());
            }
        }
    } else {
        throw Exception(SourceContext{}, "Could not find network for {}", p->getIdentifier());
    }
}

void SequenceProcessor::onProcessorNetworkWillRemoveProcessor(Processor* p) {
    if (p->getNetwork() == sub_.net.get()) {

        LambdaNetworkVisitor visitor{[this](CompositeProperty& prop) {
                                         prop.PropertyOwner::removeObserver(this);
                                         unregisterProperty(&prop);
                                     },
                                     [this](Property& prop) { unregisterProperty(&prop); }};
        p->accept(visitor);

        if (auto* sink = dynamic_cast<SequenceCompositeSinkBase*>(p)) {
            removePort(&sink->getSuperOutport());
            std::erase(sub_.sinks, sink);
        } else if (auto* source = dynamic_cast<SequenceCompositeSourceBase*>(p)) {
            removePort(&source->getSuperInport());
            std::erase(sub_.sources, source);
        }
    } else if (auto it = std::ranges::find_if(
                   copies_, [&](auto& copy) { return copy.net.get() == p->getNetwork(); });
               it != copies_.end()) {
        auto& copy = *it;

        if (auto* sink = dynamic_cast<SequenceCompositeSinkBase*>(p)) {
            std::erase(copy.sinks, sink);
        } else if (auto* source = dynamic_cast<SequenceCompositeSourceBase*>(p)) {
            std::erase(copy.sources, source);
        }
    }
}

void SequenceProcessor::onProcessorBackgroundJobsChanged(Processor*, int diff, int) {
    if (diff > 0) {
        notifyObserversStartBackgroundWork(this, diff);
    } else {
        notifyObserversFinishBackgroundWork(this, -diff);
    }
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
        const util::KeepTrueWhileInScope active{&onChangeActive};
        superProperty->set(subProperty);
    })}
    , superCallback{superProperty->onChange([this]() {
        if (onChangeActive) return;
        const util::KeepTrueWhileInScope active{&onChangeActive};
        subProperty->set(superProperty);
    })} {

    subProperty->setMetaData<BoolMetaData>(meta::exposed, true);

    const auto index = [&]() {
        if (auto* meta = subProperty->getMetaData<IntMetaData>(meta::index)) {
            return meta->get();
        } else {
            subProperty->setMetaData<IntMetaData>(meta::index, static_cast<int>(comp.size()));
            return static_cast<int>(comp.size());
        }
    }();

    auto superId = subProperty->getPath();
    replaceInString(superId, ".", "_");
    superProperty->setIdentifier(superId);
    superProperty->setSerializationMode(PropertySerializationMode::All);
    superProperty->setMetaData<IntMetaData>(meta::index, index);

    auto it = std::ranges::lower_bound(comp, index, std::ranges::less{}, [&](Property* prop) {
        return prop->getMetaData<IntMetaData>(meta::index, 0);
    });

    comp.insertProperty(std::distance(comp.begin(), it), superProperty, false);

    auto findSub = [this](Property* superProp) {
        auto imp = [&](auto self, Property* superProp) -> Property* {
            if (superProp == superProperty) {
                return subProperty;
            } else {
                auto* superOwner = dynamic_cast<CompositeProperty*>(superProp->getOwner());
                auto* subOwner = dynamic_cast<CompositeProperty*>(self(self, superOwner));
                auto pos = std::distance(superOwner->cbegin(), superOwner->find(superProp));
                return (*subOwner)[pos];
            }
        };
        return imp(imp, superProp);
    };

    LambdaNetworkVisitor visitor{[&](Property& superProp) {
        auto* subProp = findSub(&superProp);
        if (auto* meta = subProp->getMetaData<StringMetaData>(meta::displayName)) {
            superProp.setDisplayName(meta->get());
        }
        if (auto* meta = subProp->getMetaData<BoolMetaData>(meta::visible)) {
            superProp.setVisible(meta->get());
        }
        if (auto* meta = subProp->getMetaData<BoolMetaData>(meta::readOnly)) {
            superProp.setReadOnly(meta->get());
        }
        superProp.addObserver(&superObserver);
    }};
    superProperty->accept(visitor);

    superObserver.onDisplayNameChange = [findSub](Property* superProp, std::string_view name) {
        auto* subProp = findSub(superProp);
        subProp->setMetaData<StringMetaData>(meta::displayName, std::string{name});
    };
    superObserver.onVisibleChange = [findSub](Property* superProp, bool visible) {
        auto* subProp = findSub(superProp);
        subProp->setMetaData<BoolMetaData>(meta::visible, visible);
    };
    superObserver.onReadOnlyChange = [findSub](Property* superProp, bool readOnly) {
        auto* subProp = findSub(superProp);
        subProp->setMetaData<BoolMetaData>(meta::readOnly, readOnly);
    };
}

SequenceProcessor::PropertyHandler::~PropertyHandler() {
    superProperty->removeOnChange(superCallback);
    subProperty->removeOnChange(subCallback);
    delete comp.removeProperty(superProperty);
}

void SequenceProcessor::onSetIdentifier(Property* subProperty, const std::string&) {
    if (auto* superProperty = getSuperProperty(subProperty)) {
        auto superId = subProperty->getPath();
        replaceInString(superId, ".", "_");
        superProperty->setIdentifier(superId);
    }
}

void SequenceProcessor::onSetDisplayName(Property* subProperty, const std::string& displayName) {
    if (auto* superProperty = getSuperProperty(subProperty)) {
        superProperty->setDisplayName(displayName);
    }
}

void SequenceProcessor::onSetSemantics(Property* subProperty, const PropertySemantics& semantics) {
    if (auto* superProperty = getSuperProperty(subProperty)) {
        superProperty->setSemantics(semantics);
    }
}

void SequenceProcessor::onSetReadOnly(Property* subProperty, bool readonly) {
    if (auto* superProperty = getSuperProperty(subProperty)) {
        superProperty->setReadOnly(readonly);
    }
}

void SequenceProcessor::onSetVisible(Property* subProperty, bool visible) {
    if (auto* superProperty = getSuperProperty(subProperty)) {
        superProperty->setVisible(visible);
    }
}

}  // namespace inviwo
