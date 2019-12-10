/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/datastructures/geometry/plane.h>
#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.MeshPlaneClipping, Mesh Plane Clipping}
 * ![](org.inviwo.MeshPlaneClipping.png?classIdentifier=org.inviwo.MeshPlaneClipping)
 * Clips a mesh against multiple planes in world space.
 *
 * ### Inports
 *   * __inputMesh__ Mesh to clip.
 *   * __inputPlanes__ Clipping planes in world space.
 *
 * ### Outports
 *   * __outputMesh__ Clipped mesh.
 *
 * ### Properties
 *   * __Enable Clipping__ Enable clipping.
 *   * __Cap clipped holes__ Replaces removed parts with triangles aligned with the plane. Input
 * mesh must be manifold.
 */

class IVW_MODULE_BASE_API MeshPlaneClipping : public Processor {
public:
    MeshPlaneClipping();
    virtual ~MeshPlaneClipping() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    MeshInport inputMesh_;
    FlatMultiDataInport<Plane> planes_;
    MeshOutport outputMesh_;

    BoolProperty clippingEnabled_;
    BoolProperty capClippedHoles_;
};

}  // namespace inviwo
