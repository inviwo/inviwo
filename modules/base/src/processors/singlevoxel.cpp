/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/coordinatetransformer.h>  // for CoordinateSpace, Coordinat...
#include <inviwo/core/ports/volumeport.h>                      // for VolumeInport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>              // for Tags, Tags::CPU
#include <inviwo/core/properties/invalidationlevel.h>          // for InvalidationLevel, Invalid...
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>            // for DoubleVec3Property, Double...
#include <inviwo/core/properties/propertysemantics.h>          // for PropertySemantics, Propert...
#include <inviwo/core/util/formats.h>                          // for DataFormatBase
#include <inviwo/core/util/glmutils.h>                         // for Vector
#include <inviwo/core/util/glmvec.h>                           // for dvec3, dvec2, dvec4
#include <inviwo/core/util/staticstring.h>                     // for operator+
#include <inviwo/core/util/volumesampler.h>                    // for VolumeDoubleSampler
#include <modules/base/processors/singlevoxel.h>               // for SingleVoxel

#include <functional>   // for __base
#include <limits>       // for numeric_limits
#include <memory>       // for shared_ptr
#include <string>       // for operator==, string
#include <string_view>  // for string_view, operator==
#include <type_traits>  // for remove_extent_t
#include <vector>       // for operator!=, vector, operat...

#include <glm/common.hpp>  // for mix
#include <glm/mat4x4.hpp>  // for operator*
#include <glm/vec2.hpp>    // for operator*, operator+
#include <glm/vec3.hpp>    // for operator/, operator*, oper...
#include <glm/vec4.hpp>    // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SingleVoxel::processorInfo_{
    "org.inviwo.SingleVoxel",  // Class identifier
    "Single Voxel",            // Display name
    "Information",             // Category
    CodeState::Stable,         // Code state
    Tags::CPU,                 // Tags
    "Sample a volume at a given position and output the result in a property."_help};

const ProcessorInfo& SingleVoxel::getProcessorInfo() const { return processorInfo_; }

SingleVoxel::SingleVoxel()
    : Processor()
    , volume_("volume")
    , position_("position", "Position",
                util::ordinalSymmetricVector(dvec3{0.0}, dvec3{1.0}).set("sampling position"_help))
    , doubleProperty_(
          "doubleProperty", "Voxel value",
          util::ordinalSymmetricVector(0.0)
              .setInc(0.0001)
              .set("resulting sample (number of channels depending on input volume)"_help)
              .set(InvalidationLevel::Valid)
              .set(PropertySemantics::Text))
    , dvec2Property_(
          "dvec2Property", "Voxel value",
          util::ordinalSymmetricVector(dvec2{0.0})
              .setInc(dvec2{0.0001})
              .set("resulting sample (number of channels depending on input volume)"_help)
              .set(InvalidationLevel::Valid)
              .set(PropertySemantics::Text))

    , dvec3Property_(
          "dvec3Property", "Voxel value",
          util::ordinalSymmetricVector(dvec3{0.0})
              .setInc(dvec3{0.0001})
              .set("resulting sample (number of channels depending on input volume)"_help)
              .set(InvalidationLevel::Valid)
              .set(PropertySemantics::Text))
    , dvec4Property_(
          "dvec4Property", "Voxel value",
          util::ordinalSymmetricVector(dvec4{0.0})
              .setInc(dvec4{0.0001})
              .set("resulting sample (number of channels depending on input volume)"_help)
              .set(InvalidationLevel::Valid)
              .set(PropertySemantics::Text))
    , space_("space", "Space", "domain of sample position (model, world, or data)"_help,
             {{"model", "Model", CoordinateSpace::Model},
              {"world", "World", CoordinateSpace::World},
              {"data", "Data", CoordinateSpace::Data}}) {

    addPort(volume_);
    addProperties(space_, position_, doubleProperty_, dvec2Property_, dvec3Property_,
                  dvec4Property_);

    doubleProperty_.setVisible(true);
    dvec2Property_.setVisible(false);
    dvec3Property_.setVisible(false);
    dvec4Property_.setVisible(false);

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
