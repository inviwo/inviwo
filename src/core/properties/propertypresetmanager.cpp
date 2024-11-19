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

#include <inviwo/core/properties/propertypresetmanager.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/metadata/containermetadata.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/util/ostreamjoiner.h>

#include <algorithm>
#include <ostream>
#include <sstream>

namespace inviwo {

PropertyPresetManager::PropertyPresetManager(InviwoApplication* app) : app_{app} {
    loadApplicationPresets();
}

bool PropertyPresetManager::loadPreset(const std::string& name, Property* property,
                                       PropertyPresetType type) const {
    auto apply = [this](Property* p, const std::string& data) {
        NetworkLock lock(p);
        std::stringstream ss;
        ss << data;
        auto d = app_->getWorkspaceManager()->createWorkspaceDeserializer(ss, "");

        // We deserialize into a clone here and link it to the original to only set value not
        // identifiers and such.
        auto temp = std::unique_ptr<Property>(p->clone());
        auto reset = scopedSerializationModeAll(temp.get());
        temp->deserialize(d);
        p->set(temp.get());
    };

    switch (type) {
        case PropertyPresetType::Property: {
            auto pmap = getPropertyPresets(property);
            auto it = std::find_if(pmap.begin(), pmap.end(),
                                   [&](const auto& pair) { return pair.first == name; });
            if (it != pmap.end()) {
                apply(property, it->second);
                return true;
            }
            break;
        }
        case PropertyPresetType::Workspace: {
            auto it = std::find_if(workspacePresets_.begin(), workspacePresets_.end(),
                                   [&](const auto& item) { return item.name == name; });
            if (it != workspacePresets_.end() &&
                it->classIdentifier == property->getClassIdentifier()) {
                apply(property, it->data);
                return true;
            }
            break;
        }
        case PropertyPresetType::Application: {
            auto it = std::find_if(appPresets_.begin(), appPresets_.end(),
                                   [&](const auto& item) { return item.name == name; });
            if (it != appPresets_.end() && it->classIdentifier == property->getClassIdentifier()) {
                apply(property, it->data);
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

void PropertyPresetManager::savePreset(const std::string& name, Property* property,
                                       PropertyPresetType type) {
    if (!property) return;

    Serializer serializer("");
    {
        auto reset = scopedSerializationModeAll(property);
        property->serialize(serializer);
    }
    std::stringstream ss;
    serializer.writeFile(ss);

    switch (type) {
        case PropertyPresetType::Property: {
            auto& pmap = getPropertyPresets(property);
            pmap[name] = ss.str();
            break;
        }
        case PropertyPresetType::Workspace: {
            auto it = std::find_if(workspacePresets_.begin(), workspacePresets_.end(),
                                   [&](const auto& item) { return item.name == name; });
            if (it != workspacePresets_.end()) {
                it->classIdentifier = property->getClassIdentifier();
                it->data = ss.str();
            } else {
                workspacePresets_.emplace_back(property->getClassIdentifier(), name, ss.str());
            }
            break;
        }
        case PropertyPresetType::Application: {
            auto it = std::find_if(appPresets_.begin(), appPresets_.end(),
                                   [&](const auto& item) { return item.name == name; });
            if (it != appPresets_.end()) {
                it->classIdentifier = property->getClassIdentifier();
                it->data = ss.str();
            } else {
                appPresets_.emplace_back(property->getClassIdentifier(), name, ss.str());
            }
            saveApplicationPresets();
            break;
        }
        default:
            break;
    }
}

bool PropertyPresetManager::removePreset(const std::string& name, PropertyPresetType type,
                                         Property* property) {
    switch (type) {
        case PropertyPresetType::Property: {
            if (!property) return false;
            auto& pmap = getPropertyPresets(property);
            return pmap.erase(name) > 0;
        }
        case PropertyPresetType::Workspace: {
            return std::erase_if(workspacePresets_,
                                 [&](const auto& item) { return item.name == name; }) > 0;
        }
        case PropertyPresetType::Application: {
            auto removed =
                std::erase_if(appPresets_, [&](const auto& item) { return item.name == name; });
            saveApplicationPresets();
            return removed > 0;
        }
        default:
            return false;
    }
}

void PropertyPresetManager::appendPropertyPresets(Property* target, Property* source) {
    auto& pmap = getPropertyPresets(target);
    for (auto item : getPropertyPresets(source)) {
        pmap[item.first] = item.second;
    }
}

inviwo::util::OnScopeExit PropertyPresetManager::scopedSerializationModeAll(Property* property) {
    std::vector<std::pair<Property*, PropertySerializationMode>> toReset;
    std::function<void(Property*)> setPSM = [&](Property* p) {
        if (p->getSerializationMode() != PropertySerializationMode::All) {
            toReset.emplace_back(p, p->getSerializationMode());
            p->setSerializationMode(PropertySerializationMode::All);
        }
        if (auto comp = dynamic_cast<CompositeProperty*>(p)) {
            for (auto child : comp->getProperties()) {
                setPSM(child);
            }
        }
    };
    setPSM(property);
    return util::OnScopeExit{[toReset]() {
        for (auto item : toReset) {
            item.first->setSerializationMode(item.second);
        }
    }};
}

std::vector<std::string> PropertyPresetManager::getAvailablePresets(
    Property* property, PropertyPresetTypes types) const {
    std::vector<std::string> result;
    if (types & PropertyPresetType::Property) {
        auto pmap = getPropertyPresets(property);
        std::transform(pmap.begin(), pmap.end(), std::back_inserter(result),
                       [&](const auto& pair) { return pair.first; });
    }
    if (types & PropertyPresetType::Workspace) {
        for (auto& item : workspacePresets_) {
            if (item.classIdentifier == property->getClassIdentifier()) {
                result.push_back(item.name);
            }
        }
    }
    if (types & PropertyPresetType::Application) {
        for (auto& item : appPresets_) {
            if (item.classIdentifier == property->getClassIdentifier()) {
                result.push_back(item.name);
            }
        }
    }
    return result;
}

void PropertyPresetManager::loadApplicationPresets() {
    auto filename = filesystem::getPath(PathType::Settings) / "PropertyPresets.ivs";
    if (std::filesystem::is_regular_file(filename)) {
        try {
            Deserializer d(filename);
            d.deserialize("PropertyPresets", appPresets_, "Preset");
        } catch (AbortException& e) {
            LogError(e.getMessage());
        } catch (std::exception& e) {
            LogError(e.what());
        }
    }
}

void PropertyPresetManager::saveApplicationPresets() {
    try {
        Serializer s(filesystem::getPath(PathType::Settings, "/PropertyPresets.ivs", true));
        s.serialize("PropertyPresets", appPresets_, "Preset");
        s.writeFile();
    } catch (const std::exception& e) {
        LogWarn("Could not write application presets: " << e.what());
    }
}

void PropertyPresetManager::clearPropertyPresets(Property* property) {
    if (!property) return;
    auto& pmap = getPropertyPresets(property);
    pmap.clear();
}

void PropertyPresetManager::clearWorkspacePresets() { workspacePresets_.clear(); }

void PropertyPresetManager::loadWorkspacePresets(Deserializer& d) {
    workspacePresets_.clear();
    d.deserialize("PropertyPresets", workspacePresets_, "Preset");
}

void PropertyPresetManager::saveWorkspacePresets(Serializer& s) {
    s.serialize("PropertyPresets", workspacePresets_, "Preset");
}

PropertyPresetManager::Preset::Preset() = default;

PropertyPresetManager::Preset::Preset(std::string_view id, std::string_view n, std::string_view d)
    : classIdentifier(id), name(n), data(d) {}

void PropertyPresetManager::Preset::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier);
    s.serialize("name", name);
    s.serialize("data", data);
}

void PropertyPresetManager::Preset::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier);
    d.deserialize("name", name);
    d.deserialize("data", data);
}

std::map<std::string, std::string, std::less<>>& PropertyPresetManager::getPropertyPresets(
    Property* property) {
    using MT = StdUnorderedMapMetaData<std::string, std::string>;
    return property->createMetaData<MT>("SavedState")->getMap();
}

std::string_view enumToStr(PropertyPresetType p) {
    switch (p) {
        case PropertyPresetType::Property:
            return "Property";
        case PropertyPresetType::Workspace:
            return "Workspace";
        case PropertyPresetType::Application:
            return "Application";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"),
                    "Found invalid PropertyPresetType enum value '{}'", static_cast<int>(p));
}

std::ostream& operator<<(std::ostream& ss, PropertyPresetType p) { return ss << enumToStr(p); }

std::ostream& operator<<(std::ostream& ss, PropertyPresetTypes ps) {
    std::copy(ps.begin(), ps.end(), util::make_ostream_joiner(ss, ", "));
    return ss;
}

}  // namespace inviwo
