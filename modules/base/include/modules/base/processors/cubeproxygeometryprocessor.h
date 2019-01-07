/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_CUBEPROXYGEOMETRYPROCESSOR_H
#define IVW_CUBEPROXYGEOMETRYPROCESSOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

namespace inviwo {
/** \docpage{org.inviwo.CubeProxyGeometry, Cube Proxy Geometry}
 * ![](org.inviwo.CubeProxyGeometry.png?classIdentifier=org.inviwo.CubeProxyGeometry)
 *
 * Constructs a proxy geometry based on the model and world matrix of the input volume.
 * The gemetry will be shaped as a parallelepiped. If clipping is enabled the geometry
 * will be cut along the corresponding axes.
 *
 * ### Inports
 *   * __Inport__ Input Volume
 *
 * ### Outports
 *   * __Outport__ Output proxy geometry.
 *
 * ### Properties
 *   * __Enable Clipping__ Enable axis aligned clipping of the mesh
 *   * __Clip X Slices__ Clip X axis
 *   * __Clip Y Slices__ Clip Y axis
 *   * __Clip Z Slices__ Clip Z axis
 */

class IVW_MODULE_BASE_API CubeProxyGeometry : public Processor {
public:
    CubeProxyGeometry();
    ~CubeProxyGeometry();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    VolumeInport inport_;
    MeshOutport outport_;

    BoolProperty clippingEnabled_;

    IntMinMaxProperty clipX_;
    IntMinMaxProperty clipY_;
    IntMinMaxProperty clipZ_;
};

}  // namespace inviwo

#endif  // IVW_CUBEPROXYGEOMETRYPROCESSOR_H
