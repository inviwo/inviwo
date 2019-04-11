/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/util/metadatatoproperty.h>

#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadatamap.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

namespace util {

struct RegisterOrdinalPropertyForMetaData {
    template <typename T>
    auto operator()(
        std::unordered_map<std::string, std::function<void(const std::string& key, const MetaData*,
                                                           CompositeProperty&)>>& factory) {

        factory[MetaDataPrimitiveType<T, 0, 0>{}.getClassIdentifier()] =
            [](const std::string& key, const MetaData* meta, CompositeProperty& container) {
                auto m = static_cast<const MetaDataPrimitiveType<T, 0, 0>*>(meta);

                if (auto existingProperty = container.getPropertyByIdentifier(key)) {
                    if (auto p = dynamic_cast<OrdinalProperty<T>*>(existingProperty)) {
                        p->set(m->get());
                        p->setVisible(true);
                        return;
                    } else {
                        delete container.removeProperty(existingProperty);
                    }
                }

                using value_type = typename util::glmtype<T>::type;

                T min = T{0} + std::numeric_limits<value_type>::lowest();
                T max = T{0} + std::numeric_limits<value_type>::max();
                T inc = T{0} + std::numeric_limits<value_type>::lowest();

                auto p = std::make_unique<OrdinalProperty<T>>(key, key, m->get(), min, max, inc,
                                                              InvalidationLevel::Valid,
                                                              PropertySemantics::Text);
                p->setSerializationMode(PropertySerializationMode::All);
                p->setCurrentStateAsDefault();
                p->setReadOnly(true);
                container.addProperty(p.release(), true);
            };
    }
};

MetaDataToProperty::MetaDataToProperty() {
    using ordinalMetaTypes =
        std::tuple<int, float, double, vec2, vec3, vec4, dvec2, dvec3, dvec4, ivec2, ivec3, ivec4,
                   uvec2, uvec3, uvec4, mat2, mat3, mat4, dmat2, dmat3, dmat4>;
    util::for_each_type<ordinalMetaTypes>{}(RegisterOrdinalPropertyForMetaData{}, factory_);

    factory_[MetaDataPrimitiveType<bool, 0, 0>{}.getClassIdentifier()] =
        [](const std::string& key, const MetaData* meta, CompositeProperty& container) {
            auto m = static_cast<const MetaDataPrimitiveType<bool, 0, 0>*>(meta);

            if (auto existingProperty = container.getPropertyByIdentifier(key)) {
                if (auto p = dynamic_cast<BoolProperty*>(existingProperty)) {
                    p->set(m->get());
                    p->setVisible(true);
                    return;
                } else {
                    delete container.removeProperty(existingProperty);
                }
            }

            auto p = std::make_unique<BoolProperty>(key, key, m->get(), InvalidationLevel::Valid);
            p->setSerializationMode(PropertySerializationMode::All);
            p->setCurrentStateAsDefault();
            p->setReadOnly(true);
            container.addProperty(p.release(), true);
        };

    factory_[MetaDataPrimitiveType<std::string, 0, 0>{}.getClassIdentifier()] =
        [](const std::string& key, const MetaData* meta, CompositeProperty& container) {
            auto m = static_cast<const MetaDataPrimitiveType<std::string, 0, 0>*>(meta);

            if (auto existingProperty = container.getPropertyByIdentifier(key)) {
                if (auto p = dynamic_cast<StringProperty*>(existingProperty)) {
                    p->set(m->get());
                    p->setVisible(true);
                    return;
                } else {
                    delete container.removeProperty(existingProperty);
                }
            }
            auto p = std::make_unique<StringProperty>(key, key, m->get(), InvalidationLevel::Valid);
            p->setSerializationMode(PropertySerializationMode::All);
            p->setCurrentStateAsDefault();
            p->setReadOnly(true);
            container.addProperty(p.release(), true);
        };
}

void MetaDataToProperty::updateProperty(CompositeProperty& parent, const MetaDataMap* metaDataMap) {
    auto keys = metaDataMap->getKeys();
    for (auto key : keys) {
        auto meta = metaDataMap->get(key);
        auto it = factory_.find(meta->getClassIdentifier());
        if (it != factory_.end()) {
            it->second(key, meta, parent);
        } else {
            LogErrorCustom("MetaDataToProperty", "Unsupported MetaData type");
        }
    }

    // Remove unused meta data properties
    std::vector<std::string> ids;
    for (auto p : parent.getProperties()) {
        ids.push_back(p->getIdentifier());
    }
    for (auto id : ids) {
        if (!metaDataMap->get(id)) {
            delete parent.removeProperty(id);
        }
    }
}

}  // namespace util

}  // namespace inviwo
