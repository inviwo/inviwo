/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Buffe...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/ports/datainport.h>                               // for DataInport
#include <inviwo/core/ports/dataoutport.h>                              // for DataOutport
#include <inviwo/core/ports/inportiterable.h>                           // for InportIterable<>:...
#include <inviwo/core/ports/outportiterable.h>                          // for OutportIterable
#include <inviwo/core/processors/poolprocessor.h>                       // for Progress, Option
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                   // for CompositeProperty
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatVec4Property
#include <inviwo/core/properties/property.h>                            // for Property
#include <inviwo/core/properties/propertyowner.h>                       // for PropertyOwner
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/valuewrapper.h>                        // for PropertySerializa...
#include <inviwo/core/util/formats.h>                                   // for DataFormat, DataF...
#include <inviwo/core/util/glmvec.h>                                    // for vec4, dvec2, uvec3
#include <inviwo/core/util/rendercontext.h>                             // for RenderContext
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/core/util/zip.h>                                       // for zipIterator, zipper
#include <modules/base/algorithm/volume/marchingcubes.h>                // for marchingcubes
#include <modules/base/algorithm/volume/marchingcubesopt.h>             // for marchingCubesOpt
#include <modules/base/algorithm/volume/marchingtetrahedron.h>          // for marchingtetrahedron

#include <algorithm>      // for max, min
#include <cstddef>        // for size_t
#include <iterator>       // for distance
#include <limits>         // for numeric_limits<>:...
#include <numeric>        // for accumulate
#include <tuple>          // for tuple_element<>::...
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair, make_pair

#include <flags/flags.h>  // for operator|
#include <fmt/core.h>     // for format, format_to
#include <glm/fwd.hpp>    // for uvec3
#include <glm/vec2.hpp>   // for vec<>::(anonymous)
#include <glm/vec4.hpp>   // for operator/

namespace inviwo {

const ProcessorInfo SurfaceExtraction::processorInfo_{
    "org.inviwo.SurfaceExtraction",  // Class identifier
    "Surface Extraction",            // Display name
    "Mesh Creation",                 // Category
    CodeState::Experimental,         // Code state
    Tags::CPU,                       // Tags
};
const ProcessorInfo& SurfaceExtraction::getProcessorInfo() const { return processorInfo_; }

SurfaceExtraction::SurfaceExtraction()
    : PoolProcessor(pool::Option::KeepOldResults | pool::Option::DelayDispatch)
    , volume_("volume")
    , outport_("mesh")
    , extraOutport_("also_mesh")
    , method_("method", "Method",
              {{"marchingtetrahedron", "Marching Tetrahedron", Method::MarchingTetrahedron},
               {"marchingcubes", "Marching Cubes", Method::MarchingCubes},
               {"marchingCubesOpt", "Marching Cubes Optimized", Method::MarchingCubesOpt}},
              2)
    , isoValue_("iso", "ISO Value", 0.5f, 0.0f, 1.0f, 0.01f)
    , invertIso_("invert", "Invert ISO", false)
    , encloseSurface_("enclose", "Enclose Surface", true)
    , colors_("meshColors", "Mesh Colors") {

    addPort(volume_);
    addPort(outport_);
    addPort(extraOutport_);

    addProperty(method_);
    addProperty(isoValue_);
    addProperty(invertIso_);
    addProperty(encloseSurface_);
    addProperty(colors_);

    volume_.onChange([this]() {
        updateColors();
        if (volume_.hasData()) {
            auto minmax = std::make_pair(std::numeric_limits<double>::max(),
                                         std::numeric_limits<double>::lowest());
            minmax = std::accumulate(volume_.begin(), volume_.end(), minmax,
                                     [](decltype(minmax) mm, std::shared_ptr<const Volume> v) {
                                         return std::make_pair(
                                             std::min(mm.first, v->dataMap.dataRange.x),
                                             std::max(mm.second, v->dataMap.dataRange.y));
                                     });

            isoValue_.setMinValue(static_cast<float>(minmax.first));
            isoValue_.setMaxValue(static_cast<float>(minmax.second));
        }
    });
}

SurfaceExtraction::~SurfaceExtraction() = default;

void SurfaceExtraction::process() {

    const auto computeSurface = [this](vec4 color, std::shared_ptr<const Volume> vol) {
        return [vol, color, method = method_.get(), iso = isoValue_.get(),
                invert = invertIso_.get(),
                enclose = encloseSurface_.get()](pool::Progress progress) -> std::shared_ptr<Mesh> {
            RenderContext::getPtr()->activateLocalRenderContext();

            switch (method) {
                case Method::MarchingCubes:
                    return util::marchingcubes(vol, iso, color, invert, enclose, progress);
                case Method::MarchingCubesOpt:
                    return util::marchingCubesOpt(vol, iso, color, invert, enclose, progress);
                case Method::MarchingTetrahedron:
                default:
                    return util::marchingtetrahedron(vol, iso, color, invert, enclose, progress);
            }
        };
    };

    const auto changeColor = [](vec4 color, std::shared_ptr<const Mesh> oldmesh) {
        return [oldmesh, color](pool::Progress) -> std::shared_ptr<Mesh> {
            RenderContext::getPtr()->activateLocalRenderContext();

            auto mesh = std::make_shared<Mesh>(oldmesh->getDefaultMeshInfo());

            mesh->setModelMatrix(oldmesh->getModelMatrix());
            mesh->setWorldMatrix(oldmesh->getWorldMatrix());
            mesh->copyMetaDataFrom(*oldmesh);

            // We can share the buffers here since we won't ever change them in this processor
            // and this is the only place with a non-const versions.
            for (const auto& [info, buff] : oldmesh->getIndexBuffers()) {
                mesh->addIndices(info, buff);
            }

            for (const auto& [info, buff] : oldmesh->getBuffers()) {
                if (info.type == BufferType::ColorAttrib &&
                    buff->getDataFormat()->getId() == DataFormat<vec4>::id()) {
                    const auto newColors = std::make_shared<BufferRAMPrecision<vec4>>(
                        std::vector<vec4>(buff->getSize(), color));
                    mesh->addBuffer(info, std::make_shared<Buffer<vec4>>(newColors));
                } else {
                    mesh->addBuffer(info, buff);
                }
            }

            return mesh;
        };
    };

    const auto size = static_cast<size_t>(std::distance(volume_.begin(), volume_.end()));
    if (colors_.size() < size) updateColors();

    const bool stateChange = method_.isModified() || isoValue_.isModified() ||
                             invertIso_.isModified() || encloseSurface_.isModified();

    if (stateChange || size != meshes_.size()) {  // Need to recompute all...
        std::vector<decltype(computeSurface(vec4{}, std::shared_ptr<const Volume>{}))> jobs;
        for (auto [i, vol] : util::enumerate(volume_)) {
            jobs.push_back(computeSurface(getColor(i), vol));
        }
        dispatchMany(jobs, [this](std::vector<std::shared_ptr<Mesh>> result) {
            meshes_ = result;
            outport_.setData(std::make_shared<std::vector<std::shared_ptr<Mesh>>>(meshes_));
            extraOutport_.setData(std::make_shared<DataSequence<Mesh>>(meshes_));
            newResults();
        });
    } else {  // Only update the modified ones
        std::vector<std::function<std::shared_ptr<Mesh>(pool::Progress progress)>> jobs;
        std::vector<size_t> inds;
        for (auto [i, item] : util::enumerate(volume_.changedAndData())) {
            const auto portChanged = item.first;
            const auto data = item.second;

            if (portChanged) {
                jobs.push_back(computeSurface(getColor(i), data));
                inds.push_back(i);
            } else if (colors_[i]->isModified()) {
                jobs.push_back(changeColor(getColor(i), meshes_[i]));
                inds.push_back(i);
            }
        }
        if (!jobs.empty()) {
            dispatchMany(jobs, [this, inds](std::vector<std::shared_ptr<Mesh>> results) {
                for (auto [i, result] : util::zip(inds, results)) {
                    meshes_[i] = result;
                }
                outport_.setData(std::make_shared<std::vector<std::shared_ptr<Mesh>>>(meshes_));
                extraOutport_.setData(std::make_shared<DataSequence<Mesh>>(meshes_));
                newResults();
            });
        }
    }
}

void SurfaceExtraction::updateColors() {
    const static vec4 defaultColor[11] = {vec4(1.0f),
                                          vec4(0x1f, 0x77, 0xb4, 255) / vec4(255),
                                          vec4(0xff, 0x7f, 0x0e, 255) / vec4(255),
                                          vec4(0x2c, 0xa0, 0x2c, 255) / vec4(255),
                                          vec4(0xd6, 0x27, 0x28, 255) / vec4(255),
                                          vec4(0x94, 0x67, 0xbd, 255) / vec4(255),
                                          vec4(0x8c, 0x56, 0x4b, 255) / vec4(255),
                                          vec4(0xe3, 0x77, 0xc2, 255) / vec4(255),
                                          vec4(0x7f, 0x7f, 0x7f, 255) / vec4(255),
                                          vec4(0xbc, 0xbd, 0x22, 255) / vec4(255),
                                          vec4(0x17, 0xbe, 0xcf, 255) / vec4(255)};

    size_t count = 0;
    for ([[maybe_unused]] auto data : volume_) {
        count++;
        if (colors_.size() < count) {
            auto prop = new FloatVec4Property(fmt::format("color{}", count - 1),
                                              fmt::format("Color for Volume {}", count),
                                              defaultColor[(count - 1) % 11]);
            prop->setCurrentStateAsDefault();
            prop->setSemantics(PropertySemantics::Color);
            prop->setSerializationMode(PropertySerializationMode::All);
            colors_.addProperty(prop);
        }
        colors_[count - 1]->setVisible(true);
    }

    for (size_t i = count; i < colors_.size(); i++) {
        colors_[i]->setVisible(false);
    }
}

vec4 SurfaceExtraction::getColor(size_t i) const {
    return static_cast<const FloatVec4Property*>(colors_[i])->get();
}

}  // namespace inviwo
