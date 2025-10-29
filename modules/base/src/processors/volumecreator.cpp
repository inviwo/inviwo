/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/base/processors/volumecreator.h>

#include <inviwo/core/datastructures/volume/volume.h>           // for Volume
#include <inviwo/core/ports/volumeport.h>                       // for VolumeOutport
#include <inviwo/core/processors/processor.h>                   // for Processor
#include <inviwo/core/processors/processorinfo.h>               // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>              // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>               // for Tags, Tags::CPU
#include <inviwo/core/properties/optionproperty.h>              // for OptionPropertyOption, Opt...
#include <inviwo/core/properties/ordinalproperty.h>             // for IntProperty, IntSize3Prop...
#include <inviwo/core/properties/property.h>                    // for Property, OverwriteState
#include <inviwo/core/util/foreacharg.h>                        // for for_each_type
#include <inviwo/core/util/formatdispatching.h>                 // for dispatch, All
#include <inviwo/core/util/formats.h>                           // for DataFormatId, DataFormat
#include <inviwo/core/util/glmvec.h>                            // for size3_t
#include <inviwo/core/util/staticstring.h>                      // for operator+
#include <inviwo/core/util/stdextensions.h>                     // for any_of, ref
#include <modules/base/algorithm/volume/volumegeneration.h>     // for makeMarchingCubeVolume
#include <modules/base/properties/basisproperty.h>              // for BasisProperty
#include <modules/base/properties/volumeinformationproperty.h>  // for VolumeInformationProperty

#include <array>    // for array
#include <cstddef>  // for size_t

#include <glm/common.hpp>              // for max, min
#include <glm/gtx/component_wise.hpp>  // for compMax, compMin
#include <glm/gtx/hash.hpp>            // for hash<>::operator()

namespace inviwo {
class Deserializer;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeCreator::processorInfo_{
    "org.inviwo.VolumeCreator",  // Class identifier
    "Volume Creator",            // Display name
    "Data Creation",             // Category
    CodeState::Stable,           // Code state
    Tags::CPU,                   // Tags
    R"(Procedurally generate volume data on the CPU.)"_unindentHelp};
const ProcessorInfo& VolumeCreator::getProcessorInfo() const { return processorInfo_; }

VolumeCreator::VolumeCreator()
    : Processor()
    , outport_("volume", "The generated volume"_help)
    , type_{"type",
            "Type",
            R"(
            Type of volume to generate:
             * __Single Voxel__ Center voxel equal to 'value' all other 0
             * __Sphere__ Spherically symmetric density centered in the volume decaying radially with
                          the distance from the center
             * __Ripple__ A quickly oscillating density between 0 and 1
             * __Marching Cube__ A 2x2x2 volume corresponding to a marching cube case
             * __Uniform Value__ A constant 'value' in the entire volume
             * __Tornado__ A tornado vector field)"_unindentHelp,
            {{"singleVoxel", "Single Voxel", Type::SingleVoxel},
             {"sphere", "Sphere", Type::Sphere},
             {"ripple", "Ripple", Type::Ripple},
             {"marchingCube", "Marching Cube", Type::MarchingCube},
             {"uniform", "Uniform Value", Type::Uniform},
             {"tornado", "Tornado", Type::Tornado}}}
    , format_{"format", "Format",
              OptionPropertyState<DataFormatId>{
                  .options =
                      [&]() {
                          std::vector<OptionPropertyOption<DataFormatId>> formats;
                          util::for_each_type<DefaultDataFormats>{}([&]<typename Format>() {
                              formats.emplace_back(Format::str(), Format::str(), Format::id());
                          });
                          return formats;
                      }(),
                  .help = "Volume data format"_help}
                  .setSelectedValue(DataFloat32::id())}
    , dimensions_("dims", "Dimensions",
                  util::ordinalCount(size3_t(10), size3_t(512)).set("Volume Dimensions"_help))
    , value_{"value", "Value",
             util::ordinalSymmetricVector(dvec4{1.0}, dvec4{100.0})
                 .set("Value for use in some volumes"_help)}
    , index_("index", "Index",
             util::ordinalCount(5, 255)
                 .set("Marching cube case index"_help)
                 .setMax(ConstraintBehavior::Immutable))
    , information_("Information", "Data information")
    , basis_("Basis", "Basis and offset") {

    addPort(outport_);
    addProperties(type_, format_, dimensions_, value_, index_, information_, basis_);

    information_.setChecked(true);
    information_.setCurrentStateAsDefault();
}

void VolumeCreator::process() {
    if (util::any_of(util::ref<Property>(format_, type_, dimensions_, index_),
                     &Property::isModified)) {
        loadedData_ =
            dispatching::singleDispatch<std::shared_ptr<Volume>, dispatching::filter::All>(
                format_.get(), [&]<typename T>() {
                    using enum VolumeCreator::Type;
                    switch (type_.get()) {
                        case SingleVoxel:
                            return std::shared_ptr<Volume>(
                                util::makeSingleVoxelVolume<T>(dimensions_.get(), value_.get()));
                        case Sphere:
                            return std::shared_ptr<Volume>(
                                util::makeSphericalVolume<T>(dimensions_.get()));
                        case Ripple:
                            return std::shared_ptr<Volume>(
                                util::makeRippleVolume<T>(dimensions_.get()));
                        case MarchingCube:
                            return std::shared_ptr<Volume>(
                                util::makeMarchingCubeVolume<T>(index_.get()));
                        case Uniform:
                            return std::shared_ptr<Volume>(
                                util::makeUniformVolume<T>(dimensions_.get(), value_.get()));
                        case Tornado:
                            return std::shared_ptr<Volume>(
                                util::makeTornadoVolume(dimensions_.get(), index_.get()));
                        default:
                            return std::shared_ptr<Volume>{};
                    }
                });

        basis_.updateForNewEntity(*loadedData_, deserialized_);
        information_.updateForNewVolume(
            *loadedData_, deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No);
        deserialized_ = false;
    }

    auto volume = std::make_shared<Volume>(*loadedData_);
    basis_.updateEntity(*volume);
    information_.updateVolume(*volume);
    outport_.setData(volume);
}

void VolumeCreator::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

}  // namespace inviwo
