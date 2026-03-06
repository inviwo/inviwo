/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/base/processors/meshsource.h>

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/fileproperty.h>
#include <modules/base/processors/datasource.h>

#include <functional>
#include <memory>

namespace inviwo {
class InviwoApplication;
class Mesh;

const ProcessorInfo MeshSource::processorInfo_{
    "org.inviwo.GeometrySource",  // Class identifier
    "Mesh Source",                // Display name
    "Data Input",                 // Category
    CodeState::Stable,            // Code state
    Tags::CPU,                    // Tags
    "Loads a mesh"_help,
};
const ProcessorInfo& MeshSource::getProcessorInfo() const { return processorInfo_; }

MeshSource::MeshSource(InviwoApplication* app, const std::filesystem::path& filename)
    : DataSource<Mesh, MeshOutport>(util::getDataReaderFactory(app), filename, "geometry") {
    filePath.setDisplayName("Geometry file");
}

}  // namespace inviwo
