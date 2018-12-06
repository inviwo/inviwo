/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_VOLUMEDESCRIPTIONPROPERTY_H
#define IVW_VOLUMEDESCRIPTIONPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/imageproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <modules/base/properties/basisproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>

#include <limits>

namespace inviwo {

    class IVW_MODULE_BASE_API VolumeDesriptionMetadataProperty : public CompositeProperty {
    public:
        virtual std::string getClassIdentifier() const override;
        static const std::string classIdentifier;

        VolumeDesriptionMetadataProperty(std::string identifier, std::string displayName,
            InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
            PropertySemantics semantics = PropertySemantics::Default);

        VolumeDesriptionMetadataProperty(const VolumeDesriptionMetadataProperty& rhs);
        VolumeDesriptionMetadataProperty& operator=(const VolumeDesriptionMetadataProperty& that);

        virtual VolumeDesriptionMetadataProperty* clone() const override;
        virtual ~VolumeDesriptionMetadataProperty() = default;

    private:
        IntVec3Property dimension_;
        StringProperty format_;
        IntProperty numChannels_;
        IntProperty numVoxels_;
        IntVec2Property dataRange_;
        IntVec2Property valueRange_;
        StringProperty valueUnit_;
        FloatMat3Property basis_;
        FloatVec3Property offset_;
    };

    class IVW_MODULE_BASE_API VolumeDesriptionProperty : public CompositeProperty {
    public:
        virtual std::string getClassIdentifier() const override;
        static const std::string classIdentifier;

        VolumeDesriptionProperty(std::string identifier, std::string displayName,
            int volumeIdx = std::numeric_limits<int>::infinity(),
            const unsigned char* imgData = nullptr,
            size2_t imgSize = size2_t{0},
            InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
            PropertySemantics semantics = PropertySemantics::Default);

        VolumeDesriptionProperty(const VolumeDesriptionProperty& rhs);
        VolumeDesriptionProperty& operator=(const VolumeDesriptionProperty& that);

        virtual VolumeDesriptionProperty* clone() const override;
        virtual ~VolumeDesriptionProperty() = default;

        StringProperty name_; // ToDo: remove and replace with Name attribute as title of CompositeProperty itself
        ImageProperty image_; // ToDo: rename to thumbnail
        VolumeDesriptionMetadataProperty metadata_;
        /*BasisProperty basis_;
        VolumeInformationProperty information_;*/

    private:
    };

}  // namespace inviwo

#endif  // IVW_VOLUMEDESCRIPTIONPROPERTY_H
