/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/base/processors/volumesubset.h>

#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/network/networklock.h>                            // for NetworkLock
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport, Vol...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/minmaxproperty.h>                      // for IntSizeTMinMaxPro...
#include <inviwo/core/properties/valuewrapper.h>                        // for PropertySerializa...
#include <inviwo/core/util/glmmat.h>                                    // for mat3
#include <inviwo/core/util/glmvec.h>                                    // for vec3, size3_t
#include <modules/base/algorithm/volume/volumeramsubset.h>              // for VolumeRAMSubSet

#include <functional>     // for __base
#include <memory>         // for shared_ptr, share...
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <glm/detail/qualifier.hpp>  // for tvec2
#include <glm/mat3x3.hpp>            // for mat<>::col_type
#include <glm/vec2.hpp>              // for vec<>::(anonymous)
#include <glm/vec3.hpp>              // for operator/, operator+

namespace inviwo {

const ProcessorInfo VolumeSubset::processorInfo_{
    "org.inviwo.VolumeSubset",  // Class identifier
    "Volume Subset",            // Display name
    "Volume Operation",         // Category
    CodeState::Stable,          // Code state
    Tags::CPU,                  // Tags
};
const ProcessorInfo VolumeSubset::getProcessorInfo() const { return processorInfo_; }

VolumeSubset::VolumeSubset()
    : Processor()
    , inport_("inputVolume")
    , outport_("outputVolume")
    , enabled_("enabled", "Enable Operation", true)
    , adjustBasisAndOffset_("adjustBasisAndOffset", "Adjust Basis and Offset", true)
    , rangeX_("rangeX", "X Slices", 0, 256, 0, 256, 1, 1)
    , rangeY_("rangeY", "Y Slices", 0, 256, 0, 256, 1, 1)
    , rangeZ_("rangeZ", "Z Slices", 0, 256, 0, 256, 1, 1) {
    addPort(inport_);
    addPort(outport_);
    addProperty(enabled_);
    addProperty(adjustBasisAndOffset_);
    addProperty(rangeX_);
    addProperty(rangeY_);
    addProperty(rangeZ_);
    dims_ = size3_t(1, 1, 1);

    // Since the ranges depend on the input volume dimensions, we make sure to always
    // serialize them so we can do a proper renormalization when we load new data.
    rangeX_.setSerializationMode(PropertySerializationMode::All);
    rangeY_.setSerializationMode(PropertySerializationMode::All);
    rangeZ_.setSerializationMode(PropertySerializationMode::All);

    inport_.onChange([this]() {
        NetworkLock lock(this);

        // Update to the new dimensions.
        dims_ = inport_.getData()->getDimensions();

        rangeX_.setRangeNormalized(size2_t(0, dims_.x));
        rangeY_.setRangeNormalized(size2_t(0, dims_.y));
        rangeZ_.setRangeNormalized(size2_t(0, dims_.z));

        // set the new dimensions to default if we were to press reset
        rangeX_.setCurrentStateAsDefault();
        rangeY_.setCurrentStateAsDefault();
        rangeZ_.setCurrentStateAsDefault();
    });
}

VolumeSubset::~VolumeSubset() = default;

void VolumeSubset::process() {
    if (enabled_.get()) {
        const auto vol = inport_.getData()->getRepresentation<VolumeRAM>();
        const size3_t offset{rangeX_.get().x, rangeY_.get().x, rangeZ_.get().x};
        const size3_t dim = size3_t{rangeX_.get().y, rangeY_.get().y, rangeZ_.get().y} - offset;

        if (dim == dims_)
            outport_.setData(inport_.getData());
        else {
            auto volume = std::make_shared<Volume>(*inport_.getData(), NoData{});
            volume->addRepresentation(VolumeRAMSubSet::apply(vol, dim, offset));

            if (adjustBasisAndOffset_.get()) {
                vec3 volOffset = inport_.getData()->getOffset();
                mat3 volBasis = inport_.getData()->getBasis();

                const vec3 newOffset =
                    volOffset + volBasis * (static_cast<vec3>(offset) / static_cast<vec3>(dims_));

                mat3 newBasis = volBasis;
                vec3 dimRatio = (static_cast<vec3>(dim) / static_cast<vec3>(dims_));
                newBasis[0] *= dimRatio[0];
                newBasis[1] *= dimRatio[1];
                newBasis[2] *= dimRatio[2];

                volume->setBasis(newBasis);
                volume->setOffset(newOffset);
            }
            outport_.setData(volume);
        }
    } else {
        outport_.setData(inport_.getData());
    }
}

}  // namespace inviwo
