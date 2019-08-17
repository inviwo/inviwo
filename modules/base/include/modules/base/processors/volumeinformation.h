/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_VOLUMEINFORMATION_H
#define IVW_VOLUMEINFORMATION_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/util/metadatatoproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>
#include <modules/base/properties/imageinformationproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeInformation, Volume Information}
 * ![](org.inviwo.VolumeInformation.png?classIdentifier=org.inviwo.VolumeInformation)
 * Shows available information provided by input volume including metadata.
 *
 * ### Inports
 *   * __volume__   input volume
 */

/**
 * \class VolumeInformation
 * \brief provides information on input volume
 */
class IVW_MODULE_BASE_API VolumeInformation : public Processor {
public:
    VolumeInformation();
    virtual ~VolumeInformation() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volume_;

    VolumeInformationProperty volumeInfo_;

    IntSizeTProperty significantVoxels_;
    DoubleProperty significantVoxelsRatio_;

    DoubleMinMaxProperty minMaxChannel1_;
    DoubleMinMaxProperty minMaxChannel2_;
    DoubleMinMaxProperty minMaxChannel3_;
    DoubleMinMaxProperty minMaxChannel4_;

    FloatMat4Property worldTransform_;
    FloatMat3Property basis_;
    FloatVec3Property offset_;

    BoolCompositeProperty perVoxelProperties_;
    CompositeProperty transformations_;
    CompositeProperty metaDataProperty_;

    DoubleVec3Property voxelSize_;

    util::MetaDataToProperty metaDataProps_;
};

}  // namespace inviwo

#endif  // IVW_VOLUMEINFORMATION_H
