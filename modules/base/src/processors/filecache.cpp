/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/processors/filecache.h>

#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/links/propertylink.h>

#include <inviwo/core/io/serialization/ticpp.h>

#include <unordered_set>
#include <memory_resource>

namespace inviwo {

namespace detail {

std::string cacheState(Processor* processor, ProcessorNetwork& net,
                       const std::filesystem::path& refPath, std::pmr::string& xml) {
    std::pmr::monotonic_buffer_resource mbr{1024 * 32};

    std::pmr::vector<Processor*> processors(&mbr);
    std::pmr::unordered_set<Processor*> state(&mbr);
    util::traverseNetwork<util::TraversalDirection::Up, util::VisitPattern::Post>(
        state, processor, [&processors](Processor* p) { processors.push_back(p); });

    // ensure a consistent order
    std::ranges::sort(processors, std::less<>{},
                      [](const Processor* p) { return p->getIdentifier(); });

    // Skip the cache processor it self.
    std::erase(processors, processor);

    Serializer s{refPath, SerializeConstants::InviwoWorkspace, &mbr};
    s.setWorkspaceSaveMode(WorkspaceSaveMode::Undo);  // don't same any "metadata"
    const auto ns = s.switchToNewNode("ProcessorNetwork");
    s.serialize("ProcessorNetworkVersion", ProcessorNetwork::processorNetworkVersion());

    s.serializeRange("Processors", processors, [](Serializer& nested, const Processor* item) {
        nested.serialize("Processor", *item);
    });


    // We want to serialize the connection the cacheProcessor;
    processors.push_back(processor);

    s.serializeRange(
        "Connections",
        net.connectionVecRange() | std::views::filter([&](const PortConnection& connection) {
            auto* in = connection.getInport()->getProcessor();
            auto* out = connection.getOutport()->getProcessor();
            return util::contains(processors, in) && util::contains(processors, out);
        }),
        [](Serializer& nested, const PortConnection& connection) {
            const auto nodeSwitch = nested.switchToNewNode("Connection");
            connection.getOutport()->getPath(nested.addAttribute("src"));
            connection.getInport()->getPath(nested.addAttribute("dst"));
        });

    s.serializeRange(
        "PropertyLinks",
        net.linkRange() | std::views::filter([&](const PropertyLink& link) {
            return util::contains(
                       processors,
                       link.getDestination()->getOwner()->getProcessor()->getProcessor()) &&
                   util::contains(processors, link.getSource()->getOwner()->getProcessor());
        }),
        [](Serializer& nested, const PropertyLink& link) {
            const auto nodeSwitch = nested.switchToNewNode("PropertyLink");
            link.getSource()->getPath(nested.addAttribute("src"));
            link.getDestination()->getPath(nested.addAttribute("dst"));
        });

    const auto remove = [](TiXmlElement* elem, const auto& check, const auto& self) -> void {
        TiXmlElement* child = elem->FirstChildElement();
        while (child) {
            auto* curr = child;
            child = child->NextSiblingElement();

            if (check(curr, elem)) {
                elem->RemoveChild(curr);
            } else {
                self(curr, check, self);
            }
        }
    };
    remove(
        s.doc().RootElement(),
        [](TiXmlElement* current, TiXmlElement*) -> bool {
            return current->Value() == "MetaDataItem" &&
                   current->Attribute("type")
                       .transform([](std::string_view value) {
                           return value == "org.inviwo.ProcessorMetaData";
                       })
                       .value_or(false);
        },
        remove);

    // remove all ivwdataRelativePath paths
    remove(
        s.doc().RootElement(),
        [](TiXmlElement* current, TiXmlElement*) -> bool {
            return current->Value() == "ivwdataRelativePath";
        },
        remove);

    if (!refPath.empty()) {
        // remove absolutePath if we have workspaceRelativePaths
        remove(
            s.doc().RootElement(),
            [](TiXmlElement* current, TiXmlElement* parent) -> bool {
                if (current->Value() == "absolutePath") {
                    if (auto* rp = parent->FirstChildElement("workspaceRelativePath")) {
                        return rp->Attribute("content")
                            .transform([](std::string_view value) { return !value.empty(); })
                            .value_or(false);
                    }
                }
                return false;
            },
            remove);
    }

    xml.clear();
    s.write(xml);

    return {fmt::format("{:016X}", std::hash<std::string_view>{}(xml))};
}

}  // namespace detail

CacheBase::CacheBase(InviwoApplication* app)
    : Processor()
    , enabled_{"enabled", "Enabled", "Toggles the usage of the file cache"_help, true}
    , cacheDir_{"cacheDir", "Cache Dir",
                "Directory to save cached dataset too. "
                "You might want to manually clear it regularly to save space"_help,
                filesystem::getPath(PathType::Cache)}
    , refDir_{"refDir", "Reference Dir",
              "Any paths are hashed relative to this path,"
              "instead of the absolute path, if set"_help}
    , currentKey_{"key", "Hashed State", "", InvalidationLevel::Valid} {

    isReady_.setUpdate([this]() {
        if (getInports().empty()) return true;

        auto* inport = getInports().front();
        return isCached_ ||
               (inport->isConnected() && util::all_of(inport->getConnectedOutports(),
                                                      [](Outport* p) { return p->isReady(); }));
    });

    currentKey_.setReadOnly(true);
    currentKey_.setSerializationMode(PropertySerializationMode::None);

    app->getProcessorNetworkEvaluator()->addObserver(this);
}

void CacheBase::onProcessorNetworkEvaluationBegin() {
    if (isValid()) return;

    key_ = detail::cacheState(this, *getNetwork(), refDir_.get(), xml_);
    currentKey_.set(key_);

    const auto isCached = hasCache(key_) && enabled_;

    if (isCached_ != isCached) {
        isCached_ = isCached;
        isReady_.update();
        notifyObserversActiveConnectionsChange(this);
    }
}

bool CacheBase::isConnectionActive(Inport* inport, Outport*) const {
    if (inport == getInports().front()) {
        return !isCached_;
    } else {
        return true;
    }
}

void CacheBase::writeXML() const {
    if (!cacheDir_.get().empty()) {
        if (auto f = std::ofstream(cacheDir_.get() / fmt::format("{}.inv", key_))) {
            f << xml_;
        } else {
            throw Exception(SourceContext{}, "Could not write to xml file: {}/{}.inv",
                            cacheDir_.get(), key_);
        }
    }
}

}  // namespace inviwo
