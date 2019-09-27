/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_VOLUMEBASISPROPERTY_H
#define IVW_VOLUMEBASISPROPERTY_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/datastructures/spatialdata.h>

namespace inviwo {

/**
 * \ingroup properties
 * A CompositeProperty holding the properties needed to represent a bases matrix.
 */
class IVW_MODULE_BASE_API BasisProperty : public CompositeProperty {
public:
    enum class BasisPropertyMode {
        General,
        Orthogonal,
    };
    enum class BasisPropertyReference { Volume, Voxel };

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    BasisProperty(std::string identifier, std::string displayName,
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);

    void onResetOverride();

    BasisProperty(const BasisProperty& rhs);
    virtual BasisProperty* clone() const override;
    virtual ~BasisProperty() = default;

    void updateForNewEntity(const SpatialEntity<3>& volume, bool deserialize);
    void updateForNewEntity(const StructuredGridEntity<3>& volume, bool deserialize);

    void updateEntity(SpatialEntity<3>& volume);

    mat4 getBasisAndOffset() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual BasisProperty& setCurrentStateAsDefault() override;
    virtual BasisProperty& resetToDefaultState() override;

    TemplateOptionProperty<BasisPropertyMode> mode_;
    TemplateOptionProperty<BasisPropertyReference> reference_;

    BoolProperty overRideDefaults_;
    BoolProperty updateForNewEntiry_;

    FloatVec3Property size_;
    FloatVec3Property a_;
    FloatVec3Property b_;
    FloatVec3Property c_;
    BoolProperty autoCenter_;
    FloatVec3Property offset_;
    ButtonProperty resetOverride_;

private:
    auto props() {
        return std::tie(mode_, reference_, overRideDefaults_, updateForNewEntiry_, size_, a_, b_,
                        c_, autoCenter_, offset_, resetOverride_);
    }
    auto props() const {
        return std::tie(mode_, reference_, overRideDefaults_, updateForNewEntiry_, size_, a_, b_,
                        c_, autoCenter_, offset_, resetOverride_);
    }

    void update(const SpatialEntity<3>& volume, bool deserialize);
    void load();
    void save();
    void onModeChange();
    void onOverrideChange();
    void onAutoCenterChange();

    vec3 dimensions_{1.0f};
    mat4 model_{1.0f};
    ValueWrapper<mat4> overrideModel_;
    bool updating_ = false;
};

}  // namespace inviwo

#endif  // IVW_VOLUMEBASISPROPERTY_H
