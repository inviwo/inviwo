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

#include <modules/opactopt/io/amirameshreader.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, IndexBuffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for Vec3BufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, DrawType
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Buffe...
#include <inviwo/core/datastructures/buffer/buffer.h>                   // Buffer
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datareader.h>                                  // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4
#include <inviwo/core/util/logcentral.h>                                // for LogVerbosity, Log...
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT
#include <inviwo/core/util/stringconversion.h>                          // for splitStringView
#include <modules/opengl/inviwoopengl.h>

#include <algorithm>      // for max
#include <array>          // for array, array<>::v...
#include <cstdint>        // for uint32_t
#include <cstring>        // for strlen
#include <ctime>          // for size_t, clock
#include <string>         // for basic_string<>::v...
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector
#include <fmt/format.h>
#include <fmt/std.h>

namespace inviwo {

AmiraMeshReader::AmiraMeshReader() : DataReaderType<Mesh>() {
    addExtension(FileExtension("am", "AMIRA mesh reader"));
}

AmiraMeshReader* AmiraMeshReader::clone() const { return new AmiraMeshReader(*this); }

std::shared_ptr<Mesh> AmiraMeshReader::readData(const std::filesystem::path& filePath) {
    auto& path = filePath;
    std::ifstream f(path, std::ios::in);

    if (!f) {
        LogError("Could not open file");
        return nullptr;
    }

    // Parse header
    std::string line;

    std::getline(f, line);
    if (line != "# AmiraMesh 3D ASCII 2.0" && line != "# Avizo 3D ASCII 2.0") {
        LogError("Unsupported AMIRA mesh format");
        return nullptr;
    }

    int nLines, nVertices;

    std::map<int, AmiraDataType> bufferMap;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        if (line.substr(0, 12) == "define Lines") {
            std::istringstream l(line.substr(13));
            l >> nLines;
        } else if (line.substr(0, 9) == "nVertices") {
            std::istringstream v(line.substr(10));
            v >> nVertices;
        } else if (line.substr(0, 11) == "Lines { int") {
            int index;
            std::istringstream i(line.substr(line.find("@") + 1));
            i >> index;
            bufferMap.insert({index, AmiraDataType::Lines});
        } else if (line.substr(0, 18) == "Vertices { float[3") {
            int index;
            std::istringstream i(line.substr(line.find("@") + 1));
            i >> index;
            bufferMap.insert({index, AmiraDataType::Vertices});
        } else if (line.substr(0, 17) == "Vertices { float ") {
            int index;
            std::istringstream i(line.substr(line.find("@") + 1));
            i >> index;
            bufferMap.insert({index, AmiraDataType::Importance});
        } else if (line == "# Data section follows") {
            break;
        }
    }

    // Parse data
    glPrimitiveRestartIndex(UINT32_MAX);
    auto mesh = std::make_shared<Mesh>();
    AmiraDataType at;
    while (std::getline(f, line)) {
        if (line.substr(0, 1) == "@") {
            int index;
            std::istringstream i(line.substr(line.find("@") + 1));
            i >> index;
            at = bufferMap[index];

            if (at == AmiraDataType::Lines)
                processLines(f, mesh);
            else if (at == AmiraDataType::Vertices)
                processVertices(f, mesh);
            else if (at == AmiraDataType::Importance)
                processImportance(f, mesh);
        }
    }

    return mesh;
}

void AmiraMeshReader::processLines(std::ifstream& fs, std::shared_ptr<Mesh> mesh) {
    std::string line;
    std::vector<uint32_t> indices;
    while (std::getline(fs, line) && !line.empty()) {
        uint32_t index;
        std::istringstream i(line);
        i >> index;

        if (index == -1) {
            mesh->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                             util::makeIndexBuffer(std::move(indices)));
            indices = {};
        } else {
            indices.push_back(index);
        }
    }
}

void AmiraMeshReader::processVertices(std::ifstream& fs, std::shared_ptr<Mesh> mesh) {
    std::string line;
    std::vector<vec3> vertices;
    while (std::getline(fs, line) && !line.empty()) {
        double x, y, z;
        std::istringstream i(line);
        i >> x;
        i >> y;
        i >> z;
        vertices.push_back({x, y, z});
    }
    mesh->addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                    util::makeBuffer(std::move(vertices)));
}

void AmiraMeshReader::processImportance(std::ifstream& fs, std::shared_ptr<Mesh> mesh) {
    std::string line;
    std::vector<float> importance;
    while (std::getline(fs, line) && !line.empty()) {
        float imp;
        std::istringstream i(line);
        i >> imp;
        importance.push_back(imp);
    }
    mesh->addBuffer(Mesh::BufferInfo(BufferType::TexCoordAttrib),
                    util::makeBuffer(std::move(importance)));
}

}  // namespace inviwo
