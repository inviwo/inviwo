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

#include <inviwo/core/properties/propertypresetmanager.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/metadata/containermetadata.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/metadata/metadatafactory.h>

namespace inviwo {

PropertyPresetManager::PropertyPresetManager() { loadApplicationPresets(); }

bool PropertyPresetManager::loadPreset(const std::string& name, Property* property,
                                       PropertyPresetType type) const {
    auto apply = [](Property* p, const std::string& data) {
        NetworkLock lock(p);
        std::stringstream ss;
        ss << data;
        Deserializer deserializer(ss, "");
        auto app = InviwoApplication::getPtr();
        deserializer.registerFactory(app->getPropertyFactory());
        deserializer.registerFactory(app->getMetaDataFactory());
        // save current status, except property value, as the preset might overwrite it
        const auto identifier = p->getIdentifier();
        const auto displayName = p->getDisplayName();
        const auto semantics = p->getSemantics();
        const auto readOnly = p->getReadOnly();
        const auto usage = p->getUsageMode();
        p->deserialize(deserializer);
        // restore property state
        p->setIdentifier(identifier);
        p->setDisplayName(displayName);
        p->setSemantics(semantics);
        p->setReadOnly(readOnly);
        p->setUsageMode(usage);
    };

    switch (type) {
        case PropertyPresetType::Property: 
        {
            auto pmap = getPropertyPresets(property);
            auto it = std::find_if(pmap.begin(), pmap.end(),
                                   [&](const auto& pair) { return pair.first == name; });
            if (it != pmap.end()) {
                apply(property, it->second);
                return true;
            }
            break;
        }
        case PropertyPresetType::Workspace:
        {
            auto it = std::find_if(workspacePresets_.begin(), workspacePresets_.end(),
                                   [&](const auto& item) { return item.name == name; });
            if (it != workspacePresets_.end() &&
                it->classIdentifier == property->getClassIdentifier()) {
                apply(property, it->data);
                return true;
            }
            break;
        }
        case PropertyPresetType::Application:
        {
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
    property->serialize(serializer);
    std::stringstream ss;
    serializer.writeFile(ss);

    switch (type) {
        case PropertyPresetType::Property: {
            auto& pmap = getPropertyPresets(property);
            pmap[name] = ss.str();
            break;
        }
        case PropertyPresetType::Workspace: {
            workspacePresets_.emplace_back(property->getClassIdentifier(), name, ss.str());
            break;
        }
        case PropertyPresetType::Application: {
            appPresets_.emplace_back(property->getClassIdentifier(), name, ss.str());
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
            return util::erase_remove_if(workspacePresets_,
                                         [&](const auto& item) { return item.name == name; }) > 0;
        }
        case PropertyPresetType::Application: {
            auto removed = util::erase_remove_if(
                appPresets_, [&](const auto& item) { return item.name == name; });
            saveApplicationPresets();
            return removed > 0;
        }
        default:
            return false;
    }
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
        std::accumulate(workspacePresets_.begin(), workspacePresets_.end(),
                        std::back_inserter(result), [&](auto it, const auto& item) {
                            if (item.classIdentifier == property->getClassIdentifier()) {
                                *(it++) = item.name;
                            }
                            return it;
                        });
    }
    if (types & PropertyPresetType::Application) {
        std::accumulate(appPresets_.begin(), appPresets_.end(), std::back_inserter(result),
                        [&](auto it, const auto& item) {
                            if (item.classIdentifier == property->getClassIdentifier()) {
                                *(it++) = item.name;
                            }
                            return it;
                        });
    }
    return result;
}

void PropertyPresetManager::loadApplicationPresets() {
    std::string filename = filesystem::getPath(PathType::Settings, "/PropertyPresets.ivs");
    if (filesystem::fileExists(filename)) {
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
    } catch (std::exception e) {
        LogWarn("Could not write application presets");
    }
}

void PropertyPresetManager::clearPropertyPresets(Property *property) {
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

PropertyPresetManager::Preset::Preset(std::string id, std::string n, std::string d)
    : classIdentifier(id), name(n), data(d) {}

PropertyPresetManager::Preset::~Preset() = default;

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

std::map<std::string, std::string>& PropertyPresetManager::getPropertyPresets(Property* property) {
    using MT = StdUnorderedMapMetaData<std::string, std::string>;
    return property->createMetaData<MT>("SavedState")->getMap();
}

}  // namespace
