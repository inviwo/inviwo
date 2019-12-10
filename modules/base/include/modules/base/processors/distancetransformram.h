/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_DISTANCETRANSFORMRAM_H
#define IVW_DISTANCETRANSFORMRAM_H

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/clock.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

/** \docpage{org.inviwo.DistanceTransformRAM, Distance Transform}
* ![](org.inviwo.DistanceTransformRAM.png?classIdentifier=org.inviwo.DistanceTransformRAM)
*
* Computes the distance transform of a volume dataset using a threshold value
* The result is the distance from each voxel to the closest feature. It will only work correctly for
* volumes with a orthogonal basis. It uses the Saito's algorithm to compute the Euclidean distance.
*
* ### Inports
*   * __inputVolume__ Input volume
*
* ### Outports
*   * __outputVolume__ Scalar volume representing the distance transform (float)
*
* ### Properties
*   * __Threshold__ Voxles with a value  __larger___ then the then the threshold will be considered
     as features, i.e. have a zero distance.
*   * __Flip__ Consider features as voxels with a values __smaller__ then threshold instead.
*   * __Use normalized threshold__ Use normalized values when comparing to the threshold.
*   * __Scaling Factor__ Scaling factor to apply to the output distance field.
*   * __Squared Distance__ Output the squared distance field
*   * __Up sample__ Make the output volume have a higher resolution.
*   * __Data Range__ Data range to use for the output volume:
*       * Diagonal use [0, volume diagonal].
*       * MinMax use the minimal and maximal distance from the result
*       * Custom specify a custom range.
*   * __Data Range__ The data range of the output volume. (ReadOnly)
*   * __Custom Data Range__ Specify a custom output range.
*
*/

class IVW_MODULE_BASE_API DistanceTransformRAM : public PoolProcessor {
public:
    enum class DataRangeMode { Diagonal, MinMax, Custom };

    DistanceTransformRAM();
    virtual ~DistanceTransformRAM();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    VolumeInport volumePort_;
    VolumeOutport outport_;

    DoubleProperty threshold_;
    BoolProperty flip_;
    BoolProperty normalize_;
    DoubleProperty resultDistScale_;  // scaling factor for distances
    BoolProperty resultSquaredDist_;  // determines whether output uses squared euclidean distances
    BoolProperty uniformUpsampling_;
    IntProperty upsampleFactorUniform_;    // uniform upscaling of the output field
    IntSize3Property upsampleFactorVec3_;  // non-uniform upscaling of the output field

    DoubleMinMaxProperty dataRangeOutput_;
    TemplateOptionProperty<DataRangeMode> dataRangeMode_;
    DoubleMinMaxProperty customDataRange_;
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             DistanceTransformRAM::DataRangeMode m) {
    switch (m) {
        case DistanceTransformRAM::DataRangeMode::Diagonal:
            ss << "Diagonal";
            break;
        case DistanceTransformRAM::DataRangeMode::MinMax:
            ss << "MinMax";
            break;
        case DistanceTransformRAM::DataRangeMode::Custom:
            ss << "Custom";
            break;
        default:
            break;
    }
    return ss;
}

}  // namespace inviwo

#endif  // IVW_DISTANCETRANSFORMRAM_H
