/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/base/io/nrrdreader.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/rawvolumeramloader.h>
#include <inviwo/core/io/bytereaderutil.h>
#include <inviwo/core/io/inviwofileformattypes.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/charconv.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/algorithm/algorithmoptions.h>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <fmt/std.h>

namespace inviwo {

namespace {

struct NrrdState {
    std::filesystem::path dataFile;  // for detached headers (.nhdr)
    const DataFormatBase* format = nullptr;
    int dimension = 0;
    std::vector<size_t> sizes;
    ByteOrder byteOrder = ByteOrder::LittleEndian;
    std::string encoding = "raw";
    size_t lineSkip = 0;
    size_t byteSkip = 0;
    size_t dataOffset = 0;  // byte offset to inline data (for .nrrd files)

    std::optional<std::vector<double>> spacings;
    std::optional<std::vector<std::vector<double>>> spaceDirections;
    std::optional<std::vector<double>> spaceOrigin;

    size_t components = 1;  // number of components per voxel/pixel
    std::vector<std::string> kinds;

    // Derived spatial dimensions (excluding component axis)
    size_t spatialDims() const {
        return (components > 1) ? static_cast<size_t>(dimension) - 1
                                : static_cast<size_t>(dimension);
    }

    // Spatial sizes (excluding the component axis)
    std::vector<size_t> spatialSizes() const {
        if (components > 1 && !sizes.empty()) {
            return {sizes.begin() + 1, sizes.end()};
        }
        return sizes;
    }
};

const DataFormatBase* nrrdTypeToFormat(const std::string_view typeStr, size_t components) {
    // Map NRRD type string to (NumericType, precision_in_bits)
    static const std::unordered_map<std::string, std::pair<NumericType, size_t>> typeMap = {
        {"signed char", {NumericType::SignedInteger, 8}},
        {"int8", {NumericType::SignedInteger, 8}},
        {"int8_t", {NumericType::SignedInteger, 8}},
        {"uchar", {NumericType::UnsignedInteger, 8}},
        {"unsigned char", {NumericType::UnsignedInteger, 8}},
        {"uint8", {NumericType::UnsignedInteger, 8}},
        {"uint8_t", {NumericType::UnsignedInteger, 8}},
        {"short", {NumericType::SignedInteger, 16}},
        {"int16", {NumericType::SignedInteger, 16}},
        {"int16_t", {NumericType::SignedInteger, 16}},
        {"ushort", {NumericType::UnsignedInteger, 16}},
        {"unsigned short", {NumericType::UnsignedInteger, 16}},
        {"uint16", {NumericType::UnsignedInteger, 16}},
        {"uint16_t", {NumericType::UnsignedInteger, 16}},
        {"int", {NumericType::SignedInteger, 32}},
        {"int32", {NumericType::SignedInteger, 32}},
        {"int32_t", {NumericType::SignedInteger, 32}},
        {"uint", {NumericType::UnsignedInteger, 32}},
        {"unsigned int", {NumericType::UnsignedInteger, 32}},
        {"uint32", {NumericType::UnsignedInteger, 32}},
        {"uint32_t", {NumericType::UnsignedInteger, 32}},
        {"longlong", {NumericType::SignedInteger, 64}},
        {"long long", {NumericType::SignedInteger, 64}},
        {"int64", {NumericType::SignedInteger, 64}},
        {"int64_t", {NumericType::SignedInteger, 64}},
        {"ulonglong", {NumericType::UnsignedInteger, 64}},
        {"unsigned long long", {NumericType::UnsignedInteger, 64}},
        {"uint64", {NumericType::UnsignedInteger, 64}},
        {"uint64_t", {NumericType::UnsignedInteger, 64}},
        {"float", {NumericType::Float, 32}},
        {"double", {NumericType::Float, 64}},
    };

    const auto it = typeMap.find(toLower(typeStr));
    if (it == typeMap.end()) {
        return nullptr;
    }
    return DataFormatBase::get(it->second.first, components, it->second.second);
}

// Parse a parenthesized vector like "(1.0,2.0,3.0)"
std::optional<std::vector<double>> parseVector(std::string_view s) {
    auto trimmed = util::trim(s);
    if (trimmed == "none" || trimmed == "None" || trimmed == "NONE") {
        return std::nullopt;
    }
    if (trimmed.front() != '(' || trimmed.back() != ')') {
        return std::nullopt;
    }
    trimmed = trimmed.substr(1, trimmed.size() - 2);
    std::vector<double> result;
    util::forEachStringPart(trimmed, ",", [&](std::string_view part) {
        result.push_back(
            util::fromStr<double>(util::trim(part))
                .or_else([&](std::string_view error) -> std::expected<double, std::string_view> {
                    throw Exception(SourceContext{}, "{} ({})", error, part);
                })
                .value());
    });
    return result;
}

// Split a string by whitespace respecting parenthesized groups
std::vector<std::string> splitRespectingParens(std::string_view s) {
    std::vector<std::string> tokens;
    std::string current;
    int depth = 0;
    for (const char c : s) {
        if (c == '(') {
            depth++;
            current += c;
        } else if (c == ')') {
            depth--;
            current += c;
        } else if (std::isspace(static_cast<unsigned char>(c)) && depth == 0) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

bool isComponentKind(std::string_view kind) {
    static constexpr auto componentKinds = std::to_array<std::string_view>(
        {"vector", "rgb-color", "rgba-color", "hsv-color", "hsl-color", "2-vector", "3-vector",
         "4-vector", "2d-symmetric-matrix", "3d-symmetric-matrix"});

    return std::ranges::any_of(componentKinds,
                               [&](auto k) { return CaseInsensitiveEqual{}(k, kind); });
}

template <typename T>
T parse(std::string_view s, std::string_view key) {
    return util::fromStr<T>(s)
        .or_else([&](std::string_view error) -> std::expected<T, std::string_view> {
            throw Exception(SourceContext{}, "Error parsing key: '{}' got '{}'. {}", key, s, error);
        })
        .value();
}

NrrdState parseNrrdHeader(const std::filesystem::path& filePath) {
    NrrdState state;

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw DataReaderException(SourceContext{}, "Could not open NRRD file: {}", filePath);
    }

    // Read and validate magic line
    std::string magic;
    std::getline(file, magic);
    // Remove potential \r
    if (!magic.empty() && magic.back() == '\r') {
        magic.pop_back();
    }
    if (magic.substr(0, 4) != "NRRD") {
        throw DataReaderException(SourceContext{}, "Not a valid NRRD file (bad magic): {}",
                                  filePath);
    }

    std::string typeStr;

    // Parse header fields
    std::string line;
    while (std::getline(file, line)) {
        // Remove trailing \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Blank line terminates the header
        if (line.empty()) {
            break;
        }

        // Skip comments
        if (line[0] == '#') continue;

        // Determine separator: ":=" for key-value pairs, ":" for fields
        auto colonEq = line.find(":=");
        if (colonEq != std::string::npos) {
            // key-value pair (not a field specification, skip)
            continue;
        }
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;

        const std::string key = toLower(util::trim(line.substr(0, colon)));
        const auto value = util::trim(line.substr(colon + 1));

        if (key == "type") {
            typeStr = value;
        } else if (key == "dimension") {
            state.dimension = parse<int>(value, "dimension");
        } else if (key == "sizes") {
            util::forEachStringPart(value, " ", [&](std::string_view part) {
                state.sizes.push_back(parse<size_t>(part, "sizes"));
            });

        } else if (key == "spacings") {
            state.spacings.emplace();
            util::forEachStringPart(value, " ", [&](std::string_view part) {
                state.spacings->push_back(parse<double>(part, "spacings"));
            });

            // default for non-spatial axes
            std::ranges::transform(*state.spacings, state.spacings->begin(),
                                   [](double s) { return std::isnan(s) ? 1.0 : s; });

        } else if (key == "space directions") {
            state.spaceDirections.emplace();
            auto tokens = splitRespectingParens(value);
            for (const auto& t : tokens) {
                auto vec = parseVector(t);
                if (vec) {
                    state.spaceDirections->push_back(*vec);
                } else {
                    // "none" for non-spatial axis - push empty vector as placeholder
                    state.spaceDirections->push_back({});
                }
            }
        } else if (key == "space origin") {
            auto vec = parseVector(value);
            if (vec) {
                state.spaceOrigin = *vec;
            }
        } else if (key == "encoding") {
            state.encoding = toLower(value);
        } else if (key == "endian") {
            const auto val = toLower(value);
            if (val == "big") {
                state.byteOrder = ByteOrder::BigEndian;
            } else {
                state.byteOrder = ByteOrder::LittleEndian;
            }
        } else if (key == "data file" || key == "datafile") {
            state.dataFile = value;
        } else if (key == "line skip" || key == "lineskip") {
            state.lineSkip = parse<size_t>(value, "line skip");
        } else if (key == "byte skip" || key == "byteskip") {
            state.byteSkip = parse<size_t>(value, "byte skip");
        } else if (key == "kinds") {
            auto tokens = splitRespectingParens(value);
            for (auto& t : tokens) {
                state.kinds.emplace_back(util::trim(t));
            }
        }
    }

    // Record position after header for inline data
    state.dataOffset = static_cast<size_t>(file.tellg());

    // Handle line skip for inline data
    if (state.dataFile.empty()) {
        for (size_t i = 0; i < state.lineSkip; ++i) {
            std::string skip;
            std::getline(file, skip);
        }
        state.dataOffset = static_cast<size_t>(file.tellg());
        state.dataOffset += state.byteSkip;
    }

    // Validate required fields
    if (state.dimension == 0) {
        throw DataReaderException(SourceContext{}, "NRRD file missing 'dimension' field: {}",
                                  filePath);
    }
    if (state.sizes.empty()) {
        throw DataReaderException(SourceContext{}, "NRRD file missing 'sizes' field: {}", filePath);
    }
    if (static_cast<int>(state.sizes.size()) != state.dimension) {
        throw DataReaderException(SourceContext{},
                                  "NRRD 'sizes' count ({}) does not match 'dimension' ({}) in: {}",
                                  state.sizes.size(), state.dimension, filePath);
    }

    // Determine components from kinds
    if (!state.kinds.empty() && isComponentKind(state.kinds[0])) {
        state.components = state.sizes[0];
        if (state.components < 1 || state.components > 4) {
            throw DataReaderException(SourceContext{},
                                      "Unsupported number of components ({}) in NRRD file: {}",
                                      state.components, filePath);
        }
    }

    // Resolve format
    if (typeStr.empty()) {
        throw DataReaderException(SourceContext{}, "NRRD file missing 'type' field: {}", filePath);
    }
    state.format = nrrdTypeToFormat(typeStr, state.components);
    if (!state.format || state.format->getId() == DataFormatId::NotSpecialized) {
        throw DataReaderException(SourceContext{},
                                  "Unsupported NRRD type '{}' with {} components in: {}", typeStr,
                                  state.components, filePath);
    }

    // Validate encoding
    if (state.encoding != "raw" && state.encoding != "gzip" && state.encoding != "gz" &&
        state.encoding != "bzip2" && state.encoding != "bz2") {
        throw DataReaderException(SourceContext{},
                                  "Unsupported NRRD encoding '{}' in: {}. "
                                  "Supported encodings: raw, gzip, bzip2",
                                  state.encoding, filePath);
    }

    return state;
}

Compression encodingToCompression(const std::string_view encoding) {
    if (encoding == "gzip" || encoding == "gz" || encoding == "bzip2" || encoding == "bz2") {
        return Compression::Enabled;
    }
    return Compression::Disabled;
}

std::filesystem::path resolveDataFile(const NrrdState& state,
                                      const std::filesystem::path& headerPath,
                                      const std::filesystem::path& originalHeaderPath) {
    if (!state.dataFile.empty()) {
        if (state.dataFile.is_absolute()) {
            return state.dataFile;
        }
        return originalHeaderPath.parent_path() / state.dataFile;
    }
    // Inline data: data is in the same file
    return headerPath;
}

size_t resolveDataOffset(const NrrdState& state) {
    if (!state.dataFile.empty()) {
        // Detached header: data file starts from the beginning (plus byte skip)
        return state.byteSkip;
    }
    // Inline: offset computed during parsing (includes line skip and byte skip)
    return state.dataOffset;
}

// Build basis matrix and offset from NRRD spatial metadata.
// Returns {basis (3x3 column-major), offset (3-vec)}
std::pair<mat3, vec3> buildSpatialInfo(const NrrdState& state) {
    auto sSizes = state.spatialSizes();

    dmat3 basis{1.0};

    if (state.spaceDirections) {
        // in NRRD, space directions already encode the per-sample step,
        // so basis column i = spaceDirections[spatialIdx] * sizes[spatialIdx]
        size_t spatialIdx = 0;
        for (size_t i = 0; i < state.spaceDirections->size(); ++i) {
            const auto& dir = (*state.spaceDirections)[i];
            if (dir.empty()) continue;  // "none" axis (component axis)
            if (spatialIdx >= 3) break;
            if (dir.size() >= 3 && spatialIdx < sSizes.size()) {
                basis[spatialIdx] = dvec3{dir[0] * static_cast<double>(sSizes[spatialIdx]),
                                          dir[1] * static_cast<double>(sSizes[spatialIdx]),
                                          dir[2] * static_cast<double>(sSizes[spatialIdx])};
            }
            spatialIdx++;
        }
    } else if (state.spacings) {
        // spacings: use as diagonal scaling
        const size_t spatialStart = (state.components > 1) ? 1 : 0;
        for (size_t i = 0;
             i < 3 && (i + spatialStart) < state.spacings->size() && i < sSizes.size(); ++i) {
            const auto sp = (*state.spacings)[i + spatialStart];
            basis[i][i] = sp * static_cast<double>(sSizes[i]);
        }
    } else {
        // No spatial info: unit spacing
        for (size_t i = 0; i < 3 && i < sSizes.size(); ++i) {
            basis[i][i] = static_cast<double>(sSizes[i]);
        }
    }

    dvec3 offset{};
    if (state.spaceOrigin && state.spaceOrigin->size() >= 3) {
        offset = dvec3{(*state.spaceOrigin)[0], (*state.spaceOrigin)[1], (*state.spaceOrigin)[2]};
    } else {
        // Default offset: center the data at origin
        offset = -0.5 * (basis[0] + basis[1] + basis[2]);
    }

    return {basis, offset};
}
}  // namespace

NrrdLayerReader::NrrdLayerReader() : DataReaderType<Layer>() {
    addExtension(FileExtension("nrrd", "NRRD file format (Layer)"));
    addExtension(FileExtension("nhdr", "NRRD detached header (Layer)"));
}

NrrdLayerReader* NrrdLayerReader::clone() const { return new NrrdLayerReader(*this); }

std::shared_ptr<Layer> NrrdLayerReader::readData(const std::filesystem::path& filePath) {
    const auto localPath = downloadAndCacheIfUrl(filePath);
    checkExists(localPath);

    auto state = parseNrrdHeader(localPath);
    const auto spatialDims = state.spatialDims();

    if (spatialDims != 2) {
        throw DataReaderException(
            SourceContext{},
            "NrrdLayerReader requires 2D data, but NRRD file has {} spatial dimensions: {}",
            spatialDims, filePath);
    }

    auto sSizes = state.spatialSizes();
    const size2_t dimensions{sSizes[0], sSizes[1]};
    const auto dataPath = resolveDataFile(state, localPath, filePath);
    const auto dataOff = resolveDataOffset(state);
    const auto compression = encodingToCompression(state.encoding);
    const auto swizzle = swizzlemasks::defaultData(state.components);

    const size_t bytes = dimensions.x * dimensions.y * state.format->getSizeInBytes();

    auto layerRAM = createLayerRAM(dimensions, LayerType::Color, state.format, swizzle,
                                   InterpolationType::Linear, wrapping2d::clampAll);

    if (compression == Compression::Enabled) {
        util::readCompressedBytesIntoBuffer(dataPath, dataOff, bytes, state.byteOrder,
                                            state.format->getSizeInBytes() / state.components,
                                            layerRAM->getData());
    } else {
        util::readBytesIntoBuffer(dataPath, dataOff, bytes, state.byteOrder,
                                  state.format->getSizeInBytes() / state.components,
                                  layerRAM->getData());
    }

    auto layer = std::make_shared<Layer>(layerRAM);
    layer->setModelMatrix(LayerConfig::aspectPreservingModelMatrixFromDimensions(dimensions));

    return layer;
}

// --- NrrdVolumeReader ---

NrrdVolumeReader::NrrdVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("nrrd", "NRRD file format (Volume)"));
    addExtension(FileExtension("nhdr", "NRRD detached header (Volume)"));
}

NrrdVolumeReader* NrrdVolumeReader::clone() const { return new NrrdVolumeReader(*this); }

std::shared_ptr<Volume> NrrdVolumeReader::readData(const std::filesystem::path& filePath) {
    const auto localPath = downloadAndCacheIfUrl(filePath);
    checkExists(localPath);

    auto state = parseNrrdHeader(localPath);
    const auto spatialDims = state.spatialDims();

    if (spatialDims == 4) {
        // 4D: check if the 4th spatial axis has size > 1
        auto sSizes = state.spatialSizes();
        if (sSizes[3] > 1) {
            throw DataReaderException(
                SourceContext{},
                "Volume sequences (4D NRRD) are not handled by the NrrdVolumeReader, "
                "use the NrrdVolumeSequenceReader: {}",
                filePath);
        }
    } else if (spatialDims != 3) {
        throw DataReaderException(
            SourceContext{},
            "NrrdVolumeReader requires 3D data, but NRRD file has {} spatial dimensions: {}",
            spatialDims, filePath);
    }

    auto sSizes = state.spatialSizes();
    const size3_t dimensions{sSizes[0], sSizes[1], sSizes[2]};
    const auto dataPath = resolveDataFile(state, localPath, filePath);
    const auto dataOff = resolveDataOffset(state);
    const auto compression = encodingToCompression(state.encoding);

    auto [basis, offset] = buildSpatialInfo(state);

    auto volume = std::make_shared<Volume>(dimensions, state.format,
                                           swizzlemasks::defaultData(state.components),
                                           InterpolationType::Linear, wrapping3d::clampAll);
    volume->setBasis(basis);
    volume->setOffset(offset);

    auto volumeDisk = std::make_shared<VolumeDisk>(localPath, dimensions, state.format,
                                                   swizzlemasks::defaultData(state.components),
                                                   InterpolationType::Linear, wrapping3d::clampAll);
    auto loader =
        std::make_unique<RawVolumeRAMLoader>(dataPath, dataOff, state.byteOrder, compression);
    volumeDisk->setLoader(loader.release());
    volume->addRepresentation(volumeDisk);

    // Compute data range from data
    auto minmax = util::volumeMinMax(volume.get(), IgnoreSpecialValues::No);
    dvec2 computedRange(minmax.first[0], minmax.second[0]);
    for (size_t c = 1; c < state.format->getComponents(); ++c) {
        computedRange = dvec2(glm::min(computedRange[0], minmax.first[c]),
                              glm::max(computedRange[1], minmax.second[c]));
    }
    volume->dataMap.dataRange = computedRange;
    volume->dataMap.valueRange = computedRange;

    log::info("Loaded NRRD volume: {} dimensions: {}x{}x{}", filePath, dimensions.x, dimensions.y,
              dimensions.z);

    return volume;
}

// --- NrrdVolumeSequenceReader ---

NrrdVolumeSequenceReader::NrrdVolumeSequenceReader() : DataReaderType<VolumeSequence>() {
    addExtension(FileExtension("nrrd", "NRRD file format (VolumeSequence)"));
    addExtension(FileExtension("nhdr", "NRRD detached header (VolumeSequence)"));
}

NrrdVolumeSequenceReader* NrrdVolumeSequenceReader::clone() const {
    return new NrrdVolumeSequenceReader(*this);
}

std::shared_ptr<VolumeSequence> NrrdVolumeSequenceReader::readData(
    const std::filesystem::path& filePath) {

    const auto localPath = downloadAndCacheIfUrl(filePath);
    checkExists(localPath);

    auto state = parseNrrdHeader(localPath);
    const auto spatialDims = state.spatialDims();

    if (spatialDims < 3 || spatialDims > 4) {
        throw DataReaderException(
            SourceContext{},
            "NrrdVolumeSequenceReader requires 3D or 4D data, but NRRD file has {} spatial "
            "dimensions: {}",
            spatialDims, filePath);
    }

    auto sSizes = state.spatialSizes();
    const size3_t dimensions{sSizes[0], sSizes[1], sSizes[2]};
    const size_t timeSteps = (spatialDims == 4) ? sSizes[3] : 1;

    const auto dataPath = resolveDataFile(state, localPath, filePath);
    const auto dataOff = resolveDataOffset(state);
    const auto compression = encodingToCompression(state.encoding);

    auto [basis, offset] = buildSpatialInfo(state);

    const size_t bytesPerVolume = glm::compMul(dimensions) * state.format->getSizeInBytes();

    auto volumes = std::make_shared<VolumeSequence>();

    for (size_t t = 0; t < timeSteps; ++t) {
        auto volume = std::make_shared<Volume>(dimensions, state.format,
                                               swizzlemasks::defaultData(state.components),
                                               InterpolationType::Linear, wrapping3d::clampAll);
        volume->setBasis(basis);
        volume->setOffset(offset);

        auto volumeDisk = std::make_shared<VolumeDisk>(
            localPath, dimensions, state.format, swizzlemasks::defaultData(state.components),
            InterpolationType::Linear, wrapping3d::clampAll);
        auto loader = std::make_unique<RawVolumeRAMLoader>(dataPath, dataOff + t * bytesPerVolume,
                                                           state.byteOrder, compression);
        volumeDisk->setLoader(loader.release());
        volume->addRepresentation(volumeDisk);

        if (t == 0) {
            // Compute data range from first time step
            auto minmax = util::volumeMinMax(volume.get(), IgnoreSpecialValues::No);
            dvec2 computedRange(minmax.first[0], minmax.second[0]);
            for (size_t c = 1; c < state.format->getComponents(); ++c) {
                computedRange = dvec2(glm::min(computedRange[0], minmax.first[c]),
                                      glm::max(computedRange[1], minmax.second[c]));
            }
            volume->dataMap.dataRange = computedRange;
            volume->dataMap.valueRange = computedRange;
        } else {
            // Copy data range from the first volume
            volume->dataMap = volumes->front()->dataMap;
        }

        volumes->push_back(std::move(volume));
    }

    log::info("Loaded NRRD volume sequence: {} volumes: {} dimensions: {}x{}x{}", filePath,
              timeSteps, dimensions.x, dimensions.y, dimensions.z);

    return volumes;
}

}  // namespace inviwo
