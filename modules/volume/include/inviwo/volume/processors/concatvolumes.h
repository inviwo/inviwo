/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#pragma once

#include <inviwo/volume/volumemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

/** \docpage{org.inviwo.ConcatVolumes, Concat Volumes}
 * ![](org.inviwo.ConcatVolumes.png?classIdentifier=org.inviwo.ConcatVolumes)
 * Concatenate several slices of volumes along the z axis.
 */
class IVW_MODULE_VOLUME_API ConcatVolumes : public Processor {
public:
    ConcatVolumes();
    virtual ~ConcatVolumes() = default;

    virtual void process() override;
    void deserialize(Deserializer& d);

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

public:
    struct VolumeSlabProperty : public BoolCompositeProperty {
        IntSizeTProperty zRange_;
        VolumeInport volume_;
        ConcatVolumes* parent_ = nullptr;
        static size_t ID;
        static const std::string classIdentifier_;

        VolumeSlabProperty(const std::string& identifier, const std::string& displayName);
        ~VolumeSlabProperty();
        void addParent(ConcatVolumes& parent);
        virtual BoolCompositeProperty* clone() const override;

        virtual std::string getClassIdentifier() const override;
    };

    VolumeOutport outVolume_;
    IntSize2Property xyDimensions_;
    VolumeSlabProperty baseVolume_;
    ListProperty appendVolumes_;
};

}  // namespace inviwo
