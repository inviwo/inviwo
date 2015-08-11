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

#ifndef IVW_DIFFUSE_LIGHT_SOURCE_PROCESSOR_H
#define IVW_DIFFUSE_LIGHT_SOURCE_PROCESSOR_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/light/baselightsource.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/positionproperty.h>
#include <modules/base/basemoduledefine.h>

namespace inviwo {

class DiffuseLight;

/** \docpage{org.inviwo.Diffuselightsource, Diffuse light source}
 * ![](org.inviwo.Diffuselightsource.png?classIdentifier=org.inviwo.Diffuselightsource)
 *
 * Produces a planar area light source, spreading light in all directions from the plane.
 * The direction of the plane will be computed as glm::normalize(vec3(0) - lightPos) 
 * when specified in world space and normalize(camera_.getLookTo() - lightPos) when specified in view space.
 * 
 * ### Outports
 *   * __DiffuseLightSource__ Planar area light source
 * 
 * ### Properties
 *   * __Light Source Position__ Center point of the plane
 *   * __Light power__ Increases/decreases light strength
 *   * __Light size__ Width and height in world space
 *   * __Color__ Flux density per solid angle, W*s*r^-1 (intensity)
 */
class IVW_MODULE_BASE_API DiffuseLightSourceProcessor : public Processor {
public:
    DiffuseLightSourceProcessor();
    virtual ~DiffuseLightSourceProcessor();

    InviwoProcessorInfo();

protected:
    virtual void process();

    /**
     * Update light source parameters. Transformation will be given in texture space.
     *
     * @param lightSource
     * @return
     */
    void updateLightSource(DiffuseLight* lightSource);

private:
    DataOutport<LightSource> outport_;

    CameraProperty camera_; //< Link camera in order to specify position in view space.
    PositionProperty lightPosition_;
    CompositeProperty lighting_;
    FloatProperty lightPowerProp_;
    FloatVec2Property lightSize_;
    FloatVec4Property lightDiffuse_;
    
    

    DiffuseLight* lightSource_;
};

}  // namespace

#endif  // IVW_DIFFUSE_LIGHT_SOURCE_PROCESSOR_H
