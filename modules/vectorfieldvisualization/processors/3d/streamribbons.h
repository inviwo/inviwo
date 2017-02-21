/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2017 Inviwo Foundation
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

#ifndef IVW_STREAMRIBBONS_H
#define IVW_STREAMRIBBONS_H

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <modules/vectorfieldvisualization/streamlinetracer.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>

#include <modules/vectorfieldvisualization/ports/seedpointsport.h>
#include <modules/vectorfieldvisualization/properties/streamlineproperties.h>

namespace inviwo {

/** \docpage{<classIdentifier>, StreamRibbons}
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API StreamRibbons : public Processor {
public:
    enum class ColoringMethod {
        Velocity,
        Vorticity,
        ColorPort
    };
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    StreamRibbons();
    virtual ~StreamRibbons() {}

protected:
    virtual void process() override;


    inline virtual bool isReady() const override {
        if (Processor::isReady()) {
            return true;
        }

        if (!seedPoints_.isReady()) return false;
        if (colors_.isConnected() && !colors_.isReady()) return false;

        bool velocitiesReady = false;
        bool vorticitiesReady = false;

        velocitiesReady |= sampler_.isConnected() && sampler_.isReady();
        velocitiesReady |= volume_.isConnected() && volume_.isReady();

        vorticitiesReady |= vorticitySampler_.isConnected() && vorticitySampler_.isReady();
        vorticitiesReady |= vorticityVolume_.isConnected() && vorticityVolume_.isReady();

        return velocitiesReady && vorticitiesReady;
    }

private:
    DataInport<SpatialSampler<3, 3, double>> sampler_;
    DataInport<SpatialSampler<3, 3, double>> vorticitySampler_;
    SeedPointsInport seedPoints_;
    DataInport<std::vector<vec4>> colors_;

    VolumeInport volume_;
    VolumeInport vorticityVolume_;


    StreamLineProperties streamLineProperties_;

    FloatProperty ribbonWidth_;

    TransferFunctionProperty tf_;
    TemplateOptionProperty<ColoringMethod> coloringMethod_;
    FloatProperty velocityScale_;
    StringProperty maxVelocity_;
    StringProperty maxVorticity_;

    MeshOutport mesh_;
};

}  // namespace

#endif  // IVW_STREAMRIBBONS_H
