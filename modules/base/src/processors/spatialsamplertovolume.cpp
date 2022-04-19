// /*********************************************************************************
//  *
//  * Inviwo - Interactive Visualization Workshop
//  *
//  * Copyright (c) 2022 Inviwo Foundation
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

// #include <modules/base/processors/spatialsamplertovolume.h>

// namespace inviwo {

// // The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
// const ProcessorInfo SpatialSamplerToVolume::processorInfo_{
//     "org.inviwo.SpatialSamplerToVolume",  // Class identifier
//     "SpatialSampler To Volume",           // Display name
//     "Undefined",                          // Category
//     CodeState::Experimental,              // Code state
//     Tags::None,                           // Tags
// };
// const ProcessorInfo SpatialSamplerToVolume::getProcessorInfo() const { return processorInfo_; }

// SpatialSamplerToVolume::SpatialSamplerToVolume()
//     : Processor()
//     , sampler_("spatialSampler3D")
//     , volume_("volume")
//     , volumeSize_(
//           "numCells", "Volume size", {128, 128, 128},
//           std::make_pair<size3_t, ConstraintBehavior>(size3_t{1}, ConstraintBehavior::Immutable),
//           std::make_pair<size3_t, ConstraintBehavior>(size3_t{512}, ConstraintBehavior::Ignore))
//           {

//     addPorts(sampler_, volume_);
//     addProperty(volumeSize_);
// }

// void SpatialSamplerToVolume::process() {

// }

// }  // namespace inviwo
