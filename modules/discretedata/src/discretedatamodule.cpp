/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/discretedata/discretedatamodule.h>
#include <modules/discretedata/processors/adddatasetsampler.h>
#include <modules/discretedata/processors/colormapchannel.h>
#include <modules/discretedata/processors/combinechannels.h>
#include <modules/discretedata/processors/createconstantchannel.h>
#include <modules/discretedata/processors/createuniformgrid.h>
#include <modules/discretedata/processors/datasetfrombase.h>
#include <modules/discretedata/processors/datasetsource.h>
#include <modules/discretedata/processors/computegridmeasure.h>
#include <modules/discretedata/processors/datasettospatialsampler.h>
#include <modules/discretedata/processors/volumefromdataset.h>
#include <modules/discretedata/processors/meshfromdataset.h>
#include <modules/discretedata/processors/segmentationvoxelizer.h>
#include <modules/discretedata/processors/createconstantchannel.h>
#include <modules/discretedata/processors/createindexchannel.h>
#include <modules/discretedata/processors/createindexchannel.h>
#include <modules/discretedata/processors/exampledataset.h>
#include <modules/discretedata/processors/sphericalcoordinates.h>

#include <modules/discretedata/sampling/interpolant.h>
#include <modules/discretedata/sampling/celltree.h>

namespace inviwo {

DiscreteDataModule::DiscreteDataModule(InviwoApplication* app) : InviwoModule(app, "discretedata") {
    // Processors
    registerProcessor<discretedata::AddDataSetSampler>();
    // registerProcessor<discretedata::DataSetToSpatialSampler>();
    registerProcessor<discretedata::DataSetToSpatialSampler2D>();
    registerProcessor<discretedata::DataSetToSpatialSampler3D>();
    registerProcessor<discretedata::SphericalCoordinates>();
    registerProcessor<discretedata::ColormapChannel>();
    registerProcessor<discretedata::CombineChannels>();
    registerProcessor<discretedata::CreateConstantChannel>();
    registerProcessor<discretedata::CreateIndexChannel>();
    registerProcessor<discretedata::CreateUniformGrid>();
    registerProcessor<discretedata::DataSetFromVolume>();
    registerProcessor<discretedata::ComputeGridMeasure>();
    registerProcessor<discretedata::MeshFromDataSet>();
    registerProcessor<discretedata::VolumeFromDataSet>();
    registerProcessor<discretedata::SegmentationVoxelizer>();
    registerProcessor<discretedata::DataSetSource>();
    registerProcessor<discretedata::ExampleDataSet>();

    // Ports
    registerPort<discretedata::DataSetOutport>();
    registerPort<discretedata::DataSetInport>();

    // Properties
    registerProperty<discretedata::DimensionProperty>();

    discretedata::AddDataSetSampler::addInterpolantType<discretedata::SkewedBoxInterpolant>(
        "skewedBox", "Skewed Box");
    discretedata::AddDataSetSampler::addSamplerType<discretedata::CellTree>("celltree", "CellTree");
}

}  // namespace inviwo
