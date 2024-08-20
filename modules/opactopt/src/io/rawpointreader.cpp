/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/opactopt/io/rawpointreader.h>

#include <inviwo/core/io/datareaderexception.h>  // for DataReaderException
#include <inviwo/core/util/raiiutils.h>

#include <vector>  // for vector

namespace inviwo {

RawPointReader::RawPointReader() : DataReaderType<Mesh>() {
    addExtension(FileExtension("raw", "Raw point reader"));
}

RawPointReader* RawPointReader::clone() const { return new RawPointReader(*this); }

std::shared_ptr<Mesh> RawPointReader::readData(const std::filesystem::path& filePath) {
    auto fin = std::ifstream(filePath, std::ios::in | std::ios::binary);
    util::OnScopeExit close([&fin]() { fin.close(); });

    auto mesh = std::make_shared<Mesh>();
    if (fin.good()) {
        fin.seekg(0, fin.end);
        int bytes = fin.tellg();
        int N = bytes / sizeof(vec3);
        std::vector<vec3> dest(N);

        fin.seekg(0, fin.beg);
        fin.read(reinterpret_cast<char*>(dest.data()), bytes);
        LogInfo("Read " << N << " points");

        mesh->addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                        util::makeBuffer(std::move(dest)));
    } else {
        LogError("Error reading file " << filePath);
        return nullptr;
    }

    return mesh;
}

}  // namespace inviwo
