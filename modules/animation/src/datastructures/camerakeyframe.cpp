/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <modules/animation/datastructures/camerakeyframe.h>

#include <inviwo/core/datastructures/camera/camera.h>             // for Camera
#include <inviwo/core/io/serialization/deserializer.h>            // for Deserializer
#include <inviwo/core/io/serialization/serializationexception.h>  // for SerializationException
#include <inviwo/core/io/serialization/serializer.h>              // for Serializer
#include <inviwo/core/util/glmvec.h>                              // for vec3
#include <modules/animation/datastructures/animationtime.h>       // for Seconds
#include <modules/animation/datastructures/basekeyframe.h>        // for BaseKeyframe

#include <chrono>                                                 // for operator==

#include <glm/geometric.hpp>                                      // for normalize
#include <glm/vec3.hpp>                                           // for operator*, operator-
#include <glm/vector_relational.hpp>                              // for any, notEqual

namespace inviwo {

namespace animation {

CameraKeyframe::CameraKeyframe(Seconds time) : BaseKeyframe{time} {}

CameraKeyframe::CameraKeyframe(Seconds time, const Camera& cam)
    : BaseKeyframe{time}
    , lookFrom_{cam.getLookFrom()}
    , lookTo_{cam.getLookTo()}
    , lookUp_{cam.getLookUp()} {}

CameraKeyframe* CameraKeyframe::clone() const { return new CameraKeyframe(*this); }

const vec3& CameraKeyframe::getLookFrom() const { return lookFrom_; }
const vec3& CameraKeyframe::getLookTo() const { return lookTo_; }
const vec3& CameraKeyframe::getLookUp() const { return lookUp_; }

void CameraKeyframe::setLookFrom(vec3 val) { lookFrom_ = val; }
void CameraKeyframe::setLookTo(vec3 val) { lookTo_ = val; }
void CameraKeyframe::setLookUp(vec3 val) { lookUp_ = val; }

vec3 CameraKeyframe::getDirection() const { return lookTo_ - lookFrom_; }

void CameraKeyframe::updateFrom(const Camera& cam) {
    lookFrom_ = cam.getLookFrom();
    lookTo_ = cam.getLookTo();
    lookUp_ = glm::normalize(cam.getLookUp());
}

void CameraKeyframe::serialize(Serializer& s) const {
    BaseKeyframe::serialize(s);
    s.serialize("lookFrom", lookFrom_);
    s.serialize("lookTo", lookTo_);
    s.serialize("lookUp", lookUp_);
}

void CameraKeyframe::deserialize(Deserializer& d) {
    BaseKeyframe::deserialize(d);
    d.deserialize("lookFrom", lookFrom_);
    d.deserialize("lookTo", lookTo_);
    d.deserialize("lookUp", lookUp_);
}

bool operator==(const CameraKeyframe& a, const CameraKeyframe& b) {
    return a.getTime() == b.getTime() &&
           !(glm::any(glm::notEqual(a.getLookFrom(), b.getLookFrom())) ||
             glm::any(glm::notEqual(a.getLookTo(), b.getLookTo())) ||
             glm::any(glm::notEqual(a.getLookUp(), b.getLookUp())));
}
bool operator!=(const CameraKeyframe& a, const CameraKeyframe& b) { return !(a == b); }

}  // namespace animation

}  // namespace inviwo
