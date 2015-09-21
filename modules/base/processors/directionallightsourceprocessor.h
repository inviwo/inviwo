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

#ifndef IVW_DIRECTIONAL_LIGHT_SOURCE_PROCESSOR_H
#define IVW_DIRECTIONAL_LIGHT_SOURCE_PROCESSOR_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/light/baselightsource.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/positionproperty.h>
#include <modules/base/basemoduledefine.h>

namespace inviwo {

class DirectionalLight;
/** \docpage{org.inviwo.Directionallightsource, Directional light source}
 * ![](org.inviwo.Directionallightsource.png?classIdentifier=org.inviwo.Directionallightsource)
 *
 * Produces a light source with parallel light rays, spreading light in the direction from an infinite plane.
 * The direction of the plane will be computed as glm::normalize(vec3(0) - lightPos)
 * when specified in world space and normalize(camera_.getLookTo() - lightPos) when specified in view space.
 * 
 * ### Outports
 *   * __DirectionalLightSource__ Directional light source
 * 
 * ### Properties
 *   * __Light power__ Increases/decreases light strength
 *   * __Color__ Flux density per solid angle, W*s*r^-1 (intensity)
 *   * __Light Source Position__ Origin of the light source
 *   * __Enabled__ Turns light on or off
 */
class IVW_MODULE_BASE_API DirectionalLightSourceProcessor : public Processor {
public:
    DirectionalLightSourceProcessor();
    virtual ~DirectionalLightSourceProcessor() = default;

    InviwoProcessorInfo();

protected:
    virtual void process() override;

    /**
     * Update light source parameters. Transformation will be given in texture space.
     *
     * @param lightSource
     * @return
     */
    void updateDirectionalLightSource(DirectionalLight* lightSource);

private:
    DataOutport<LightSource> outport_;

    CameraProperty camera_; //< Link camera in order to specify position in view space.
    PositionProperty lightPosition_;
    CompositeProperty lighting_;
    FloatProperty lightPowerProp_;
    FloatVec4Property lightDiffuse_;
    BoolProperty lightEnabled_;
    
    
    std::shared_ptr<DirectionalLight> lightSource_;
};

}  // namespace

#endif  // IVW_DIRECTIONAL_LIGHT_SOURCE_PROCESSOR_H
