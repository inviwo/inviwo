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
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
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

// TODO(when needed): implement support for HxTriangularGrid
// see https://github.com/strawlab/py_amira_file_reader/blob/master/tests/data/hybrid-testgrid-2d.am

namespace {

std::expected<std::pair<std::string_view, amira::DataSpec>, std::string> getDataSpecForType(
    const amira::AmiraMeshHeader& header, std::string_view type) {
    if (auto it = header.dataSpecs.find(type); it == header.dataSpecs.end()) {
        return std::unexpected(format("Missing data specifier for type '{}'", type));
    } else {
        return *it;
    }
}

void parseIndices(Mesh& mesh, std::string_view data) {
    const auto end = data.find('@');
    std::vector<std::uint32_t> indices;
    for (size_t first = 0; first < end;) {
        size_t second = data.find_first_of("\r\n", first);
        std::string_view line = data.substr(first, second - first);

        auto [str, _] = util::splitByFirst(line, '#');
        str = util::trim(str);

        if (!str.empty()) {
            // expecting a single value
            std::int64_t value;
            amira::fromStr(str, value);
            if (value == -1) {
                mesh.addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                                util::makeIndexBuffer(std::move(indices)));
                indices.clear();
            } else {
                indices.push_back(static_cast<uint32_t>(value));
            }
        }

        first = second + (data.substr(second, 2).starts_with("\r\n") ? 2 : 1);
    }
    mesh.addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                    util::makeIndexBuffer(std::move(indices)));
}

template <typename T>
T parseValue(std::string_view line) {
    T value{};
    const auto tokens = amira::split<5>(line, ' ');
    for (int i = 0; i < util::extent_v<T>; ++i) {
        if constexpr (util::extent_v<T> == 1) {
            amira::fromStr(tokens[i], value);
        } else {
            amira::fromStr(tokens[i], value[i]);
        }
    }
    return value;
}

std::shared_ptr<BufferBase> parseBufferData(std::string_view data,
                                            const amira::DataSpec& dataSpec) {
    const auto* format = DataFormatBase::get(
        dataSpec.format.numericType, dataSpec.format.numComponents, dataSpec.format.precision);

    return dispatching::singleDispatch<std::shared_ptr<BufferBase>, dispatching::filter::All>(
        format->getId(), [data]<typename T>() {
            const auto end = data.find('@');
            std::vector<T> vec;

            for (size_t first = 0; first < end;) {
                size_t second = data.find_first_of("\r\n", first);
                std::string_view line = data.substr(first, second - first);

                auto [str, _] = util::splitByFirst(line, '#');
                str = util::trim(str);
                if (!str.empty()) {
                    vec.emplace_back(parseValue<T>(str));
                }
                if (second == std::string_view::npos) {
                    first = std::string_view::npos;
                } else {
                    first = second + (data.substr(second).starts_with("\r\n") ? 2 : 1);
                }
            }
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

    FILE* file = filesystem::fopen(filePath, "rb");
    if (!file) {
        throw DataReaderException(SourceContext{}, "Could not open file: {}", path);
    }
    const util::OnScopeExit closeFile{[file]() { std::fclose(file); }};

    // Get the file size, so we can pre-allocate the string. HUGE speed impact.
    fseek(file, 0, SEEK_END);
    const auto length = std::ftell(file);  // NOLINT(google-runtime-int)
    fseek(file, 0, SEEK_SET);

    if (length <= 0) {
        throw DataReaderException(SourceContext{}, "Empty AmiraMesh file: {}", path);
    }

    std::pmr::string data(length, '0');
    if (std::fread(data.data(), length, 1, file) != 1) {
        throw DataReaderException(SourceContext{}, "Could not read AmiraMesh file: {}", path);
    }

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
        //} else if (iCaseCmp(it->second, "HxTriangularGrid")) {
        //    meshType = MeshType::Triangles;
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

        if (!indexSpec.has_value()) {
            throw DataReaderException(SourceContext{}, "{}: {}", indexSpec.error(), path);
        }
        if (!coordinatesSpec.has_value()) {
            throw DataReaderException(SourceContext{}, "{}: {}", coordinatesSpec.error(), path);
        }

        auto mesh = std::make_shared<Mesh>();
        parseIndices(*mesh, std::string_view{data}.substr(
                                locateData(indexSpec->first, indexSpec->second.identifier)));
        mesh->addBuffer(
            Mesh::BufferInfo(BufferType::PositionAttrib),
            parseBufferData(std::string_view{data}.substr(locateData(
                                coordinatesSpec->first, coordinatesSpec->second.identifier)),
                            coordinatesSpec->second));

        if (vertexDataSpec) {
            mesh->addBuffer(
                Mesh::BufferInfo(BufferType::Unknown),  // TODO: change to ScalarMetaAttrib
                parseBufferData(std::string_view{data}.substr(locateData(
                                    vertexDataSpec->first, vertexDataSpec->second.identifier)),
                                vertexDataSpec->second));
        }
        return mesh;
    } else if (meshType == MeshType::Triangles) {
        // TODO
        throw DataReaderException(SourceContext{}, "Not implemented");
    } else {
        throw DataReaderException(SourceContext{}, "Unsupported mesh type: {}", path);
    }
}

}  // namespace inviwo
