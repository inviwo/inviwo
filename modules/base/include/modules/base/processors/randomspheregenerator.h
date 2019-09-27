/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_RANDOMSPHEREGENERATOR_H
#define IVW_RANDOMSPHEREGENERATOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <random>

namespace inviwo {

class PickingEvent;
class Mesh;

/** \docpage{org.inviwo.RandomSphereGenerator, Random Sphere Generator}
 * ![](org.inviwo.RandomSphereGenerator.png?classIdentifier=org.inviwo.RandomSphereGenerator)
 * Create a grid of randomly generated spheres.
 *
 * ### Outports
 *   * __mesh__ generated sphere meshes with randomized colors and radii
 */

class IVW_MODULE_BASE_API RandomSphereGenerator : public Processor {
public:
    RandomSphereGenerator();
    virtual ~RandomSphereGenerator() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    float rand(const float min, const float max) const;
    vec3 randVec3(const float min = 0, const float max = 1) const;
    vec3 randVec3(const vec3& min, const vec3& max) const;
    void handlePicking(PickingEvent* p, std::function<void(vec3)> callback);
    static vec3 getDelta(const Camera& camera, PickingEvent* p);

    MeshOutport meshOut_;

    Int64Property seed_;
    ButtonProperty reseed_;

    FloatProperty scale_;
    FloatProperty size_;
    IntVec3Property gridDim_;

    BoolProperty jigglePos_;

    BoolProperty enablePicking_;
    CameraProperty camera_;

    PickingMapper spherePicking_;

    std::shared_ptr<BufferRAMPrecision<vec3>> positionBuffer_;
    std::shared_ptr<BufferRAMPrecision<float>> radiiBuffer_;

    mutable std::mt19937 rand_;
    mutable std::uniform_real_distribution<float> dis_;
};

}  // namespace inviwo

#endif  // IVW_RANDOMSPHEREGENERATOR_H
