/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
 * ...
 * 
 * ### Inports
 *   * __geometry.input__ ...
 * 
 * ### Outports
 *   * __geometry.output__ ...
 * 
 * ### Properties
 *   * __Move Camera Along Normal__ ...
 *   * __Plane Point__ ...
 *   * __Render As Points by Default__ ...
 *   * __Move Plane Point Along Normal__ ...
 *   * __Camera__ ...
 *   * __Plane Normal__ ...
 *   * __Align Plane Normal To Camera Normal__ ...
 *   * __Enable clipping__ ...
 *   * __Plane Point Along Normal Move__ ...
 *
 */
class IVW_MODULE_BASE_API MeshClipping : public Processor {
public:
    MeshClipping();
    ~MeshClipping();

    InviwoProcessorInfo();

    void initialize() override;
    void deinitialize() override;

protected:
    virtual void process() override;

    void onMovePointAlongNormalToggled();
    void onAlignPlaneNormalToCameraNormalPressed();

    Mesh* clipGeometryAgainstPlaneRevised(const Mesh*, Plane);
    Mesh* clipGeometryAgainstPlane(const Mesh*, Plane);
    float degreeToRad(float);

private:
    MeshInport inport_;
    MeshOutport outport_;

    BoolProperty clippingEnabled_;
    BoolProperty movePointAlongNormal_;
    BoolProperty moveCameraAlongNormal_;
    FloatProperty pointPlaneMove_;
    FloatVec3Property planePoint_;
    FloatVec3Property planeNormal_;
    ButtonProperty alignPlaneNormalToCameraNormal_;
    BoolProperty renderAsPoints_;
    CameraProperty camera_;

    float previousPointPlaneMove_;

    static const float EPSILON;
};
} // namespace

#endif // IVW_MESHCLIPPING_H
