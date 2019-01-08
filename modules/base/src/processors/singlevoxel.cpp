/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#include <inviwo/core/util/volumesampler.h>
#include <modules/base/processors/singlevoxel.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SingleVoxel::processorInfo_{
    "org.inviwo.SingleVoxel",  // Class identifier
    "Single Voxel",            // Display name
    "Information",             // Category
    CodeState::Experimental,   // Code state
    Tags::None,                // Tags
};

const ProcessorInfo SingleVoxel::getProcessorInfo() const { return processorInfo_; }

SingleVoxel::SingleVoxel()
    : Processor()
    , volume_("volume")
    , position_("position", "Position", dvec3(0.0), dvec3(0.0), dvec3(1.0))
    , doubleProperty_("doubleProperty", "Voxel value", 0, std::numeric_limits<double>::lowest(),
                      std::numeric_limits<double>::max(), 0.0001, InvalidationLevel::Valid,
                      PropertySemantics::Text)
    , dvec2Property_("dvec2Property", "Voxel value", dvec2(0),
                     dvec2(std::numeric_limits<double>::lowest()),
                     dvec2(std::numeric_limits<double>::max()), dvec2(0.0001),
                     InvalidationLevel::Valid, PropertySemantics::Text)
    , dvec3Property_("dvec3Property", "Voxel value", dvec3(0),
                     dvec3(std::numeric_limits<double>::lowest()),
                     dvec3(std::numeric_limits<double>::max()), dvec3(0.0001),
                     InvalidationLevel::Valid, PropertySemantics::Text)
    , dvec4Property_("dvec4Property", "Voxel value", dvec4(0),
                     dvec4(std::numeric_limits<double>::lowest()),
                     dvec4(std::numeric_limits<double>::max()), dvec4(0.0001),
                     InvalidationLevel::Valid, PropertySemantics::Text)
    , space_("space", "Space") {
    addPort(volume_);
    addProperty(space_);
    addProperty(position_);
    addProperty(doubleProperty_);
    addProperty(dvec2Property_);
    addProperty(dvec3Property_);
    addProperty(dvec4Property_);

    doubleProperty_.setVisible(true);
    dvec2Property_.setVisible(false);
    dvec3Property_.setVisible(false);
    dvec4Property_.setVisible(false);

    doubleProperty_.setSemantics(PropertySemantics("Text"));
    dvec2Property_.setSemantics(PropertySemantics("Text"));
    dvec3Property_.setSemantics(PropertySemantics("Text"));
    dvec4Property_.setSemantics(PropertySemantics("Text"));

    space_.addOption("model", "Model", CoordinateSpace::Model);
    space_.addOption("world", "World", CoordinateSpace::World);
    space_.addOption("data", "Data", CoordinateSpace::Data);

    setAllPropertiesCurrentStateAsDefault();
}

void SingleVoxel::process() {
    auto vol = volume_.getData();
    auto df = vol->getDataFormat();

    auto comps = df->getComponents();
    doubleProperty_.setVisible(comps == 1);
    dvec2Property_.setVisible(comps == 2);
    dvec3Property_.setVisible(comps == 3);
    dvec4Property_.setVisible(comps == 4);

    if (comps == 1) {
        VolumeDoubleSampler<1> sampler(vol);
        auto sample = sampler.sample(position_, space_.get());
        doubleProperty_.set(sample);
    }
    if (comps == 2) {
        VolumeDoubleSampler<2> sampler(vol);
        auto sample = sampler.sample(position_, space_.get());
        dvec2Property_.set(sample);
    }
    if (comps == 3) {
        VolumeDoubleSampler<3> sampler(vol);
        auto sample = sampler.sample(position_, space_.get());
        dvec3Property_.set(sample);
    }
    if (comps == 4) {
        VolumeDoubleSampler<4> sampler(vol);
        auto sample = sampler.sample(position_, space_.get());
        dvec4Property_.set(sample);
    }
}

}  // namespace inviwo
