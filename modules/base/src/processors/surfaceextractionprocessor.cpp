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

#include <modules/base/processors/surfaceextractionprocessor.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/colorbrewer.h>
#include <modules/base/algorithm/volume/marchingcubes.h>
#include <modules/base/algorithm/volume/marchingcubesopt.h>
#include <modules/base/algorithm/volume/marchingtetrahedron.h>

#include <flags/flags.h>
#include <fmt/base.h>

namespace inviwo {

const ProcessorInfo SurfaceExtraction::processorInfo_{
    "org.inviwo.SurfaceExtraction",     // Class identifier
    "Surface Extraction",               // Display name
    "Mesh Creation",                    // Category
    CodeState::Experimental,            // Code state
    Tags::CPU | Tag{"Marching Cubes"},  // Tags
    R"(Extracts isosurfaces for each of the given isovalues in the input volumes using the
    Marching Cubes algorithm. The resulting surfaces are provided as a mesh sequence, one mesh
    per volume.)"_unindentHelp,
};
const ProcessorInfo& SurfaceExtraction::getProcessorInfo() const { return processorInfo_; }

SurfaceExtraction::SurfaceExtraction()
    : PoolProcessor{pool::Option::KeepOldResults | pool::Option::DelayDispatch}
    , volume_{"volume", "Input volumes"_help}
    , outport_{"mesh", "Isosurface meshes"_help}
    , method_{"method",
              "Method",
              {{"marchingtetrahedron", "Marching Tetrahedron", Method::MarchingTetrahedron},
               {"marchingcubes", "Marching Cubes", Method::MarchingCubes},
               {"marchingCubesOpt", "Marching Cubes Optimized", Method::MarchingCubesOpt}},
              2}
    , coloring_{"coloring",
                "Coloring",
                "Determines the coloring of the resulting isosurfaces based on the colors of the "
                "isovalues, the indices of the input volumes, or a mix of both."_help,
                {{"isovalues", "Iso Value Colors", ColoringMode::IsoValues},
                 {"volumeindex", "Volume Index", ColoringMode::VolumeIndex},
                 {"mixed", "Mix Isovalues and Volume Indices", ColoringMode::Mixed}},
                0}
    , blendFactor_{"blendFactor",
                   "Blend Factor",
                   "Blending factor for mixing isovalue colors the volume index TF."_help,
                   0.5f,
                   {0.0f, ConstraintBehavior::Immutable},
                   {1.0f, ConstraintBehavior::Immutable},
                   0.001f}
    , isoValues_{"isovalues",
                 "Isovalues",
                 "Isovalues and corresponding colors for the contours"_help,
                 {{{.pos = 0.5, .color = vec4{1.0f}}}},
                 TFData{&volume_}}
    , volumeTF_{"volumeTF", "Volume Index TF",
                "Transfer function used for volume index coloring"_help,
                colorbrewer::getTransferFunction(colorbrewer::Category::Qualitative,
                                                 colorbrewer::Family::Dark2, 4, true)}

    , invertIso_{"invert", "Flip Surface Normals ", false}
    , encloseSurface_{"enclose", "Enclose Surface", true} {

    addPorts(volume_, outport_);

    addProperties(method_, isoValues_, volumeTF_, coloring_, blendFactor_, invertIso_,
                  encloseSurface_);
}

SurfaceExtraction::~SurfaceExtraction() = default;

namespace {

auto updateIsosurfaceColors(const std::vector<TFPrimitiveData>& isoValues,
                            std::shared_ptr<Mesh>& oldmesh) {
    return [oldmesh, isoValues](pool::Progress) -> std::shared_ptr<Mesh> {
        auto* surfaceIndexBuffer = oldmesh->getBuffer(BufferType::IntMetaAttrib);
        if (!surfaceIndexBuffer) {
            // cannot update isosurface colors without surface indices
            return oldmesh;
        }
        const auto* surfaceIndexRAM = dynamic_cast<const BufferRAMPrecision<int>*>(
            surfaceIndexBuffer->getRepresentation<BufferRAM>());
        if (!surfaceIndexRAM) {
            throw Exception{SourceContext{},
                            "Unexpected buffer format for surface indices: {}, expected int",
                            surfaceIndexBuffer->getDataFormat()->getString()};
        }
        const auto& surfaceIndices = surfaceIndexRAM->getDataContainer();

        // We can share the buffers here since we won't ever change them in this processor
        // and this is the only place with a non-const versions.
        auto mesh = std::make_shared<Mesh>(*oldmesh, noData);
        for (const auto& [info, buff] : oldmesh->getIndexBuffers()) {
            mesh->addIndices(info, buff);
        };
        for (const auto& [info, buff] : oldmesh->getBuffers()) {
            if (info.type == BufferType::ColorAttrib) {
                // update colors based on corresponding surface index
                std::vector<vec4> colors;
                colors.reserve(buff->getSize());
                for (const auto& index : surfaceIndices) {
                    colors.emplace_back(isoValues[index].color);
                }
                mesh->addBuffer(info, util::makeBuffer(std::move(colors)));
            } else {
                mesh->addBuffer(info, buff);
            }
        }
        return mesh;
    };
}

}  // namespace

void SurfaceExtraction::process() {
    const auto computeSurface = [this](const std::vector<TFPrimitiveData>& isoValues,
                                       std::shared_ptr<const Volume> vol) {
        return [vol, method = method_.get(), isoValues, isoValueMode = isoValues_.get().getMode(),
                invert = invertIso_.get(),
                enclose = encloseSurface_.get()](pool::Progress progress) -> std::shared_ptr<Mesh> {
            switch (method) {
                case Method::MarchingCubes:
                    return util::marchingcubes(vol, isoValues, isoValueMode, invert, enclose,
                                               progress);
                case Method::MarchingCubesOpt:
                    return util::marchingCubesOpt(vol, isoValues, isoValueMode, invert, enclose,
                                                  progress);
                case Method::MarchingTetrahedron:
                default:
                    return util::marchingtetrahedron(vol, isoValues, isoValueMode, invert, enclose,
                                                     progress);
            }
        };
    };

    const bool stateChange = volume_.isChanged() || method_.isModified() ||
                             isoValues_.isModified() || invertIso_.isModified() ||
                             encloseSurface_.isModified();

    std::vector<std::function<std::shared_ptr<Mesh>(pool::Progress progress)>> jobs;

    const float mix = [mode = coloring_.get(), blend = blendFactor_.get()]() {
        switch (mode) {
            using enum ColoringMode;
            default:
            case IsoValues:   return 0.0f;
            case VolumeIndex: return 1.0f;
            case Mixed:       return blend;
        }
    }();

    auto transformIsoValueColors = [this, mix, source = isoValues_.get().get()](size_t index,
                                                                                size_t maxCount) {
        std::vector<TFPrimitiveData> dest{source};
        if (coloring_.get() != ColoringMode::IsoValues) {
            for (auto& value : dest) {
                const vec4 volumeColor{volumeTF_.get().sample((static_cast<float>(index) + 0.5f) /
                                                              static_cast<float>(maxCount))};
                value.color = glm::mix(value.color, volumeColor, mix);
            }
        }
        return dest;
    };

    if (stateChange) {  // Need to recompute all...
        const auto volumeCount = static_cast<size_t>(std::distance(volume_.begin(), volume_.end()));

        for (auto [i, vol] : util::enumerate(volume_)) {
            jobs.emplace_back(computeSurface(transformIsoValueColors(i, volumeCount), vol));
        }
    } else if (coloring_.isModified() || (coloring_.get() != ColoringMode::IsoValues &&
                                          (blendFactor_.isModified() || volumeTF_.isModified()))) {
        for (auto&& [i, mesh] : util::enumerate(meshes_)) {
            jobs.emplace_back(
                updateIsosurfaceColors(transformIsoValueColors(i, meshes_.size()), mesh));
        }
    }
    dispatchMany(jobs, [this](std::vector<std::shared_ptr<Mesh>> result) {
        meshes_ = std::move(result);
        auto sequence = std::make_shared<DataSequence<Mesh>>(meshes_);
        outport_.setData(sequence);
        newResults();
    });
}

}  // namespace inviwo
