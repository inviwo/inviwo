/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <modules/basegl/raycasting/positionindicatorcomponent.h>

#include <modules/opengl/shader/shaderutils.h>

#include <string_view>
#include <fmt/format.h>

namespace inviwo {

PositionIndicatorComponent::PositionIndicatorComponent()
    : RaycasterComponent(), positionIndicator_("positionindicator", "Position Indicator") {}

std::string PositionIndicatorComponent::getName() const {
    return positionIndicator_.getIdentifier();
}

void PositionIndicatorComponent::setUniforms(Shader &shader, TextureUnitContainer &) const {
    utilgl::setUniforms(shader, positionIndicator_);
}

std::vector<Property *> PositionIndicatorComponent::getProperties() {
    return {&positionIndicator_};
}

auto PositionIndicatorComponent::getSegments() const -> std::vector<Segment> {
    std::string_view code{
        "result = drawPlanes(result, samplePos, rayDirection, tIncr, {}.plane{}, t, tDepth);\n"};

    std::stringstream ss;
    if (positionIndicator_) {
        if (positionIndicator_.plane1_) {
            ss << fmt::format(code, positionIndicator_.getIdentifier(), 1);
        }
        if (positionIndicator_.plane2_) {
            ss << fmt::format(code, positionIndicator_.getIdentifier(), 2);
        }
        if (positionIndicator_.plane3_) {
            ss << fmt::format(code, positionIndicator_.getIdentifier(), 2);
        }
    }
    return {Segment{"#include \"utils/raycastgeometry.glsl\"", Segment::include, 1100},
            Segment{fmt::format("uniform VolumeIndicatorParameters {};",
                                positionIndicator_.getIdentifier()),
                    Segment::uniform, 1100},
            Segment{ss.str(), Segment::loop, 1100}};
}

}  // namespace inviwo
