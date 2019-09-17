/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_MESHCLIPPING_H
#define IVW_MESHCLIPPING_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>
#include <inviwo/core/datastructures/geometry/plane.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.MeshClipping, Mesh Clipping}
 * ![](org.inviwo.MeshClipping.png?classIdentifier=org.inviwo.MeshClipping)
 *
 * Remove parts of a mesh that is on the back side of a plane.
 * Replaces removed parts with triangles aligned with the plane.
 * Link the camera property to move the camera along the plane, or to align plane with view
 * direction. Coordinates are specified in world space.
 *
 * Supports SimpleMesh and BasicMesh
 *
 * ### Inports
 *   * __inputMesh__ Input mesh
 *
 * ### Outports
 *   * __clippedMesh__ Clipped output mesh
 *   * __clippingPlane__ Plane used for clipping the mesh in world space coordinate system
 *
 * ### Properties
 *   * __Move Plane Point Along Normal__ Enable slider for adjusting plane position
 *   * __Plane Point Along Normal Move__ Offset plane along its normal
 *   * __Move Camera Along Normal__ Moves camera along with plane movement.
 *   * __Plane Point__ World space space position of plane
 *   * __Plane Normal__ ... World space space normal of plane
 *   * __Camera__ Camera used for moving or aligning plane.
 *   * __Align Plane Normal To Camera Normal__ Aligns plane normal with camera
 *   * __Enable clipping__ Pass through mesh if disabled.
 *

 */
class IVW_MODULE_BASE_API MeshClipping : public Processor {
public:
    MeshClipping();
    ~MeshClipping();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    void onAlignPlaneNormalToCameraNormalPressed();

    MeshInport inport_;
    MeshOutport outport_;
    DataOutport<Plane> clippingPlane_;

    BoolProperty clippingEnabled_;
    BoolProperty movePointAlongNormal_;
    BoolProperty moveCameraAlongNormal_;
    FloatProperty pointPlaneMove_;

    BoolProperty capClippedHoles_;

    FloatVec3Property planePoint_;   ///< World space plane position
    FloatVec3Property planeNormal_;  ///< World space plane normal

    ButtonProperty alignPlaneNormalToCameraNormal_;
    CameraProperty camera_;

    float previousPointPlaneMove_;
};
}  // namespace inviwo

#endif  // IVW_MESHCLIPPING_H
