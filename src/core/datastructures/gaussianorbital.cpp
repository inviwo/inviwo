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

#include <inviwo/core/datastructures/gaussianorbital.h>

namespace inviwo {



GaussianOrbital::GaussianOrbital() : p{}, coefs{} {}
GaussianOrbital::GaussianOrbital(const vec4& p, const vec3& coefs)
    : p{p}, coefs{coefs} {}

GaussianOrbital::GaussianOrbital(const GaussianOrbital& other)
    : p{other.p}, coefs{other.coefs} {}

GaussianOrbital::GaussianOrbital(GaussianOrbital&& other) noexcept
    : p{std::move(other.p)}, coefs{std::move(other.coefs)} {}

GaussianOrbital& GaussianOrbital::operator=(GaussianOrbital& other) {
    
    std::swap(p, other.p);
    std::swap(coefs, other.coefs);
    return *this;
}

GaussianOrbital& GaussianOrbital::operator=(GaussianOrbital&& other) noexcept {
    
    p = std::move(other.p);
    coefs = std::move(other.coefs);
    return *this;
}

const uvec3 GaussianOrbital::colorCode = uvec3(188, 101, 101);
const std::string GaussianOrbital::classIdentifier = "org.inviwo.GaussianOrbital";
const std::string GaussianOrbital::dataName = "GaussianOrbital";

    


}  // namespace inviwo
