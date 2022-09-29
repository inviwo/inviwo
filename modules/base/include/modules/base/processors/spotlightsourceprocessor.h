/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>                     // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/light/baselightsource.h>  // for LightSource
#include <inviwo/core/ports/dataoutport.h>                     // for DataOutport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>             // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>          // for CompositeProperty
#include <inviwo/core/properties/ordinalproperty.h>            // for FloatProperty, FloatVec2Pr...
#include <inviwo/core/properties/positionproperty.h>           // for PositionProperty

#include <memory>                                              // for shared_ptr
#include <string>                                              // for operator+

#include <fmt/core.h>                                          // for format

namespace inviwo {

class SpotLight;

/** \docpage{org.inviwo.Spotlightsource, Spot light source}
 * ![](org.inviwo.Spotlightsource.png?classIdentifier=org.inviwo.Spotlightsource)
 *
 * Produces a spot light source, spreading light in the shape of a cone.
 * The direction of the cone will be computed as glm::normalize(vec3(0) - lightPos)
 * when specified in world space and normalize(camera_.getLookTo() - lightPos) when specified in
 * view space.
 *
 *
 *
 *
 * ### Properties
 *   * __Light Source Position__ Start point of the cone.
 *   * __Light power (%)__ Increases/decreases light strength.
 *   * __Light size__ ...
 *   * __Light Cone Radius Angle__ Cone radius angle of the light source
 *   * __Color__ Flux density per solid angle, W*s*r^-1 (intensity)
 *   * __Light Fall Off Angle__ Fall off angle of the light source
 *
 */
class IVW_MODULE_BASE_API SpotLightSourceProcessor : public Processor {
public:
    SpotLightSourceProcessor();
    virtual ~SpotLightSourceProcessor() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

    /**
     * Update light source parameters. Transformation will be given in texture space.
     *
     * @param lightSource
     * @return
     */
    void updateSpotLightSource(SpotLight* lightSource);

private:
    DataOutport<LightSource> outport_;

    CameraProperty camera_;  //< Link camera in order to specify position in view space.
    PositionProperty lightPosition_;
    CompositeProperty lighting_;
    FloatProperty lightPowerProp_;
    FloatVec2Property lightSize_;
    FloatVec3Property lightDiffuse_;
    FloatProperty lightConeRadiusAngle_;
    FloatProperty lightFallOffAngle_;

    std::shared_ptr<SpotLight> lightSource_;
};

}  // namespace inviwo
