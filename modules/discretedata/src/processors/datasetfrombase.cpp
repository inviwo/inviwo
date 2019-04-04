/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/processors/datasetfrombase.h>
#include <inviwo/core/processors/processorinfo.h>
#include <modules/discretedata/connectivity/structuredgrid.h>

namespace inviwo {
namespace discretedata {

const ProcessorInfo DataSetFromVolume::processorInfo_{
    "org.inviwo.kth.DataSetFromVolume",  // Class identifier
    "Data Set from Volume",              // Display name
    "Data Set",                          // Category
    CodeState::Experimental,             // Code state
    Tags::None                           // Tags
};

DataSetFromVolume::DataSetFromVolume()
    : Processor()
    , portInData("BaseData")
    , portInDataSet("PriorDataSet")
    , portOutData("DataSetOutput")
    , channelName("dataName", "Data Name", "default")
    , saveToPrimitive("saveToType", "Primitive with Data") {
    addPort(portInData);
    addPort(portOutData);
    addPort(portInDataSet);
    portInDataSet.setOptional(true);
    addProperty(channelName);

    saveToPrimitive.addOption("vertex", "Vertex", GridPrimitive::Vertex);
    saveToPrimitive.addOption("volume", "Volume", GridPrimitive::Volume);
    addProperty(saveToPrimitive);
}

void DataSetFromVolume::process() {

    GridPrimitive dimensionTo = saveToPrimitive.get();

    // Read volume information.
    auto volume = portInData.getData();
    const VolumeRAM *dataRAM = volume->getRepresentation<VolumeRAM>();

    auto dims = dataRAM->getDimensions();
    size3_t dimGrid = dims;

    ind numElements = dimGrid.x * dimGrid.y * dimGrid.z;

    DataSet *dataSet;

    if (portInDataSet.isConnected() && portInDataSet.hasData())
        dataSet = new DataSet(*portInDataSet.getData());
    else {
        // Build a new grid.
        dataSet = new DataSet(GridPrimitive::Volume,
                              std::vector<ind>({(ind)dimGrid.x-1, (ind)dimGrid.y-1, (ind)dimGrid.z-1}));
    }

    // Copy data.
    const void *dataConst = dataRAM->getData();
    char *data = new char[dataRAM->getNumberOfBytes()];
    memcpy(data, dataConst, dataRAM->getNumberOfBytes());

    DataFormatId type = dataRAM->getDataFormatId();
    Channel *channel = nullptr;

    switch (type) {
        case DataFormatId::Float16:
            channel =
                new BufferChannel<f16, 1>((f16 *)data, numElements, channelName.get(), dimensionTo);
            break;
        case DataFormatId::Float32:
            channel = new BufferChannel<glm::f32, 1>((glm::f32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Float64:
            channel = new BufferChannel<glm::f64, 1>((glm::f64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Int8:
            channel = new BufferChannel<glm::i8, 1>((glm::i8 *)data, numElements, channelName.get(),
                                                    dimensionTo);
            break;
        case DataFormatId::Int16:
            channel = new BufferChannel<glm::i16, 1>((glm::i16 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Int32:
            channel = new BufferChannel<glm::i32, 1>((glm::i32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Int64:
            channel = new BufferChannel<glm::i64, 1>((glm::i64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::UInt8:
            channel = new BufferChannel<glm::u8, 1>((glm::u8 *)data, numElements, channelName.get(),
                                                    dimensionTo);
            break;
        case DataFormatId::UInt16:
            channel = new BufferChannel<glm::u16, 1>((glm::u16 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::UInt32:
            channel = new BufferChannel<glm::u32, 1>((glm::u32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::UInt64:
            channel = new BufferChannel<glm::u64, 1>((glm::u64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2Float16:
            channel =
                new BufferChannel<f16, 2>((f16 *)data, numElements, channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2Float32:
            channel = new BufferChannel<glm::f32, 2>((glm::f32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2Float64:
            channel = new BufferChannel<glm::f64, 2>((glm::f64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2Int8:
            channel = new BufferChannel<glm::i8, 2>((glm::i8 *)data, numElements, channelName.get(),
                                                    dimensionTo);
            break;
        case DataFormatId::Vec2Int16:
            channel = new BufferChannel<glm::i16, 2>((glm::i16 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2Int32:
            channel = new BufferChannel<glm::i32, 2>((glm::i32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2Int64:
            channel = new BufferChannel<glm::i64, 2>((glm::i64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2UInt8:
            channel = new BufferChannel<glm::u8, 2>((glm::u8 *)data, numElements, channelName.get(),
                                                    dimensionTo);
            break;
        case DataFormatId::Vec2UInt16:
            channel = new BufferChannel<glm::u16, 2>((glm::u16 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2UInt32:
            channel = new BufferChannel<glm::u32, 2>((glm::u32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec2UInt64:
            channel = new BufferChannel<glm::u64, 2>((glm::u64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3Float16:
            channel =
                new BufferChannel<f16, 3>((f16 *)data, numElements, channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3Float32:
            channel = new BufferChannel<glm::f32, 3>((glm::f32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3Float64:
            channel = new BufferChannel<glm::f64, 3>((glm::f64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3Int8:
            channel = new BufferChannel<glm::i8, 3>((glm::i8 *)data, numElements, channelName.get(),
                                                    dimensionTo);
            break;
        case DataFormatId::Vec3Int16:
            channel = new BufferChannel<glm::i16, 3>((glm::i16 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3Int32:
            channel = new BufferChannel<glm::i32, 3>((glm::i32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3Int64:
            channel = new BufferChannel<glm::i64, 3>((glm::i64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3UInt8:
            channel = new BufferChannel<glm::u8, 3>((glm::u8 *)data, numElements, channelName.get(),
                                                    dimensionTo);
            break;
        case DataFormatId::Vec3UInt16:
            channel = new BufferChannel<glm::u16, 3>((glm::u16 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3UInt32:
            channel = new BufferChannel<glm::u32, 3>((glm::u32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec3UInt64:
            channel = new BufferChannel<glm::u64, 3>((glm::u64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4Float16:
            channel =
                new BufferChannel<f16, 4>((f16 *)data, numElements, channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4Float32:
            channel = new BufferChannel<glm::f32, 4>((glm::f32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4Float64:
            channel = new BufferChannel<glm::f64, 4>((glm::f64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4Int8:
            channel = new BufferChannel<glm::i8, 4>((glm::i8 *)data, numElements, channelName.get(),
                                                    dimensionTo);
            break;
        case DataFormatId::Vec4Int16:
            channel = new BufferChannel<glm::i16, 4>((glm::i16 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4Int32:
            channel = new BufferChannel<glm::i32, 4>((glm::i32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4Int64:
            channel = new BufferChannel<glm::i64, 4>((glm::i64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4UInt8:
            channel = new BufferChannel<glm::u8, 4>((glm::u8 *)data, numElements, channelName.get(),
                                                    dimensionTo);
            break;
        case DataFormatId::Vec4UInt16:
            channel = new BufferChannel<glm::u16, 4>((glm::u16 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4UInt32:
            channel = new BufferChannel<glm::u32, 4>((glm::u32 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        case DataFormatId::Vec4UInt64:
            channel = new BufferChannel<glm::u64, 4>((glm::u64 *)data, numElements,
                                                     channelName.get(), dimensionTo);
            break;
        default:
            ivwAssert(false, "No valid type set");
    }

    // Catch non-initialized channel.
    if (!channel) {
        LogError("Buffer initialization failed.");
        return;
    }
    dataSet->addChannel(channel);

    // Create position analytic channel.
    CurvilinearPositions<float, 3> gridFunc(*volume);
    auto positions = new AnalyticChannel<float, 3, vec3>(
        // [&](vec3 &val, ind idx) {
        //     auto matrix = volume->getIndexMatrix();
        //     vec4 pos = {idx % dims.x, (idx / dims.x) % dims.y, idx / (dims.x * dims.y), 0};
        //     vec4 val4 = matrix * pos;
        //     val = {val4.x, val4.y, val4.z};
        // },
        gridFunc, numElements, "Positions", GridPrimitive::Vertex);
    dataSet->addChannel(positions);

    portOutData.setData(dataSet);
}

const ProcessorInfo DataSetFromVolume::getProcessorInfo() const { return processorInfo_; }

}  // namespace discretedata
}  // namespace inviwo
