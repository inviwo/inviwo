/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/volumeport.h>                       // for VolumeOutport
#include <inviwo/core/processors/processor.h>                   // for Processor
#include <inviwo/core/processors/processorinfo.h>               // for ProcessorInfo
#include <inviwo/core/properties/optionproperty.h>              // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>             // for IntProperty, IntSize3Prop...
#include <inviwo/core/util/formats.h>                           // for DataFormatId
#include <inviwo/core/util/staticstring.h>                      // for operator+
#include <modules/base/properties/basisproperty.h>              // for BasisProperty
#include <modules/base/properties/volumeinformationproperty.h>  // for VolumeInformationProperty
#include <inviwo/core/ports/layerport.h>                        // for LayerInport

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, opera...

namespace inviwo {
class Deserializer;
class Volume;

/** \docpage{org.inviwo.VolumeCreator, Volume Creator}
 * ![](org.inviwo.VolumeCreator.png?classIdentifier=org.inviwo.VolumeCreator)
 * Procedurally generate volume data on the CPU.
 *
 * ### Outports
 *   * __volume__ The generated volume.
 *
 * ### Properties
 *   * __Type__ Type of volume to generate:
 *       * __Single Voxel__ Center voxel equal to 1 all other 0
 *       * __Sphere__ Spherically symmetric density centered in the volume decaying radially with
 *         the distance from the center
 *       * __Ripple__ A quickly oscillating density between 0 and 1
 *       * __Marching Cube__ A 2x2x2 volume corresponding to a marching cube case
 *   * __Dimensions__ Volume Dimensions
 *   * __Format__ Volume data format
 *   * __Index__ Marching cube case index
 */

class IVW_MODULE_BASE_API GaussianVolumeCreator : public Processor {
public:
    GaussianVolumeCreator();
    virtual ~GaussianVolumeCreator() = default;

    enum class Type { SingleVoxel, Sphere, Ripple, MarchingCube, Gaussian };

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void deserialize(Deserializer& d) override;

private:
    DataInport<std::vector<vec4>> points_;
    VolumeOutport outport_;
    OptionProperty<Type> type_;
    OptionProperty<DataFormatId> format_;
    IntSize3Property dimensions_;
    IntProperty index_;
    FloatProperty sigma_;
    IntProperty nPoints_;
    FloatProperty radii_;
    
    VolumeInformationProperty information_;
    BasisProperty basis_;
    std::shared_ptr<Volume> loadedData_;
    bool deserialized_ = false;
};

}  // namespace inviwo
