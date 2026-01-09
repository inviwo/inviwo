/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/charconv.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/sourcecontext.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/io/amirameshutils.h>

#include <expected>
#include <string>
#include <string_view>
#include <fmt/std.h>
#include <fmt/format.h>

namespace inviwo {

namespace {

std::expected<std::pair<std::string_view, amira::DataSpec>, std::string> getDataSpecForType(
    const amira::AmiraMeshHeader& header, std::string_view type) {
    if (auto it = header.dataSpecs.find(type); it == header.dataSpecs.end()) {
        return std::unexpected(format("Missing data specifier for type '{}'", type));
    } else {
        return *it;
    }
}

void parseLineIndices(Mesh& mesh, std::string_view data) {
    const auto end = data.find('@');
    std::vector<std::uint32_t> indices;

    util::forEachLine(data.substr(0, end), [&mesh, &indices](std::string_view line) {
        auto [str, _] = util::splitByFirst(line, '#');
        str = util::trim(str);
        if (!str.empty()) {
            // expecting a single value
            std::int64_t value = 0;
            util::fromStr(str, value);
            if (value == -1) {
                mesh.addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                                util::makeIndexBuffer(std::move(indices)));
                indices.clear();
            } else {
                indices.push_back(static_cast<uint32_t>(value));
            }
        }
    });
    mesh.addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                    util::makeIndexBuffer(std::move(indices)));
}

void parseTriangleIndices(Mesh& mesh, std::string_view data, const amira::DataSpec& dataSpec,
                          std::string_view type) {
    if (dataSpec.format.numericType != NumericType::SignedInteger &&
        dataSpec.format.numericType != NumericType::UnsignedInteger) {
        throw DataReaderException(SourceContext{},
                                  "Expected integer format in data specification '{}' for '{}'",
                                  dataSpec.name, type);
    }
    if (dataSpec.format.numComponents < 3) {
        throw DataReaderException(
            SourceContext{},
            "Unsupported number of indices per primitive. Data specification '{}' for '{}' "
            "must have at least 3 components",
            dataSpec.name, type);
    }

    const auto end = data.find('@');
    std::vector<std::uint32_t> indices;

    util::forEachLine(data.substr(0, end), [&indices](std::string_view line) {
        auto [str, _] = util::splitByFirst(line, '#');
        str = util::trim(str);
        if (!str.empty()) {
            auto parts = util::splitIntoArray<3>(str, ' ');
            std::array<std::uint32_t, 3> triIndices{amira::parseValue<std::uint32_t>(parts[0]),
                                                    amira::parseValue<std::uint32_t>(parts[1]), 0};

            // decompose arbitrary n-gon into triangles
            bool odd = true;
            util::forEachStringPart(parts[2], " ",
                                    [&indices, &triIndices, &odd](std::string_view s) {
                                        triIndices[2] = amira::parseValue<std::uint32_t>(s);
                                        if (!odd) {
                                            // correct windedness
                                            std::swap(triIndices[0], triIndices[1]);
                                        }
                                        for (auto index : triIndices) {
                                            // Assumption: indices for triangles are starting at 1
                                            indices.emplace_back(index - 1);
                                        }
                                        std::ranges::rotate(triIndices, triIndices.begin() + 1);
                                        odd = !odd;
                                    });
        }
    });
    mesh.addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::None),
                    util::makeIndexBuffer(std::move(indices)));
}

std::shared_ptr<BufferBase> parseBufferData(std::string_view data,
                                            const amira::DataSpec& dataSpec) {
    const auto* format = DataFormatBase::get(
        dataSpec.format.numericType, dataSpec.format.numComponents, dataSpec.format.precision);

    return dispatching::singleDispatch<std::shared_ptr<BufferBase>, dispatching::filter::All>(
        format->getId(), [data]<typename T>() {
            const auto end = data.find('@');
            std::vector<T> vec;

            util::forEachLine(data.substr(0, end), [&vec](std::string_view line) {
                auto [str, _] = util::splitByFirst(line, '#');
                str = util::trim(str);
                if (!str.empty()) {
                    vec.emplace_back(amira::parseValue<T>(str));
                }
            });
            return util::makeBuffer(std::move(vec));
        });
}
}  // namespace

AmiraMeshReader::AmiraMeshReader() : DataReaderType<Mesh>() {
    addExtension(FileExtension("am", "AMIRA mesh reader"));
}

AmiraMeshReader* AmiraMeshReader::clone() const { return new AmiraMeshReader(*this); }

std::shared_ptr<Mesh> AmiraMeshReader::readData(const std::filesystem::path& path) {
    const auto filePath = downloadAndCacheIfUrl(path);

    checkExists(filePath);

    const std::string data = readFileContents(filePath);

    amira::AmiraMeshHeader header;
    try {
        header = amira::parseAmiraMeshHeader(data);
    } catch (DataReaderException& e) {
        throw DataReaderException(e.getContext(), "{} ({})", e.what(), path);
    }

    if (header.dimension.value_or(-1) != 3) {
        throw DataReaderException(SourceContext{},
                                  "File is not a 3D AmiraMesh file. Expected '# AmiraMesh 3D': {}",
                                  path);
    }
    if (header.format != amira::DataSectionFormat::Ascii) {
        throw DataReaderException(SourceContext{},
                                  "Unsupported data encoding. Expected 'ASCII': {}", path);
    }

    // verify mesh parameters
    enum class MeshType : std::uint8_t { Unspecified, Lines, Triangles };

    MeshType meshType = MeshType::Unspecified;
    if (auto it = header.parameters.find("ContentType"); it == header.parameters.end()) {
        throw DataReaderException(SourceContext{}, "Missing ContentType, expected uniform: {}",
                                  path);
    } else if (iCaseCmp(it->second, "HxTriangularGrid")) {
        meshType = MeshType::Triangles;
    } else if (iCaseCmp(it->second, "HxLineSet")) {
        meshType = MeshType::Lines;
    } else {
        throw DataReaderException(
            SourceContext{},
            "Unsupported ContentType '{}', expected 'HxTriangularGrid' or 'HxLineSet': {}",
            it->second, path);
    }

    // locate data section
    size_t dataSectionOffset = data.find("# Data section follows");
    if (dataSectionOffset == std::string_view::npos) {
        throw DataReaderException(SourceContext{}, "Data section not found: {}", path);
    }

    auto locateData = [&](std::string_view key, int identifier) {
        const std::string id = fmt::format("@{}", identifier);
        if (auto pos = data.find(id, dataSectionOffset); pos != std::string_view::npos) {
            // data starts after the identifier followed by a newline character
            return data.find('\n', pos) + 1;
        } else {
            throw DataReaderException(SourceContext{}, "Data for {} @{} not found: {}", key,
                                      identifier, path);
        }
    };

    if (meshType == MeshType::Lines) {
        auto indexSpec = getDataSpecForType(header, "LineIdx");
        auto coordinatesSpec = getDataSpecForType(header, "Coordinates");
        auto vertexDataSpec = getDataSpecForType(header, "Data");  // optional
        if (!vertexDataSpec.has_value()) {
            vertexDataSpec = getDataSpecForType(header, "Data0");
        }

        if (!indexSpec.has_value()) {
            throw DataReaderException(SourceContext{}, "{}: {}", indexSpec.error(), path);
        }
        if (!coordinatesSpec.has_value()) {
            throw DataReaderException(SourceContext{}, "{}: {}", coordinatesSpec.error(), path);
        }

        auto mesh = std::make_shared<Mesh>();
        parseLineIndices(*mesh, std::string_view{data}.substr(
                                    locateData(indexSpec->first, indexSpec->second.identifier)));
        mesh->addBuffer(
            Mesh::BufferInfo(BufferType::PositionAttrib),
            parseBufferData(std::string_view{data}.substr(locateData(
                                coordinatesSpec->first, coordinatesSpec->second.identifier)),
                            coordinatesSpec->second));

        if (vertexDataSpec) {
            mesh->addBuffer(
                Mesh::BufferInfo(BufferType::ScalarMetaAttrib),
                parseBufferData(std::string_view{data}.substr(locateData(
                                    vertexDataSpec->first, vertexDataSpec->second.identifier)),
                                vertexDataSpec->second));
        }
        return mesh;
    } else if (meshType == MeshType::Triangles) {
        auto coordinatesSpec = getDataSpecForType(header, "Coordinates");
        if (!coordinatesSpec.has_value()) {
            throw DataReaderException(SourceContext{}, "{}: {}", coordinatesSpec.error(), path);
        }

        // indices require a dependent lookup of 'Coordinates'
        const std::string_view coordName = coordinatesSpec->second.name;

        auto indexSpec = getDataSpecForType(header, coordName);
        auto vertexDataSpec = getDataSpecForType(header, "Data");  // optional

        if (!indexSpec.has_value()) {
            throw DataReaderException(SourceContext{}, "{}: {}", indexSpec.error(), path);
        }
        auto mesh =
            std::make_shared<Mesh>(Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None});
        parseTriangleIndices(*mesh,
                             std::string_view{data}.substr(
                                 locateData(indexSpec->first, indexSpec->second.identifier)),
                             indexSpec->second, indexSpec->first);
        mesh->addBuffer(
            Mesh::BufferInfo(BufferType::PositionAttrib),
            parseBufferData(std::string_view{data}.substr(locateData(
                                coordinatesSpec->first, coordinatesSpec->second.identifier)),
                            coordinatesSpec->second));

        if (vertexDataSpec) {
            mesh->addBuffer(
                Mesh::BufferInfo(BufferType::ScalarMetaAttrib),
                parseBufferData(std::string_view{data}.substr(locateData(
                                    vertexDataSpec->first, vertexDataSpec->second.identifier)),
                                vertexDataSpec->second));
        }
        return mesh;
    } else {
        throw DataReaderException(SourceContext{}, "Unsupported mesh type: {}", path);
    }
}

}  // namespace inviwo
