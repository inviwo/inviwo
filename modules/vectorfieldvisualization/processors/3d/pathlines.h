/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2016 Inviwo Foundation
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

#ifndef IVW_PATHLINES_H
#define IVW_PATHLINES_H

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/vectorfieldvisualization/pathlinetracer.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>

#include <modules/vectorfieldvisualization/ports/seedpointsport.h>
#include <modules/vectorfieldvisualization/properties/pathlineproperties.h>
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>

namespace inviwo {

/** \docpage{org.inviwo.PathLines, Path Lines}
 * ![](org.inviwo.PathLines.png?classIdentifier=org.inviwo.PathLines)
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
class IVW_MODULE_VECTORFIELDVISUALIZATION_API PathLines : public Processor { 
public:
    enum class ColoringMethod{
        Velocity, 
        Timestamp,
        ColorPort
    };
    PathLines();
    virtual ~PathLines() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
     
    virtual void process() override; 

    virtual bool isReady() const override {
        if (Processor::isReady()) {
            return true;
        }

        if (seedPoints_.isConnected() && !seedPoints_.isReady()) return false;
        if (colors_.isConnected() && !colors_.isReady()) return false;

        if (sampler_.isConnected()) {
            return sampler_.isReady();
        }
        if (volume_.isConnected()) {
            return volume_.isReady();
        }
        return false;
    }

    virtual void deserialize(Deserializer& d) override;

private:
    DataInport<Spatial4DSampler<3, double>> sampler_;
    SeedPointsInport seedPoints_;
    DataInport<std::vector<vec4>> colors_;
    VolumeSequenceInport volume_;
    IntegralLineSetOutport lines_;


    MeshOutport linesStripsMesh_;


    PathLineProperties pathLineProperties_;

   
    TransferFunctionProperty tf_;
    TemplateOptionProperty<ColoringMethod> coloringMethod_;
    FloatProperty velocityScale_;
    StringProperty maxVelocity_;

    BoolProperty allowLooping_;
};

} // namespace

#endif // IVW_PATHLINES_H

