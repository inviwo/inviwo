/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <modules/base/io/datvolumesequencereader.h>

#include <inviwo/core/datastructures/camera/camera.h>      // for mat4
#include <inviwo/core/datastructures/datamapper.h>         // for DataMapper
#include <inviwo/core/datastructures/image/imagetypes.h>   // for operator>>, InterpolationType
#include <inviwo/core/datastructures/unitsystem.h>         // for Axis, Unit, defaultAxes
#include <inviwo/core/datastructures/volume/volume.h>      // for Volume, DataReaderType
#include <inviwo/core/datastructures/volume/volumedisk.h>  // for VolumeDisk
#include <inviwo/core/io/datareader.h>                     // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>            // for DataReaderException
#include <inviwo/core/io/rawvolumeramloader.h>             // for RawVolumeRAMLoader
#include <inviwo/core/metadata/metadata.h>                 // for StringMetaData
#include <inviwo/core/util/fileextension.h>                // for FileExtension
#include <inviwo/core/util/filesystem.h>                   // for getFileDirectory, isAbsolutePath
#include <inviwo/core/util/formatconversion.h>             // for formatBytesToString
#include <inviwo/core/util/formats.h>                      // for DataFormatBase, DataFormat
#include <inviwo/core/util/glmmat.h>                       // for mat3
#include <inviwo/core/util/glmvec.h>                       // for dvec2, vec3, size3_t
#include <inviwo/core/util/logcentral.h>                   // for LogCentral, LogInfo, LogWarn
#include <inviwo/core/util/sourcecontext.h>                // for IVW_CONTEXT
#include <inviwo/core/util/stringconversion.h>             // for toLower, trim, splitByFirst
#include <modules/base/algorithm/algorithmoptions.h>       // for IgnoreSpecialValues, IgnoreSpe...
#include <modules/base/algorithm/dataminmax.h>             // for volumeMinMax

#include <algorithm>      // for copy
#include <array>          // for array
#include <cstddef>        // for size_t
#include <fstream>        // for stringstream, basic_istream
#include <iterator>       // for back_insert_iterator, back_ins...
#include <optional>       // for optional, nullopt
#include <sstream>        // for basic_stringstream<>::string_type
#include <string>         // for string, char_traits, basic_string
#include <type_traits>    // for remove_reference<>::type, remo...
#include <unordered_map>  // for unordered_map, operator!=, __h...
#include <utility>        // for move, pair

#include <glm/common.hpp>              // for max, min
#include <glm/gtx/component_wise.hpp>  // for compMul
#include <glm/mat3x3.hpp>              // for mat<>::col_type
#include <glm/mat4x4.hpp>              // for mat<>::col_type
#include <glm/vec2.hpp>                // for vec<>::(anonymous)
#include <glm/vec3.hpp>                // for vec<>::(anonymous), vec, opera...
#include <glm/vec4.hpp>                // for vec
#include <llnl-units/units.hpp>             // for unit_from_string

#include <fmt/std.h>

namespace inviwo {

DatVolumeSequenceReader::DatVolumeSequenceReader()
    : DataReaderType<VolumeSequence>(), enableLogOutput_(true) {
    addExtension(FileExtension("dat", "Inviwo dat file format"));
}

DatVolumeSequenceReader::DatVolumeSequenceReader(const DatVolumeSequenceReader& rhs) = default;
DatVolumeSequenceReader& DatVolumeSequenceReader::operator=(const DatVolumeSequenceReader& that) =
    default;

DatVolumeSequenceReader* DatVolumeSequenceReader::clone() const {
    return new DatVolumeSequenceReader(*this);
}

std::shared_ptr<DatVolumeSequenceReader::VolumeSequence> DatVolumeSequenceReader::readData(
    const std::filesystem::path& filePath) {

    const auto fileDirectory = filePath.parent_path();

    // Read the dat file content
    auto f = open(filePath);

    struct State {
        std::filesystem::path rawFile;
        size3_t dimensions{0u};
        size_t byteOffset = 0u;
        const DataFormatBase* format = nullptr;
        bool littleEndian = true;

        std::string formatFlag = "";
        mat3 basis{2.0f};
        std::optional<vec3> offset = std::nullopt;
        std::optional<vec3> spacing = std::nullopt;
        std::optional<vec3> a = std::nullopt;
        std::optional<vec3> b = std::nullopt;
        std::optional<vec3> c = std::nullopt;

        std::array<Axis, 3> axes = util::defaultAxes<3>();

        mat4 wtm{1.0f};

        std::optional<dvec2> dataRange = std::nullopt;
        std::optional<dvec2> valueRange = std::nullopt;
        Unit valueUnit = Unit{};
        std::string valueName{};
        size_t sequences{1};

        SwizzleMask swizzleMask{swizzlemasks::rgba};
        InterpolationType interpolation{InterpolationType::Linear};
        Wrapping3D wrapping{wrapping3d::clampAll};

        std::unordered_map<std::string, std::string> metadata;
        std::vector<std::filesystem::path> datFiles;
    };

    using parser = void (*)(State&, std::stringstream&);

    const std::unordered_map<std::string, parser> parsers = {
        {"objectfilename", [](State& state, std::stringstream& ss) { ss >> state.rawFile; }},
        {"rawfile", [](State& state, std::stringstream& ss) { ss >> state.rawFile; }},
        {"datfile",
         [](State& state, std::stringstream& ss) { state.datFiles.push_back(ss.str()); }},
        {"byteorder",
         [](State& state, std::stringstream& ss) {
             const auto val = toLower(ss.str());
             if (val == "bigendian") {
                 state.littleEndian = false;
             } else if (val == "littleendian") {
                 state.littleEndian = true;
             } else {
                 ss.setstate(std::ios_base::failbit);
             }
         }},
        {"byteoffset", [](State& state, std::stringstream& ss) { ss >> state.byteOffset; }},
        {"sequences", [](State& state, std::stringstream& ss) { ss >> state.sequences; }},
        {"resolution",
         [](State& state, std::stringstream& ss) {
             ss >> state.dimensions.x >> state.dimensions.y >> state.dimensions.z;
         }},
        {"dimensions",
         [](State& state, std::stringstream& ss) {
             ss >> state.dimensions.x >> state.dimensions.y >> state.dimensions.z;
         }},
        {"spacing",
         [](State& state, std::stringstream& ss) {
             state.spacing.emplace();
             ss >> state.spacing->x >> state.spacing->y >> state.spacing->z;
         }},
        {"slicethickness",
         [](State& state, std::stringstream& ss) {
             state.spacing.emplace();
             ss >> state.spacing->x >> state.spacing->y >> state.spacing->z;
         }},
        {"basisvector1",
         [](State& state, std::stringstream& ss) {
             state.a.emplace();
             ss >> state.a->x >> state.a->y >> state.a->z;
         }},
        {"basisvector2",
         [](State& state, std::stringstream& ss) {
             state.b.emplace();
             ss >> state.b->x >> state.b->y >> state.b->z;
         }},
        {"basisvector3",
         [](State& state, std::stringstream& ss) {
             state.c.emplace();
             ss >> state.c->x >> state.c->y >> state.c->z;
         }},
        {"offset",
         [](State& state, std::stringstream& ss) {
             state.offset.emplace();
             ss >> state.offset->x >> state.offset->y >> state.offset->z;
         }},
        {"worldvector1",
         [](State& state, std::stringstream& ss) {
             ss >> state.wtm[0][0] >> state.wtm[1][0] >> state.wtm[2][0] >> state.wtm[3][0];
         }},
        {"worldvector2",
         [](State& state, std::stringstream& ss) {
             ss >> state.wtm[0][1] >> state.wtm[1][1] >> state.wtm[2][1] >> state.wtm[3][1];
         }},
        {"worldvector3",
         [](State& state, std::stringstream& ss) {
             ss >> state.wtm[0][2] >> state.wtm[1][2] >> state.wtm[2][2] >> state.wtm[3][2];
         }},
        {"worldvector4",
         [](State& state, std::stringstream& ss) {
             ss >> state.wtm[0][3] >> state.wtm[1][3] >> state.wtm[2][3] >> state.wtm[3][3];
         }},
        {"format",
         [](State& state, std::stringstream& ss) {
             ss >> state.formatFlag;
             // Backward support for USHORT_12 key
             if (state.formatFlag == "USHORT_12") {
                 state.format = DataUInt16::get();
                 if (!state.dataRange) {  // Check so that data range has not been set before
                     state.dataRange = dvec2(0.0, 4095.0);
                 }
             } else {
                 state.format = DataFormatBase::get(state.formatFlag);
             }
         }},
        {"datarange",
         [](State& state, std::stringstream& ss) {
             state.dataRange.emplace();
             ss >> state.dataRange->x >> state.dataRange->y;
         }},
        {"valuerange",
         [](State& state, std::stringstream& ss) {
             state.valueRange.emplace();
             ss >> state.valueRange->x >> state.valueRange->y;
         }},
        {"valueunit",
         [](State& state, std::stringstream& ss) {
             state.valueUnit = units::unit_from_string(ss.str());
         }},
        {"valuename", [](State& state, std::stringstream& ss) { state.valueName = ss.str(); }},
        {"swizzlemask", [](State& state, std::stringstream& ss) { ss >> state.swizzleMask; }},
        {"interpolation", [](State& state, std::stringstream& ss) { ss >> state.interpolation; }},
        {"wrapping", [](State& state, std::stringstream& ss) { ss >> state.wrapping; }},
        {"axisnames",
         [](State& state, std::stringstream& ss) {
             ss >> state.axes[0].name >> state.axes[1].name >> state.axes[2].name;
         }},
        {"axis1name", [](State& state, std::stringstream& ss) { ss >> state.axes[0].name; }},
        {"axis2name", [](State& state, std::stringstream& ss) { ss >> state.axes[1].name; }},
        {"axis3name", [](State& state, std::stringstream& ss) { ss >> state.axes[2].name; }},
        
        {"axisunits",
         [](State& state, std::stringstream& ss) {
             std::string unit;
             ss >> unit;
             state.axes[0].unit = units::unit_from_string(unit);
             ss >> unit;
             state.axes[1].unit = units::unit_from_string(unit);
             ss >> unit;
             state.axes[2].unit = units::unit_from_string(unit);
         }},
        {"axis1unit",
         [](State& state, std::stringstream& ss) {
             std::string unit;
             ss >> unit;
             state.axes[0].unit = units::unit_from_string(unit);
         }},

        {"axis2unit",
         [](State& state, std::stringstream& ss) {
             std::string unit;
             ss >> unit;
             state.axes[1].unit = units::unit_from_string(unit);
         }},

        {"axis3unit", [](State& state, std::stringstream& ss) {
            std::string unit;
            ss >> unit;
            state.axes[2].unit = units::unit_from_string(unit);
         }}};

    State state{};

    while (!f.eof()) {
        std::string textLine;
        getline(f, textLine);
        textLine = trim(std::move(textLine));

        if (textLine == "" || textLine[0] == '#' || textLine[0] == '/') continue;
        const auto [keyPart, valuePart] =
            util::splitByFirst(util::splitByFirst(textLine, '#').first, ':');
        if (valuePart.empty()) continue;

        const auto key = toLower(std::string{util::trim(keyPart)});
        const auto value = util::trim(valuePart);

        std::stringstream ss;
        ss << value;

        const auto it = parsers.find(key);
        if (it != parsers.end()) {
            it->second(state, ss);
            if (!ss) {
                throw DataReaderException(
                    IVW_CONTEXT, "Unable to parse key: '{}' with value: '{} in .dat file: '{}'",
                    key, value, filePath);
            }
        } else {
            state.metadata[key] = value;
        }
    };

    // Check if other dat files where specified, and then only consider them as a sequence
    auto volumes = std::make_shared<VolumeSequence>();

    if (!state.datFiles.empty()) {
        for (size_t t = 0; t < state.datFiles.size(); ++t) {
            auto datVolReader = std::make_unique<DatVolumeSequenceReader>();
            datVolReader->enableLogOutput_ = false;
            auto path = state.datFiles[t].is_absolute() ? state.datFiles[t]
                                                        : fileDirectory / state.datFiles[t];
            auto v = datVolReader->readData(path);

            std::copy(v->begin(), v->end(), std::back_inserter(*volumes));
        }
        if (enableLogOutput_) {
            LogInfo("Loaded multiple volumes: " << filePath
                                                << " volumes: " << state.datFiles.size());
        }

    } else {
        if (state.dimensions == size3_t(0)) {
            throw DataReaderException(
                IVW_CONTEXT, "Error: Unable to find \"Resolution\" tag in .dat file: {}", filePath);
        } else if (state.format == nullptr) {
            throw DataReaderException(
                IVW_CONTEXT, "Error: Unable to find \"Format\" tag in .dat file: {}", filePath);
        } else if (state.format->getId() == DataFormatId::NotSpecialized) {
            throw DataReaderException(
                IVW_CONTEXT,
                "Error: Invalid format string found: {} in {} \nThe valid formats are:\n"
                "FLOAT16, FLOAT32, FLOAT64, INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, "
                "UINT64, Vec2FLOAT16, Vec2FLOAT32, Vec2FLOAT64, Vec2INT8, Vec2INT16, "
                "Vec2INT32, Vec2INT64, Vec2UINT8, Vec2UINT16, Vec2UINT32, Vec2UINT64, "
                "Vec3FLOAT16, Vec3FLOAT32, Vec3FLOAT64, Vec3INT8, Vec3INT16, Vec3INT32, "
                "Vec3INT64, Vec3UINT8, Vec3UINT16, Vec3UINT32, Vec3UINT64, Vec4FLOAT16, "
                "Vec4FLOAT32, Vec4FLOAT64, Vec4INT8, Vec4INT16, Vec4INT32, Vec4INT64, "
                "Vec4UINT8, Vec4UINT16, Vec4UINT32, Vec4UINT64",
                state.formatFlag, filePath);

        } else if (state.rawFile == "") {
            throw DataReaderException(
                IVW_CONTEXT, "Error: Unable to find \"ObjectFilename\" tag in .dat file: {}",
                filePath);
        }

        if (state.spacing) {
            state.basis[0][0] = state.dimensions.x * state.spacing->x;
            state.basis[1][1] = state.dimensions.y * state.spacing->y;
            state.basis[2][2] = state.dimensions.z * state.spacing->z;
        }

        if (state.a && state.b && state.c) {
            state.basis[0][0] = state.a->x;
            state.basis[1][0] = state.a->y;
            state.basis[2][0] = state.a->z;
            state.basis[0][1] = state.b->x;
            state.basis[1][1] = state.b->y;
            state.basis[2][1] = state.b->z;
            state.basis[0][2] = state.c->x;
            state.basis[1][2] = state.c->y;
            state.basis[2][2] = state.c->z;
        }

        // If not specified, center the data around origo.
        if (!state.offset) {
            state.offset = -0.5f * (state.basis[0] + state.basis[1] + state.basis[2]);
        }

        auto volume = std::make_shared<Volume>(state.dimensions, state.format, state.swizzleMask,
                                               state.interpolation, state.wrapping);
        volume->setBasis(state.basis);
        volume->setOffset(*state.offset);
        volume->setWorldMatrix(state.wtm);

        if (state.dataRange) {
            volume->dataMap_.dataRange = *state.dataRange;
        }
        if (state.valueRange) {
            volume->dataMap_.valueRange = *state.valueRange;
        } else {
            volume->dataMap_.valueRange = volume->dataMap_.dataRange;
        }

        volume->dataMap_.valueAxis.unit = state.valueUnit;
        volume->dataMap_.valueAxis.name = state.valueName;
        volume->axes = state.axes;

        const auto bytes = glm::compMul(state.dimensions) * (state.format->getSize());

        for (auto elem : state.metadata) {
            volume->setMetaData<StringMetaData>(elem.first, elem.second);
        }

        for (size_t t = 0; t < state.sequences; ++t) {
            if (t == 0) {
                volumes->push_back(std::move(volume));
            } else {
                volumes->push_back(std::shared_ptr<Volume>(volumes->front()->clone()));
            }
            auto diskRepr = std::make_shared<VolumeDisk>(filePath, state.dimensions, state.format,
                                                         state.swizzleMask, state.interpolation,
                                                         state.wrapping);
            const auto filePos = t * bytes + state.byteOffset;

            auto loader = std::make_unique<RawVolumeRAMLoader>(fileDirectory / state.rawFile,
                                                               filePos, state.littleEndian);
            diskRepr->setLoader(loader.release());
            volumes->back()->addRepresentation(diskRepr);
            // Compute data range if not specified
            if (t == 0 && !state.dataRange) {
                // Use min/max value in data as data range if none is given
                // Only consider first time step since it can be time consuming
                // to compute for all time steps
                auto minmax = util::volumeMinMax(volumes->front().get(), IgnoreSpecialValues::No);
                // minmax always have four components, unused components are set to zero.
                // Hence, only consider components used by the data format
                dvec2 computedRange(minmax.first[0], minmax.second[0]);
                for (size_t component = 1; component < state.format->getComponents(); ++component) {
                    computedRange = dvec2(glm::min(computedRange[0], minmax.first[component]),
                                          glm::max(computedRange[1], minmax.second[component]));
                }
                // Set value range
                volumes->front()->dataMap_.dataRange = computedRange;
                // Also set value range if not specified
                if (!state.valueRange) {
                    volumes->front()->dataMap_.valueRange = computedRange;
                }
                // Performance warning for larger volumes (rougly > 2MB)
                if (bytes < 128 * 128 * 128 || state.sequences > 1) {
                    LogWarn(
                        "Performance warning: Using min/max of data since DataRange was not "
                        "specified. Data range refer to the range of the data type, i.e. [0 4095] "
                        "for 12-bit unsigned integer data. It is important that the data range is "
                        "specified for data types with a large range (for example 32/64-bit float "
                        "and integer) since the data is often normalized to [0 1], when for "
                        "example performing color mapping, i.e. applying a transfer function. "
                        "\nValue range refer to the physical meaning of the value, i.e. Hounsfield "
                        "value range is from [-1000 3000]. Improve volume read performance by "
                        "adding for example: \n"
                        << "DataRange: " << computedRange[0] << " " << computedRange[1]
                        << "\nValueRange: " << computedRange[0] << " " << computedRange[1]
                        << "\nin file: " << filePath);
                }
                if (state.sequences > 1) {
                    LogWarn(
                        "Multiple volumes in file but we only computed DataRange of the first "
                        "volume sequence due to performance consideration. \nWe strongly recommend "
                        "setting the DataRange, for example to min(Volume sequence), max(Volume "
                        "sequence).");
                }
            }
        }

        if (enableLogOutput_) {
            const auto size = util::formatBytesToString(bytes * state.sequences);
            LogInfo("Loaded volume sequence: " << filePath << " size: " << size);
        }
    }
    return volumes;
}

}  // namespace inviwo
