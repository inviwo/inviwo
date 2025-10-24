/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/processors/layercreator.h>

#include <inviwo/core/util/formatdispatching.h>
#include <modules/base/algorithm/image/layergeneration.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerCreator::processorInfo_{
    "org.inviwo.LayerCreator",  // Class identifier
    "Layer Creator",            // Display name
    "Data Creation",            // Category
    CodeState::Stable,          // Code state
    Tags::CPU,                  // Tags
    R"(Procedurally generate Layer data on the CPU.)"_unindentHelp,
};

const ProcessorInfo& LayerCreator::getProcessorInfo() const { return processorInfo_; }

LayerCreator::LayerCreator()
    : Processor{}
    , outport_("layer", "The generated Layer"_help)
    , type_{"type",
            "Type",
            R"(
            Type of Layer to generate:
             * __Single Voxel__ Center texel equal to 'value', all others are set to zero
             * __Radial__ Radially symmetric density centered in the Layer decaying with
                          the distance from the center
             * __Ripple__ A quickly oscillating density between 0 and 1
             * __Uniform Value__ A constant 'value' in the entire Layer)"_unindentHelp,
            {{"singleVoxel", "Single Voxel", Type::SingleVoxel},
             {"radial", "Radial", Type::Radial},
             {"ripple", "Ripple", Type::Ripple},
             {"uniform", "Uniform Value", Type::Uniform}}}
    , format_{"format", "Format",
              OptionPropertyState<DataFormatId>{
                  .options =
                      [&]() {
                          std::vector<OptionPropertyOption<DataFormatId>> formats;
                          util::for_each_type<DefaultDataFormats>{}([&formats]<typename Format>() {
                              formats.emplace_back(Format::str(), Format::str(), Format::id());
                          });
                          return formats;
                      }(),
                  .help = "Layer data format"_help}
                  .setSelectedValue(DataFloat32::id())}
    , dimensions_{"dims", "Dimensions",
                  util::ordinalCount(size2_t{10}, size2_t{512}).set("Layer Dimensions"_help)}
    , value_{"value", "Value",
             util::ordinalSymmetricVector(dvec4{1.0}, dvec4{100.0})
                 .set("Value for use in some layers"_help)}
    , information_("Information", "Data information")
    , basis_("Basis", "Basis and offset") {

    addPort(outport_);
    addProperties(type_, format_, dimensions_, value_, information_, basis_);

    information_.setChecked(true);
    information_.setCurrentStateAsDefault();
}

void LayerCreator::process() {
    if (util::any_of(util::ref<Property>(format_, type_, dimensions_), &Property::isModified)) {
        loadedData_ = dispatching::singleDispatch<std::shared_ptr<Layer>, dispatching::filter::All>(
            format_.get(), [&]<typename T>() {
                using enum LayerCreator::Type;
                switch (type_.get()) {
                    case SingleVoxel:
                        return std::shared_ptr<Layer>(
                            util::makeSingleTexelLayer<T>(dimensions_.get(), value_.get()));
                    case Radial:
                        return std::shared_ptr<Layer>(util::makeRadialLayer<T>(dimensions_.get()));
                    case Ripple:
                        return std::shared_ptr<Layer>(util::makeRippleLayer<T>(dimensions_.get()));
                    case Uniform:
                        return std::shared_ptr<Layer>(
                            util::makeUniformLayer<T>(dimensions_.get(), value_.get()));
                    default:
                        return std::shared_ptr<Layer>{};
                }
            });

        basis_.updateForNewEntity(*loadedData_, deserialized_);
        information_.updateForNewLayer(
            *loadedData_, deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No);
        deserialized_ = false;
    }

    auto layer = std::make_shared<Layer>(*loadedData_);
    basis_.updateEntity(*layer);
    information_.updateLayer(*layer);
    outport_.setData(layer);
}

void LayerCreator::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

}  // namespace inviwo
