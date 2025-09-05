/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/io/amirameshreader.h>

#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/sourcecontext.h>
#include <inviwo/core/util/filesystem.h>

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <numeric>
#include <fmt/std.h>

namespace inviwo {

// TODO: implement support for HxTriangularGrid
// see https://github.com/strawlab/py_amira_file_reader/blob/master/tests/data/hybrid-testgrid-2d.am

namespace {

void checkContentType(std::string_view str, const std::filesystem::path& path) {
    if (!str.ends_with(R"("HxLineSet")")) {
        throw DataReaderException(
            SourceContext{},
            "Unsupported AmiraMesh content type: {} ({})\nOnly \"HxLineSet\" is supported.", str,
            path);
    }
}

void processLines(std::ifstream& fs, Mesh& mesh) {
    std::string line;
    std::vector<uint32_t> indices;
    while (std::getline(fs, line) && !line.empty()) {
        int64_t index = -1;
        std::istringstream i(line);
        i >> index;

        if (index == -1) {
            mesh.addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                            util::makeIndexBuffer(std::move(indices)));
            indices.clear();
        } else {
            indices.push_back(static_cast<uint32_t>(index));
        }
    }
}

void processVertices(std::ifstream& fs, Mesh& mesh) {
    std::string line;
    std::vector<vec3> vertices;
    while (std::getline(fs, line) && !line.empty()) {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        std::istringstream i(line);
        i >> x;
        i >> y;
        i >> z;
        vertices.emplace_back(x, y, z);
    }
    mesh.addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                   util::makeBuffer(std::move(vertices)));
}

void processVertexData(std::ifstream& fs, Mesh& mesh) {
    std::string line;
    std::vector<float> vertexData;
    while (std::getline(fs, line) && !line.empty()) {
        float imp = 0.0f;
        std::istringstream i(line);
        i >> imp;
        vertexData.push_back(imp);
    }
    mesh.addBuffer(Mesh::BufferInfo(BufferType::Unknown), util::makeBuffer(std::move(vertexData)));
}

}  // namespace

AmiraMeshReader::AmiraMeshReader() : DataReaderType<Mesh>() {
    addExtension(FileExtension("am", "AMIRA mesh reader"));
}

AmiraMeshReader* AmiraMeshReader::clone() const { return new AmiraMeshReader(*this); }

std::shared_ptr<Mesh> AmiraMeshReader::readData(const std::filesystem::path& path) {
    const auto filePath = downloadAndCacheIfUrl(path);

    checkExists(filePath);

    std::ifstream f = open(filePath, std::ios::in);

    // Parse header
    std::string line;

    std::getline(f, line);
    if (line != "# AmiraMesh 3D ASCII 2.0" && line != "# Avizo 3D ASCII 2.0") {
        throw DataReaderException(SourceContext{}, "Unsupported AmiraMesh format: {}", path);
    }

    auto extractIndex = [](const std::string& str) {
        int index = 0;
        std::istringstream i(str.substr(str.find('@') + 1));
        i >> index;
        return index;
    };

    int nVertices = 0;
    std::map<int, AmiraDataType> bufferMap;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        if (line.starts_with("define Lines")) {
            std::istringstream l(line.substr(13));
            int nLines = 0;
            l >> nLines;
        } else if (line.starts_with("nVertices")) {
            std::istringstream v(line.substr(10));
            v >> nVertices;
        } else if (line.contains("Lines { int LineIdx }")) {
            bufferMap.insert({extractIndex(line), AmiraDataType::Lines});
        } else if (line.contains("Vertices { float[3] Coordinates }")) {
            bufferMap.insert({extractIndex(line), AmiraDataType::Vertices});
        } else if (line.contains("Vertices { float Data }")) {
            bufferMap.insert({extractIndex(line), AmiraDataType::VertexData});
        } else if (line == "# Data section follows") {
            break;
        } else if (trim(line).starts_with("ContentType")) {
            checkContentType(line, path);
        }
    }

    // Parse data
    auto mesh = std::make_shared<Mesh>();
    while (std::getline(f, line)) {
        if (line.starts_with("@")) {
            if (const AmiraDataType at = bufferMap[extractIndex(line)];
                at == AmiraDataType::Lines) {
                processLines(f, *mesh);
            } else if (at == AmiraDataType::Vertices) {
                processVertices(f, *mesh);
            } else if (at == AmiraDataType::VertexData) {
                processVertexData(f, *mesh);
            }
        }
    }

    if (auto vertexCount = mesh->getBuffer(BufferType::PositionAttrib)->getSize();
        vertexCount != nVertices) {
        log::error("Vertex count does not match. Expected {} vertices and found {}: {}", nVertices,
                   vertexCount, path);
    }

    return mesh;
}

}  // namespace inviwo
