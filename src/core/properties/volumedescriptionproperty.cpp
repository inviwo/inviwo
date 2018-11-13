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

#include <inviwo/core/properties/volumedescriptionproperty.h>

namespace inviwo {

    PropertyClassIdentifier(VolumeDesriptionMetadataProperty, "org.inviwo.VolumeDesriptionMetadataProperty");

    VolumeDesriptionMetadataProperty::VolumeDesriptionMetadataProperty(
        std::string identifier,
        std::string displayName,
        InvalidationLevel invalidationLevel,
        PropertySemantics semantics) : CompositeProperty(identifier, displayName, invalidationLevel, semantics),
        dimension_{"dimension", "Dimension"},
        format_{"format", "Format"},
        numChannels_{"numchannels", "Num. Channels"},
        numVoxels_{"numvoxels", "Num. Voxels"},
        dataRange_{"datarange", "Data Range"},
        valueRange_{"valuerange", "Value Range"},
        valueUnit_{"valueuunit", "Value Unit"},
        basis_{"basis", "Basis"},
        offset_{"offset", "Offset"}
    {
        addProperty(dimension_);
        addProperty(format_);
        addProperty(numChannels_);
        addProperty(numVoxels_);
        addProperty(dataRange_);
        addProperty(valueRange_);
        addProperty(valueUnit_);
        addProperty(basis_);
        addProperty(offset_);

        dimension_.setReadOnly(true);
        format_.setReadOnly(true);
        numChannels_.setReadOnly(true);
        numVoxels_.setReadOnly(true);
        dataRange_.setReadOnly(true);
        valueRange_.setReadOnly(true);
        valueUnit_.setReadOnly(true);
        basis_.setReadOnly(true);
        offset_.setReadOnly(true);

        dimension_.setSemantics(PropertySemantics::Text);
        numChannels_.setSemantics(PropertySemantics::Text);
        numVoxels_.setSemantics(PropertySemantics::Text);
        dataRange_.setSemantics(PropertySemantics::Text);
        valueRange_.setSemantics(PropertySemantics::Text);
        offset_.setSemantics(PropertySemantics::Text);
    }

    VolumeDesriptionMetadataProperty::VolumeDesriptionMetadataProperty(const VolumeDesriptionMetadataProperty& rhs)
        : CompositeProperty(rhs)
        , dimension_(rhs.dimension_)
        , format_(rhs.format_)
        , numChannels_(rhs.numChannels_)
        , numVoxels_(rhs.numVoxels_)
        , dataRange_(rhs.dataRange_)
        , valueRange_(rhs.valueRange_)
        , valueUnit_(rhs.valueUnit_)
        , basis_(rhs.basis_)
        , offset_(rhs.offset_) {

        addProperty(dimension_);
        addProperty(format_);
        addProperty(numChannels_);
        addProperty(numVoxels_);
        addProperty(dataRange_);
        addProperty(valueRange_);
        addProperty(valueUnit_);
        addProperty(basis_);
        addProperty(offset_);
    }

    VolumeDesriptionMetadataProperty& VolumeDesriptionMetadataProperty::operator=(const VolumeDesriptionMetadataProperty& that) {
        if (this != &that) {
            CompositeProperty::operator=(that);
        }
        return *this;
    }

    VolumeDesriptionMetadataProperty* VolumeDesriptionMetadataProperty::clone() const { return new VolumeDesriptionMetadataProperty(*this); }

    // ##############################################################################################################

    PropertyClassIdentifier(VolumeDesriptionProperty, "org.inviwo.VolumeDesriptionProperty");

    VolumeDesriptionProperty::VolumeDesriptionProperty(
        std::string identifier,
        std::string displayName,
        int volumeIdx,
        const unsigned char* imgData,
        size2_t imgSize,
        InvalidationLevel invalidationLevel,
        PropertySemantics semantics) : CompositeProperty(identifier, displayName, invalidationLevel, semantics),
        name_{"name", "Name"},
        image_{"image", "Image", volumeIdx, imgData, imgSize},
        metadata_{"metadata", "Metadata"}
        //basis_{"basis", "Basis and Offset"},
        //information_{"metadata", "Metadata"}
    {
        addProperty(name_);
        name_.setReadOnly(true);
        addProperty(image_);
        addProperty(metadata_);
        metadata_.setCollapsed(true);
        /*addProperty(basis_);
        basis_.setCollapsed(true);
        addProperty(information_);
        information_.setCollapsed(true);*/
    }

    VolumeDesriptionProperty::VolumeDesriptionProperty(const VolumeDesriptionProperty& rhs)
        : CompositeProperty(rhs)
        , name_(rhs.name_)
        , image_(rhs.image_)
        , metadata_(rhs.metadata_)
        //, basis_(rhs.basis_)
        //, information_(rhs.information_)
    {
        addProperty(name_);
        addProperty(image_);
        addProperty(metadata_);
        //addProperty(basis_);
        //addProperty(information_);
    }

    VolumeDesriptionProperty& VolumeDesriptionProperty::operator=(const VolumeDesriptionProperty& that) {
        if (this != &that) {
            CompositeProperty::operator=(that);
        }
        return *this;
    }

    VolumeDesriptionProperty* VolumeDesriptionProperty::clone() const { return new VolumeDesriptionProperty(*this); }

}  // namespace inviwo
