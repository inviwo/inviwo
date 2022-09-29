/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#pragma once

#include <modules/base/basemoduledefine.h>           // for IVW_MODULE_BASE_API

#include <inviwo/core/interaction/pickingmapper.h>   // for PickingMapper
#include <inviwo/core/ports/meshport.h>              // for MeshOutport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>     // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>   // for ButtonProperty
#include <inviwo/core/properties/cameraproperty.h>   // for CameraProperty
#include <inviwo/core/properties/ordinalproperty.h>  // for IntProperty, FloatProperty, Int64Pro...
#include <inviwo/core/util/glmvec.h>                 // for vec3, vec4

#include <cstddef>                                   // for size_t
#include <functional>                                // for function
#include <random>                                    // for mt19937, uniform_real_distribution
#include <vector>                                    // for vector

namespace inviwo {

class Mesh;
class PickingEvent;

class IVW_MODULE_BASE_API RandomMeshGenerator : public Processor {
public:
    RandomMeshGenerator();
    virtual ~RandomMeshGenerator() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    struct Box {
        vec3 rotation;
        vec3 position;
        vec3 scale;
        vec4 color;
    };
    struct Sphere {
        vec3 center;
        float radius;
        vec4 color;
    };
    struct Cylinder {
        vec3 start;
        vec3 end;
        float radius;
        vec4 color;
    };
    struct Cone {
        vec3 start;
        vec3 end;
        float radius;
        vec4 color;
    };
    struct Torus {
        vec3 center;
        vec3 up;
        float radius1;
        float radius2;
        vec4 color;
    };

    float rand(const float min, const float max);
    vec3 randVec3(const float min = 0, const float max = 1);

    void addPickingBuffer(Mesh& mesh, size_t id);
    void handlePicking(PickingEvent* p, std::function<void(vec3)> callback);

    MeshOutport mesh_;

    std::mt19937 rand_;
    std::uniform_real_distribution<float> dis_;

    Int64Property seed_;
    ButtonProperty reseed_;

    FloatProperty scale_;
    FloatProperty size_;
    IntProperty numberOfBoxes_;
    IntProperty numberOfSpheres_;
    IntProperty numberOfCylinders_;
    IntProperty numberOfCones_;
    IntProperty numberOfToruses_;

    std::vector<Box> boxes_;
    std::vector<Sphere> spheres_;
    std::vector<Cylinder> cylinders_;
    std::vector<Cone> cones_;
    std::vector<Torus> toruses_;

    BoolProperty enablePicking_;
    PickingMapper boxPicking_;
    PickingMapper spherePicking_;
    PickingMapper cylinderPicking_;
    PickingMapper conePicking_;
    PickingMapper torusPicking_;
    CameraProperty camera_;
};

}  // namespace inviwo
