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

#ifndef IVW_PROPERTYPRESETMANAGER_H
#define IVW_PROPERTYPRESETMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/raiiutils.h>
#include <flags/flags.h>

namespace inviwo {
class InviwoApplication;
class Property;

enum class PropertyPresetType { Property = 1, Workspace = 2, Application = 4 };
ALLOW_FLAGS_FOR_ENUM(PropertyPresetType)
using PropertyPresetTypes = flags::flags<PropertyPresetType>;

/**
 * \class PropertyPresetManager
 * \brief Manage property presets. Has a set of global application presets, a set of workspace
 * presets, and handles property specific presets.
 */
class IVW_CORE_API PropertyPresetManager {
public:
    PropertyPresetManager(InviwoApplication* app);
    virtual ~PropertyPresetManager() = default;

    bool loadPreset(const std::string& name, Property* property, PropertyPresetType type) const;

    void savePreset(const std::string& name, Property* property, PropertyPresetType type);
    bool removePreset(const std::string& name, PropertyPresetType type,
                      Property* property = nullptr);
    std::vector<std::string> getAvailablePresets(Property* property,
                                                 PropertyPresetTypes types) const;

    void clearPropertyPresets(Property* property);

    // clear load and save workspace presets
    void clearWorkspacePresets();
    void loadWorkspacePresets(Deserializer& d);
    void saveWorkspacePresets(Serializer& s);

    /**
     * Append all Presets in property source to property target.
     * If the Preset already exists in target it will be overwritten.
     * The properties should be of the same type.
     */
    static void appendPropertyPresets(Property* target, Property* source);

    /**
     * Set PropertySerializationMode to All on property and all its sub properties.
     * The returned guard will reset the PropertySerializationModes to their original values when
     * it goes out of scope. This is useful when copying properties.
     */
    static util::OnScopeExit scopedSerializationModeAll(Property* property);

private:
    void loadApplicationPresets();
    void saveApplicationPresets();

    static std::map<std::string, std::string>& getPropertyPresets(Property* property);

    struct Preset : Serializable {
        Preset();
        Preset(std::string id, std::string n, std::string d);
        virtual ~Preset();
        std::string classIdentifier;
        std::string name;
        std::string data;

        virtual void serialize(Serializer& s) const override;
        virtual void deserialize(Deserializer& d) override;
    };

    InviwoApplication* app_;
    std::vector<Preset> appPresets_;
    std::vector<Preset> workspacePresets_;
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PropertyPresetType p) {
    switch (p) {
        case PropertyPresetType::Property:
            ss << "Property";
            break;
        case PropertyPresetType::Workspace:
            ss << "Workspace";
            break;
        case PropertyPresetType::Application:
            ss << "Application";
            break;
        default:
            break;
    }
    return ss;
}
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             PropertyPresetTypes ps) {
    std::copy(ps.begin(), ps.end(), util::make_ostream_joiner(ss, ", "));
    return ss;
}

}  // namespace inviwo

#endif  // IVW_PROPERTYPRESETMANAGER_H
