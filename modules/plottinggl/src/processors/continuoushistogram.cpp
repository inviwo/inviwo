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

#include <modules/plottinggl/processors/continuoushistogram.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/exception.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/shader/standardshaders.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/openglutils.h>

#include <algorithm>
#include <ranges>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ContinuousHistogram::processorInfo_{
    "org.inviwo.ContinuousHistogram",  // Class identifier
    "Continuous Histogram",            // Display name
    "Volume Operation",                // Category
    CodeState::Experimental,           // Code state
    Tags::GL | Tag{"Histogram"},       // Tags
    R"(Create a continuous 2D histogram (Continuous Scatterplot) from two volumes.)"_unindentHelp,
};

const ProcessorInfo ContinuousHistogram::getProcessorInfo() const { return processorInfo_; }

ContinuousHistogram::ContinuousHistogram()
    : Processor{}
    , inport1_{"inport1", "Volume 1"_help}
    , inport2_{"inport2", "Volume 2"_help}
    , outport_{"outport", "Histogram Layer"_help}
    , histogramResolution_{"histogramResolution", "Histogram Resolution",
                           util::ordinalCount(1024, 16384)
                               .setMin(1)
                               .set("Resolution of the resulting 2D histogram."_help)}
    , scalingFactor_{"scalingFactor", "Scaling Factor", util::ordinalScale(1.0f, 100.0f)}
    , errorThreshold_{"errorThreshold", "Error Threshold",
                      util::ordinalLength(0.5f, 1.0f)
                          .set(
                              "Error threshold with respect to the resolution of the histogram"_help)}
    , channel1_{"channel1", "Volume 1 Channel", util::enumeratedOptions("Channel", 4), 0}
    , channel2_{"channel2", "Volume 2 Channel", util::enumeratedOptions("Channel", 4), 0}
    , scaling_{"scaling", "Scaling",
               OptionPropertyState<Scaling>{.options = {{"linear", "Linear", Scaling::Linear},
                                                        {"log", "Logarithmic", Scaling::Log}}}
                   .setSelectedValue(Scaling::Linear)}
    , dataRange_{"dataRange", "Data Range"}
    , shaders_{{{ShaderType::Vertex, std::string{"continuoushistogram.vert"}},
                {ShaderType::Fragment, std::string{"continuoushistogram.frag"}}},
               {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec2"},
                {BufferType::ScalarMetaAttrib, MeshShaderCache::Mandatory, "float"}},
               [&](Shader& shader) -> void {
                   shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
                   shader.build();
               }} {

    addPorts(inport1_, inport2_, outport_);
    addProperties(histogramResolution_, errorThreshold_, scalingFactor_, channel1_, channel2_,
                  scaling_, dataRange_);
}

void ContinuousHistogram::initializeResources() {
    for (auto& [state, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void ContinuousHistogram::configureShader(Shader& shader) { shader.build(); }

void ContinuousHistogram::process() {
    const LayerConfig newConfig{
        .dimensions = size2_t{static_cast<size_t>(histogramResolution_.get())},
        .format = DataFloat32::get(),
        .swizzleMask = swizzlemasks::luminance,
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 1.0},
    };
    if (config_ != newConfig) {
        cache_.clear();
        config_ = newConfig;
    }

    auto&& [fbo, layer] = [&]() -> decltype(auto) {
        auto unusedIt = std::ranges::find_if(
            cache_, [](const std::pair<FrameBufferObject, std::shared_ptr<Layer>>& item) {
                return item.second.use_count() == 1;
            });
        if (unusedIt != cache_.end()) {
            return *unusedIt;
        } else {
            auto& item = cache_.emplace_back(FrameBufferObject{}, std::make_shared<Layer>(config_));
            auto* layerGL = item.second->getEditableRepresentation<LayerGL>();
            utilgl::Activate activateFbo{&item.first};
            item.first.attachColorTexture(layerGL->getTexture().get(), 0);
            return item;
        }
    }();

    if (!mesh_ || util::any_of(util::ref<Property>(channel1_, channel2_), &Property::isModified) ||
        inport1_.isChanged() || inport2_.isChanged()) {
        mesh_ = createDensityMesh();
    }

    auto& shader = shaders_.getShader(*mesh_);

    utilgl::Activate activateShader{&shader};
    utilgl::setShaderUniforms(shader, *layer, "outportParameters");
    utilgl::setShaderUniforms(shader, scalingFactor_);

    TextureUnitContainer cont;
    // utilgl::bindAndSetUniforms(shader_, cont, *inport1_.getData(), "inport");

    const auto dim = layer->getDimensions();
    {
        utilgl::Activate activateFbo{&fbo};
        utilgl::ViewportState viewport{0, 0, static_cast<GLsizei>(dim.x),
                                       static_cast<GLsizei>(dim.y)};
        utilgl::clearCurrentTarget();
        utilgl::DepthMaskState depthMask{GL_FALSE};
        utilgl::GlBoolState depthTest{GL_DEPTH_TEST, false};
        // utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE);
        utilgl::GlBoolState blend(GL_BLEND, true);
        utilgl::BlendModeEquationState blenEq(GL_ONE, GL_ONE, GL_FUNC_ADD);

        utilgl::setShaderUniforms(shader, *mesh_, "geometry");
        MeshDrawerGL::DrawObject drawer{mesh_->getRepresentation<MeshGL>(),
                                        mesh_->getDefaultMeshInfo()};
        drawer.draw();
    }
    outport_.setData(layer);
}

std::shared_ptr<Mesh> ContinuousHistogram::createDensityMesh() const {
    const auto histSize = size2_t{static_cast<size_t>(std::max(histogramResolution_.get(), 1))};

    auto volume1 = inport1_.getData();
    auto volume2 = inport2_.getData();

    const auto c1 = channel1_.getSelectedIndex();
    const auto c2 = channel2_.getSelectedIndex();

    if (volume1->getDimensions() != volume2->getDimensions()) {
        throw Exception(IVW_CONTEXT, "Volume dimensions must match, got {} and {}",
                        volume1->getDimensions(), volume2->getDimensions());
    }

    if (c1 >= volume1->getDataFormat()->getComponents()) {
        throw Exception(IVW_CONTEXT, "Channel 1 is greater than the available channels {} >= {}",
                        c1 + 1, volume1->getDataFormat()->getComponents());
    }
    if (c2 >= volume2->getDataFormat()->getComponents()) {
        throw Exception(IVW_CONTEXT, "Channel 2 is greater than the available channels {} >= {}",
                        c2 + 1, volume2->getDataFormat()->getComponents());
    }

    auto vrep1 = volume1->getRepresentation<VolumeRAM>();
    auto vrep2 = volume2->getRepresentation<VolumeRAM>();

    auto& map1 = volume1->dataMap;
    auto& map2 = volume2->dataMap;

    const util::IndexMapper3D volumeIm{vrep1->getDimensions()};

    auto mesh = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);

    dispatching::doubleDispatch<void, dispatching::filter::All, dispatching::filter::All>(
        vrep1->getDataFormatId(), vrep2->getDataFormatId(), [&]<typename T1, typename T2>() {
            auto src1 = static_cast<const VolumeRAMPrecision<T1>*>(vrep1)->getView();
            auto src2 = static_cast<const VolumeRAMPrecision<T2>*>(vrep2)->getView();

            const auto dims = ivec3{vrep1->getDimensions()};
            util::IndexMapper<3> indexMapper{dims};

            auto getVoxelValue = [&](auto& view, size_t index, size_t channel) -> double {
                return static_cast<double>(util::glmcomp(view[index], channel));
            };

            const double volumeVoxel = std::abs(glm::determinant(volume1->getBasis()));
            const std::array<ivec3, 7> offsets = {
                ivec3{1, 0, 0}, ivec3{0, 1, 0}, ivec3{1, 1, 0}, ivec3{0, 0, 1},
                ivec3{1, 0, 1}, ivec3{0, 1, 1}, ivec3{1, 1, 1},
            };
            auto projectedRectangle = [&](const ivec3& pos) -> std::tuple<dvec2, dvec2> {
                dvec2 min{getVoxelValue(src1, volumeIm(pos), c1),
                          getVoxelValue(src2, volumeIm(pos), c2)};
                dvec2 max{min};
                for (const ivec3& offset : offsets) {
                    const size_t index = volumeIm(pos + offset);
                    const dvec2 v{getVoxelValue(src1, index, c1), getVoxelValue(src2, index, c2)};
                    min = glm::min(min, v);
                    max = glm::max(max, v);
                }
                const auto p1 =
                    dvec2{map1.mapFromDataToNormalized(min.x), map2.mapFromDataToNormalized(min.y)};
                const auto p2 =
                    dvec2{map1.mapFromDataToNormalized(max.x), map2.mapFromDataToNormalized(max.y)};
                return {p1, p2};
            };

            std::vector<vec2> positions;
            std::vector<float> densities;
            auto addVertices = [&](const auto& min, const auto& max, float density) {
                positions.push_back(vec2{min});
                positions.push_back(vec2{max.x, min.y});
                positions.push_back(vec2{min.x, max.y});
                positions.push_back(vec2{min.x, max.y});
                positions.push_back(vec2{max.x, min.y});
                positions.push_back(vec2{max.x, max.y});
                for (size_t i = 0; i < 6; ++i) {
                    densities.push_back(density);
                }
            };

            dvec2 min{std::numeric_limits<double>::max()};
            dvec2 max{std::numeric_limits<double>::lowest()};
            ivec3 pos;
            for (pos.z = 0; pos.z < dims.z - 1; ++pos.z) {
                for (pos.y = 0; pos.y < dims.y - 1; ++pos.y) {
                    for (pos.x = 0; pos.x < dims.x - 1; ++pos.x) {
                        const auto&& [p1, p2] = projectedRectangle(pos);
                        const auto extent = dvec2{p2.x - p1.x, p2.y - p1.y};

                        // TODO: subdivision if rectangle extend > threshold

                        // FIXME: how to handle zero-sized rects? Set to size = 1?
                        if (extent.x > 1.0e-5 && extent.y > 1.0e-5) {
                            const double density = volumeVoxel / (extent.x * extent.y);
                            addVertices(p1, p2, static_cast<float>(density));

                            min = glm::min(min, p1);
                            max = glm::max(max, p2);
                        }
                    }
                }
            }

            LogInfo("Triangles min/max: " << min << ", " << max);
            const auto [minSigma, maxSigma] = std::ranges::minmax_element(densities);
            LogInfo("Density min/max: " << *minSigma << ", " << *maxSigma);
            std::transform(densities.begin(), densities.end(), densities.begin(),
                           [max = *maxSigma](auto v) { return v / max; });

            mesh->addBuffer(BufferType::PositionAttrib, util::makeBuffer(std::move(positions)));
            mesh->addBuffer(BufferType::ScalarMetaAttrib, util::makeBuffer(std::move(densities)));
        });

    // map [0,1] coords to [-1,1]
    mat4 m = glm::scale(vec3{2.0f});
    m[3] = vec4{-1.0f, -1.0f, 0.0, 1.0f};
    mesh->setModelMatrix(m);

    return mesh;
}

}  // namespace inviwo
