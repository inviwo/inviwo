/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <inviwo/volume/volumemoduledefine.h>
#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <optional>

namespace inviwo {

/** \docpage{org.inviwo.VolumeVoronoiSegmentation, Volume Voronoi Segmentation}
 * ![](org.inviwo.VolumeVoronoiSegmentation.png?classIdentifier=org.inviwo.VolumeVoronoiSegmentation)
 *
 * Processor which calculates the voronoi segmentation of a volume, given the volume together with
 * seed points (and optional weights) as input.
 *
 *
 * ### Inports
 *   * __inputVolume__ The input volume.
 *   * __seedPoints__ Seed points together with indices and optional weights. The seed points are
 *                    expected to be in the model space. The column with the weights should have
 *                    header name "r".
 *
 * ### Outports
 *   * __outport__ Volume where each voxel has the value of the closest seed point, according to the
 *                 voronoi algorithm.
 *
 * ### Properties
 *   * __Weighted voronoi__ Choose whether the weighted version of voronoi should be used or not.
 *
 */

class IVW_MODULE_VOLUME_API VolumeVoronoiSegmentation : public PoolProcessor {
public:
    VolumeVoronoiSegmentation();
    virtual ~VolumeVoronoiSegmentation() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volume_;
    DataFrameInport dataFrame_;
    VolumeOutport outport_;
    BoolProperty weighted_;

    DataFrameColumnProperty xCol_;
    DataFrameColumnProperty yCol_;
    DataFrameColumnProperty zCol_;
    DataFrameColumnProperty wCol_;
};

}  // namespace inviwo
