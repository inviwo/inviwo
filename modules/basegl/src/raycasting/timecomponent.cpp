/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/raycasting/timecomponent.h>
#include <modules/opengl/shader/shader.h>

#include <chrono>
#include <functional>
#include <fmt/format.h>

namespace inviwo {

TimeComponent::TimeComponent(std::string_view name,
                             std::function<void(InvalidationLevel)> invalidate)
    : RaycasterComponent{}
    , timer{std::chrono::milliseconds{33},
            [invalidate = std::move(invalidate)]() {
                invalidate(InvalidationLevel::InvalidOutput);
            }}
    , name_{name} {

    timer.start();
}

std::string_view TimeComponent::getName() const { return name_; }

void TimeComponent::process(Shader& shader, TextureUnitContainer&) {
    shader.setUniform(name_, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
                                 std::chrono::steady_clock::now().time_since_epoch())
                                 .count());
}

auto TimeComponent::getSegments() -> std::vector<Segment> {
    return {Segment{fmt::format(FMT_STRING("uniform float {};"), name_), Segment::uniform, 600}};
}

}  // namespace inviwo
