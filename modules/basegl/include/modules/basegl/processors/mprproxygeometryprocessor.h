/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_MPRPROXYGEOMETRYPROCESSOR_H
#define IVW_MPRPROXYGEOMETRYPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

namespace inviwo {
/**
 *
 *
 * Constructs a proxy geometry based on one of three MPR planes, to generate entry/exit points for a
 * raycaster. Like CubeProxyGeometry. Can be seen as general "Slab Proxy Geometry" to facilitate
 * volume slice sampling on arbitrary planes with thickness. It exists because Cube Proxy Geometry
 * can only clip axis-aligned and Volume Slice has no thickness. Slabs are constructed in world
 * space and the side length is the length of the longest axis of the model, to display a complete
 * volume slice. Plug output into Geometry Entry Exit Points processor, not normal Entry Exit Points
 * processor, to transform world space positions into volume positions, that are needed for
 * raycasting. Remember to deactivate near plane clipping in Geometry Entry Exit Points processor.
 * Remember to turn off interaction event handling in Trackball in Geometry Entry Exit Points
 * processor, when using static camera, i.e. always looking straight onto slice.
 *
 * ### Inports
 *   * __Inport__ Input Volume
 *
 * ### Outports
 *   * __Outport__ Output proxy geometry.
 *
 * ### Properties
 */

class IVW_MODULE_BASEGL_API MPRProxyGeometry : public Processor {
public:
    MPRProxyGeometry();
    ~MPRProxyGeometry();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    VolumeInport inport_;
    MeshOutport outport_;

    ButtonProperty recenterP_;

    FloatVec3Property mprP_;
    FloatVec3Property mprBasisN_;
    FloatVec3Property mprBasisR_;
    FloatVec3Property mprBasisU_;

    FloatProperty slabThickness_;

    FloatProperty depth_;

    FloatProperty zoom_;

    ButtonProperty adjustCam_;
    BoolProperty enableStaticCam_;
    CameraProperty cam_;
};

}  // namespace inviwo

#endif  // IVW_MPRPROXYGEOMETRYPROCESSOR_H
