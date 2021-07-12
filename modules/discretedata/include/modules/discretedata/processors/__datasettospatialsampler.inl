// /*********************************************************************************
//  *
//  * Inviwo - Interactive Visualization Workshop
//  *
//  * Copyright (c) 2021 Inviwo Foundation
//  * All rights reserved.
//  *
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions are met:
//  *
//  * 1. Redistributions of source code must retain the above copyright notice, this
//  * list of conditions and the following disclaimer.
//  * 2. Redistributions in binary form must reproduce the above copyright notice,
//  * this list of conditions and the following disclaimer in the documentation
//  * and/or other materials provided with the distribution.
//  *
//  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
//  * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  *
//  *********************************************************************************/

// #include <modules/discretedata/processors/datasettospatialsampler.h>
// #include <fmt/format.h>

// namespace inviwo {
// namespace discretedata {

// // The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
// template <unsigned int SpatialDims, unsigned int DataDims, typename T>
// const ProcessorInfo DataSetToSpatialSampler<SpatialDims, DataDims, T>::processorInfo_{
//     fmt::format("org.inviwo.DataSetToSpatialSampler{}Dto{}D", SpatialDims,
//                 DataDims),  // Class identifier
//     fmt::format("Data Set To Spatial Sampler {}D to {}D", SpatialDims, DataDims),  // Display name
//     "Undefined",                                                                   // Category
//     CodeState::Experimental,                                                       // Code state
//     Tags::None,                                                                    // Tags
// };

// template <unsigned int SpatialDims, unsigned int DataDims, typename T>
// const ProcessorInfo DataSetToSpatialSampler<SpatialDims, DataDims, T>::getProcessorInfo() const {
//     return processorInfo_;
// }

// template <unsigned int SpatialDims, unsigned int DataDims, typename T>
// DataSetToSpatialSampler<SpatialDims, DataDims, T>::DataSetToSpatialSampler()
//     : Processor()
//     , outport_("outport")
//     , position_("position", "Position", vec3(0.0f), vec3(-100.0f), vec3(100.0f)) {

//     addPort(outport_);
//     addProperty(position_);
// }

// template <unsigned int SpatialDims, unsigned int DataDims, typename T>
// void DataSetToSpatialSampler<SpatialDims, DataDims, T>::process() {
//     // outport_.setData(myImage);
// }

// }  // namespace discretedata
// }  // namespace inviwo
