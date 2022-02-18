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
#include <modules/discretedata/processors/channeloperations.h>
#include <modules/discretedata/processors/colormapchannel.h>
#include <modules/discretedata/processors/combinechannels.h>
#include <modules/discretedata/processors/combinedatasets.h>
#include <modules/discretedata/processors/createconstantchannel.h>
#include <modules/discretedata/processors/createuniformgrid.h>
#include <modules/discretedata/processors/datasetfrombase.h>
#include <modules/discretedata/processors/datasetroi.h>
#include <modules/discretedata/processors/datasetsource.h>
#include <modules/discretedata/processors/computegridmeasure.h>
#include <modules/discretedata/processors/datasetinformation.h>
#include <modules/discretedata/processors/datasettospatialsampler.h>
#include <modules/discretedata/processors/imagefromdataset.h>
#include <modules/discretedata/processors/volumefromdataset.h>
#include <modules/discretedata/processors/meshfromdataset.h>
#include <modules/discretedata/processors/segmentationvoxelizer.h>
#include <modules/discretedata/processors/createconstantchannel.h>
#include <modules/discretedata/processors/createindexchannel.h>
#include <modules/discretedata/processors/createindexchannel.h>
#include <modules/discretedata/processors/exampledataset.h>
#include <modules/discretedata/processors/extendspatialsampler.h>
#include <modules/discretedata/processors/sphericalcoordinates.h>

#include <modules/discretedata/sampling/interpolant.h>
#include <modules/discretedata/sampling/celltree.h>
using namespace inviwo::discretedata;

namespace inviwo {

DiscreteDataModule::DiscreteDataModule(InviwoApplication* app) : InviwoModule(app, "discretedata") {
    // Processors
    registerProcessor<ChannelOperations>();
    registerProcessor<DataSetROI>();
    registerProcessor<CombineDataSets>();
    registerProcessor<DataSetInformation>();
    registerProcessor<AddDataSetSampler>();
    registerProcessor<DataSetToSpatialSampler2D>();
    registerProcessor<DataSetToSpatialSampler3D>();
    registerProcessor<SphericalCoordinates>();
    registerProcessor<ColormapChannel>();
    registerProcessor<CombineChannels>();
    registerProcessor<CreateConstantChannel>();
    registerProcessor<CreateIndexChannel>();
    registerProcessor<CreateUniformGrid>();
    registerProcessor<DataSetFromVolume>();
    registerProcessor<ComputeGridMeasure>();
    registerProcessor<ExtendSpatialSampler>();
    registerProcessor<MeshFromDataSet>();
    registerProcessor<ImageFromDataSet>();
    registerProcessor<VolumeFromDataSet>();
    registerProcessor<SegmentationVoxelizer>();
    registerProcessor<DataSetSource>();
    registerProcessor<ExampleDataSet>();

    // Ports
    registerPort<DataSetOutport>();
    registerPort<DataSetInport>();

    // Properties
    registerProperty<DimensionProperty>();
    registerProperty<DataChannelProperty>();
    registerProperty<ChannelOpProperty<NormalizeChannelOperation>>();
    registerProperty<ChannelOpProperty<MagnitudeOperation>>();
    registerProperty<ChannelOpProperty<NormalizedMagnitudeOperation>>();
    registerProperty<ChannelOpProperty<AppendOperation>>();
    // Property* prop = new DataChannelProperty("Anke", "Anke");
    // // new ChannelOpProperty<NormalizedMagnitudeOperation>("Anke", "Anke");
    // std::cout << fmt::format("@@@ IDENTIFIER {}", prop->getClassIdentifier())
    //           // std::make_unique<ChannelOpProperty<NormalizedMagnitudeOperation>>
    //           //    ("Anke", "Anke")->getClassIdentifier())
    //           << std::endl;
    // std::cout << fmt::format("@@@ IDENTIFIER {}",
    //                          std::make_unique<DataChannelProperty>("Anke", "Anke")
    //                              ->getClassIdentifier())
    //           << std::endl;

    AddDataSetSampler::addInterpolantType<SkewedBoxInterpolant>("skewedBox", "Skewed Box");
    AddDataSetSampler::addSamplerType<CellTree>("celltree", "CellTree");
}

}  // namespace inviwo
