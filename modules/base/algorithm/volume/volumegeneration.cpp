/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/base/algorithm/volume/volumegeneration.h>
#include <bitset>
namespace inviwo {

std::unique_ptr<Volume> util::makeSingleVoxelVolume(const size3_t& size) {
    const size3_t mid{(size - size3_t{1u}) / size_t{2}};
    return util::generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        if (ind == mid)
            return 1.0f;
        else
            return 0.0f;
    });
}

std::unique_ptr<Volume> util::makeSphericalVolume(const size3_t& size) {
    const vec3 rsize{size};
    const vec3 center = (rsize / 2.0f);
    const float r0 = glm::length(rsize);
    return util::generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        const auto pos = vec3(ind) + vec3{0.5f};
        return r0 / (r0 + glm::length2(center - pos));
    });
}

std::unique_ptr<Volume> util::makeRippleVolume(const size3_t& size) {
    const vec3 rsize{size};
    const vec3 center = (rsize / 2.0f);
    const float r0 = glm::length(rsize);
    return util::generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        const auto pos = vec3(ind) + vec3{0.5f};
        const auto r = glm::length2(center - pos);
        return std::sin(rsize.x * 0.5f * glm::pi<float>() * r / r0);
    });
}

std::unique_ptr<Volume> util::makeMarchingCubeVolume(const size_t& index) {
    std::bitset<8> corners(index);
    const std::array<size3_t, 8> vertices = {
        {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}};
    std::unordered_map<size3_t, float> map;
    for (int i = 0; i < 8; ++i) {
        map[vertices[i]] = corners[i] ? 1.0f : 0.0f;
    }
    return util::generateVolume({2, 2, 2}, mat3(1.0), [&](const size3_t& ind) { return map[ind]; });
}

}  // namespace inviwo
