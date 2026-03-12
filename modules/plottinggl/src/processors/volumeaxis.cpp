/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <modules/plottinggl/processors/volumeaxis.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/foreacharg.h>
#include <modules/opengl/texture/textureutils.h>

#include <fmt/core.h>

#include <ranges>

namespace inviwo::plot {

namespace {

std::optional<int> labelScaleStep(VolumeAxis::LabelScale scale) {
    switch (scale) {
        case VolumeAxis::LabelScale::None:
            return std::nullopt;
        case VolumeAxis::LabelScale::Tens:
            return 1;
        case VolumeAxis::LabelScale::Thousands:
            return 3;
    }
    return std::nullopt;
}

std::string formatExponent(int exp) {
    constexpr std::array<std::string_view, 10> powers = {"\u2070", "\u00B9", "\u00B2", "\u00B3",
                                                         "\u2074", "\u2075", "\u2076", "\u2077",
                                                         "\u2078", "\u2079"};
    constexpr std::string_view minus = "\u207B";

    std::string res{};
    if (exp == 0) return res;
    res += "10";

    if (exp < 0) {
        res += minus;
        exp = -exp;
    }
    while (exp != 0) {
        res += powers[exp % 10];
        exp /= 10;
    }
    res += ' ';
    return res;
}

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeAxis::processorInfo_{
    "org.inviwo.VolumeAxis",     // Class identifier
    "Volume Axis",               // Display name
    "Plotting",                  // Category
    CodeState::Stable,           // Code state
    Tags::GL | Tag{"Plotting"},  // Tags
    "Renders x, y, and z axes next to the input volume."_help};

const ProcessorInfo& VolumeAxis::getProcessorInfo() const { return processorInfo_; }

VolumeAxis::VolumeAxis()
    : Processor()
    , inport_{"volume", "Input volume"_help}
    , imageInport_{"imageInport", "Background image (optional)"_help}
    , outport_{"outport",
               "Output image containing the rendered volume axes and the optional input image"_help}
    , captionType_("captionType", "Caption Type",
                   {{"string", "Caption String", CaptionType::String},
                    {"data", "Caption from Data", CaptionType::Data},
                    {"custom", "Custom Format (example '{n}{u: [}')", CaptionType::Custom}},
                   0)
    , customCaption_("customCaption", "Custom Caption", "{n}{u: [}")
    , labelScale_{"labelScale",
                  "Label Scaling",
                  {{"none", "None", LabelScale::None},
                   {"tens", "Tens", LabelScale::Tens},
                   {"thousands", "Thousands", LabelScale::Thousands}},
                  0}
    , axisHelper_{util::boundingBox(inport_)} {

    imageInport_.setOptional(true);

    addPorts(inport_, imageInport_, outport_);

    util::for_each_in_tuple(
        [&](Property& p) {
            if (p.getIdentifier() == "visibility") {
                addProperties(captionType_, customCaption_, labelScale_);
            }
            addProperty(p);
        },
        axisHelper_.props());

    captionType_.onChange([this]() {
        if (imageInport_.hasData()) {
            exps_ = axisHelper_.adjustRanges(inport_.getData().get(),
                                             labelScaleStep(labelScale_.get()));
        }
        updateCaptions();
    });
    labelScale_.onChange([this]() {
        if (imageInport_.hasData()) {
            exps_ = axisHelper_.adjustRanges(inport_.getData().get(),
                                             labelScaleStep(labelScale_.get()));
        }
        updateCaptions();
    });
    customCaption_.onChange([this]() {
        if (captionType_.get() == CaptionType::Custom) {
            updateCaptions();
        }
    });

    // adjust scaling factor for label offsets and tick lengths
    axisHelper_.offsetScaling_.onChange(
        [&]() { axisHelper_.adjustScalingFactor(inport_.getData().get()); });

    // adjust axis ranges when input mesh, i.e. its basis, changes
    inport_.onChange([&]() {
        axisHelper_.adjustScalingFactor(inport_.getData().get());
        exps_ =
            axisHelper_.adjustRanges(inport_.getData().get(), labelScaleStep(labelScale_.get()));
        updateCaptions();
    });
    // sync ranges when custom range is enabled or disabled
    axisHelper_.rangeMode_.onChange([this]() {
        if (imageInport_.hasData()) {
            exps_ = axisHelper_.adjustRanges(inport_.getData().get(),
                                             labelScaleStep(labelScale_.get()));
        }
        updateCaptions();
    });

    setAllPropertiesCurrentStateAsDefault();
}

void VolumeAxis::process() {
    if (imageInport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_, ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    axisHelper_.renderAxes(outport_.getDimensions(), *inport_.getData());

    utilgl::deactivateCurrentTarget();
}

void VolumeAxis::updateCaptions() {

    auto scaleUnit = [&](Unit unit, int exp) {
        switch (labelScale_.get()) {
            case LabelScale::None:
                return unit;
            case LabelScale::Tens:
                [[fallthrough]];
            case LabelScale::Thousands:
                [[fallthrough]];
            default:
                return Unit{std::pow(10, exp), unit};
        }
    };

    switch (captionType_.get()) {
        case CaptionType::Data:
            if (auto volume = inport_.getData()) {
                for (auto&& [prop, axis, exp] :
                     std::views::zip(std::to_array({&axisHelper_.xAxis_, &axisHelper_.yAxis_,
                                                    &axisHelper_.zAxis_}),
                                     volume->axes, std::span(glm::value_ptr(exps_), 3))) {
                    prop->captionSettings_.title_.set(
                        fmt::format("{}{: [}", axis.name, scaleUnit(axis.unit, exp)));
                }
            }
            break;
        case CaptionType::Custom:
            if (auto volume = inport_.getData()) {
                try {
                    for (auto&& [prop, axis, exp] :
                         std::views::zip(std::to_array({&axisHelper_.xAxis_, &axisHelper_.yAxis_,
                                                        &axisHelper_.zAxis_}),
                                         volume->axes, std::span(glm::value_ptr(exps_), 3))) {

                        prop->captionSettings_.title_.set(fmt::format(
                            fmt::runtime(customCaption_.get()), fmt::arg("n", axis.name),
                            fmt::arg("u", axis.unit), fmt::arg("su", scaleUnit(axis.unit, exp)),
                            fmt::arg("s", formatExponent(exp)), fmt::arg("e", exp)));
                    }
                } catch (const fmt::format_error& e) {
                    log::error("Invalid custom caption format: {}: {}", customCaption_.get(),
                               e.what());
                }
            }
            break;
        case CaptionType::String:
        default:
            break;
    }
}

}  // namespace inviwo::plot
