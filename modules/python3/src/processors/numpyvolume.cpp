/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/python3/processors/numpyvolume.h>
#include <modules/python3/python3module.h>
#include <pybind11/pybind11.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NumPyVolume::processorInfo_{
    "org.inviwo.NumPyVolume",  // Class identifier
    "NumPy Volume",            // Display name
    "NumPy",                   // Category
    CodeState::Experimental,   // Code state
    {"Python"},                // Tags
};
const ProcessorInfo NumPyVolume::getProcessorInfo() const { return processorInfo_; }

NumPyVolume::NumPyVolume()
    : Processor()
    , outport_("outport")
    , size_("size", "Size", size3_t(64), size3_t(32), size3_t(512))
    , script_(InviwoApplication::getPtr()->getModuleByType<Python3Module>()->getPath(
                  ModulePath::Scripts) +
              "/numpyvolumeprocessor.py") {
    addPort(outport_);
    addProperty(size_);

    script_.onChange([&]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void NumPyVolume::process() {
    auto vol = std::make_shared<Volume>(size_.get(), DataFloat32::get());
    auto volObj = pybind11::cast(vol.get());
    script_.run({{"vol", volObj}});
    vol->dataMap_.dataRange = dvec2(0, 1);
    outport_.setData(vol);
}

}  // namespace inviwo
