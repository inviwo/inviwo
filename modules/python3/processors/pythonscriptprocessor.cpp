/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/python3/processors/pythonscriptprocessor.h>
#include <modules/python3/python3module.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <pybind11/pybind11.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PythonScriptProcessor::processorInfo_{
    "org.inviwo.PythonScriptProcessor",  // Class identifier
    "Python Script Processor",           // Display name
    "Python",                            // Category
    CodeState::Experimental,             // Code state
    "Python, CPU",                       // Tags
};
const ProcessorInfo PythonScriptProcessor::getProcessorInfo() const { return processorInfo_; }

PythonScriptProcessor::PythonScriptProcessor()
    : Processor()
    , script_("")
    , scriptFileName_("scriptFileName", "File Name", "")
    , mesh_("mesh")
    , volume_("volume") {
    addPort(mesh_);
    addPort(volume_);

    addProperty(scriptFileName_);

    scriptFileName_.onChange([this]() { script_.setFilename(scriptFileName_.get()); });

    script_.onChange([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void PythonScriptProcessor::process() {
    if (script_.getSource().empty()) {
        mesh_.setData(std::make_shared<Mesh>());
        return;
    }
    script_.run({}, [&](pybind11::dict dict) {
        if (dict.contains("mesh")) {
            auto pyMesh = dict["mesh"];
            auto mesh = std::shared_ptr<BasicMesh>(pyMesh.cast<BasicMesh*>());
            pyMesh.cast<pybind11::object>().release();
            mesh_.setData(mesh);
        }

        if (dict.contains("volume")) {
            auto pyVolume = dict["volume"];
            auto volume = std::shared_ptr<Volume>(pyVolume.cast<Volume*>());
            pyVolume.cast<pybind11::object>().release();
            volume_.setData(volume);
        }
    });
}

}  // namespace inviwo
