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
#include <inviwo/core/util/volumesampler.h>
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
    : PoolProcessor{pool::Option::DelayDispatch}
    , inport1_{"inport1", "Volume 1"_help}
    , inport2_{"inport2", "Volume 2"_help}
    , outport_{"outport", "Histogram Layer"_help}
    , histogramResolution_{"histogramResolution", "Histogram Resolution",
                           util::ordinalCount(1024, 16384)
                               .setMin(1)
                               .set("Resolution of the resulting 2D histogram."_help)}
    , scalingFactor_{"scalingFactor", "Scaling Factor", util::ordinalScale(1.0f, 100.0f)}
    , enableSubdivisions_{"enableSubdivisions", "Subdivisions", false}
    , errorThreshold_{"errorThreshold", "Error Threshold",
                      util::ordinalLength(1.0, 1.0).set(
                          "Error threshold with respect to the resolution of the histogram"_help)}
    , channel1_{"channel1", "Volume 1 Channel", util::enumeratedOptions("Channel", 4), 0}
    , channel2_{"channel2", "Volume 2 Channel", util::enumeratedOptions("Channel", 4), 0}
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
    addProperties(histogramResolution_, scalingFactor_, enableSubdivisions_, errorThreshold_,
                  channel1_, channel2_, dataRange_);
}

void ContinuousHistogram::initializeResources() {
    for (auto& [state, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void ContinuousHistogram::configureShader(Shader& shader) { shader.build(); }

namespace detail {

double getVoxelValue(auto& view, size_t index, size_t channel) {
    return static_cast<double>(util::glmcomp(view[index], channel));
}

double getInterpolatedValue(const VolumeSampler<dvec4>& sampler, const vec3& pos, size_t channel) {
    const auto sample = sampler.sample(pos);
    return util::glmcomp(sample, channel);
}

constexpr std::array<ivec3, 8> offsets = {
    ivec3{0, 0, 0}, ivec3{1, 0, 0}, ivec3{0, 1, 0}, ivec3{1, 1, 0},
    ivec3{0, 0, 1}, ivec3{1, 0, 1}, ivec3{0, 1, 1}, ivec3{1, 1, 1},
};

constexpr std::array<dvec3, 8> offsetsD = {
    dvec3{0, 0, 0}, dvec3{1, 0, 0}, dvec3{0, 1, 0}, dvec3{1, 1, 0},
    dvec3{0, 0, 1}, dvec3{1, 0, 1}, dvec3{0, 1, 1}, dvec3{1, 1, 1},
};

class Interpolator {
public:
    // explicit Interpolator(std::array<double, 8> values) : values_{std::move(values)} {};
    explicit Interpolator(std::array<double, 8>&& values) : values_{values} {}
    Interpolator(const Interpolator&) = default;
    Interpolator(Interpolator&&) = default;

    Interpolator& operator=(const Interpolator&) = default;
    Interpolator& operator=(Interpolator&&) = default;

    double operator()(const dvec3& interpolant) const {
        auto linear = [](double a, double b, double t) { return a * (1.0 - t) + b * t; };

        const double x1 = linear(values_[0], values_[1], interpolant.x);
        const double x2 = linear(values_[2], values_[3], interpolant.x);
        const double x3 = linear(values_[4], values_[5], interpolant.x);
        const double x4 = linear(values_[6], values_[7], interpolant.x);

        const double y1 = linear(x1, x2, interpolant.y);
        const double y2 = linear(x3, x4, interpolant.y);

        return linear(y1, y2, interpolant.z);
    }

private:
    std::array<double, 8> values_;
};

auto axisAlignedBoundingRect(std::ranges::input_range auto&& x, std::ranges::input_range auto&& y)
    -> std::tuple<dvec2, dvec2> {
    auto minmaxX = std::ranges::minmax_element(x);
    auto minmaxY = std::ranges::minmax_element(y);
    return {dvec2{*minmaxX.min, *minmaxY.min}, dvec2{*minmaxX.max, *minmaxY.max}};
};

template <typename CreateTriangleCallback>
void splitCell(const Interpolator& scalars1, const Interpolator& scalars2, dvec3 interpolant,
               double threshold, CreateTriangleCallback callback, int subdivision) {
    const double currentSize = pow(2.0, -subdivision);

    const auto&& [p1, p2] = axisAlignedBoundingRect(
        std::array<double, 8>{
            scalars1(interpolant + offsetsD[0] * currentSize),
            scalars1(interpolant + offsetsD[1] * currentSize),
            scalars1(interpolant + offsetsD[2] * currentSize),
            scalars1(interpolant + offsetsD[3] * currentSize),
            scalars1(interpolant + offsetsD[4] * currentSize),
            scalars1(interpolant + offsetsD[5] * currentSize),
            scalars1(interpolant + offsetsD[6] * currentSize),
            scalars1(interpolant + offsetsD[7] * currentSize),
        },
        std::array<double, 8>{
            scalars2(interpolant + offsetsD[0] * currentSize),
            scalars2(interpolant + offsetsD[1] * currentSize),
            scalars2(interpolant + offsetsD[2] * currentSize),
            scalars2(interpolant + offsetsD[3] * currentSize),
            scalars2(interpolant + offsetsD[4] * currentSize),
            scalars2(interpolant + offsetsD[5] * currentSize),
            scalars2(interpolant + offsetsD[6] * currentSize),
            scalars2(interpolant + offsetsD[7] * currentSize),
        });
    const auto extent = dvec2{p2.x - p1.x, p2.y - p1.y};

    constexpr int maxSubdiv = 16;
    if (glm::compMax(extent) > threshold && subdivision < maxSubdiv) {
        const double nextSize = currentSize * 0.5;
        for (auto& offset : offsetsD) {
            splitCell(scalars1, scalars2, interpolant + offset * nextSize, threshold, callback,
                      subdivision + 1);
        }
    } else {
        callback(p1, p2, subdivision);
    }
};

std::shared_ptr<Mesh> createDensityMesh(pool::Stop stop, pool::Progress progress,
                                        const size2_t& histogramSize,
                                        std::shared_ptr<const Volume> volume1, size_t channel1,
                                        std::shared_ptr<const Volume> volume2, size_t channel2,
                                        bool enableSubdivs, double errorThreshold) {
    if (stop) return nullptr;

    const auto histPixelSize = 1.0 / dvec2{histogramSize};

    auto vrep1 = volume1->getRepresentation<VolumeRAM>();
    auto vrep2 = volume2->getRepresentation<VolumeRAM>();

    auto& map1 = volume1->dataMap;
    auto& map2 = volume2->dataMap;

    const util::IndexMapper3D volumeIm{vrep1->getDimensions()};

    std::shared_ptr<Mesh> mesh =
        dispatching::doubleDispatch<std::shared_ptr<Mesh>, dispatching::filter::All,
                                    dispatching::filter::All>(
            vrep1->getDataFormatId(), vrep2->getDataFormatId(),
            [&]<typename T1, typename T2>() -> std::shared_ptr<Mesh> {
                auto src1 = static_cast<const VolumeRAMPrecision<T1>*>(vrep1)->getView();
                auto src2 = static_cast<const VolumeRAMPrecision<T2>*>(vrep2)->getView();

                const auto dims = ivec3{vrep1->getDimensions()};
                const double volumeVoxel = std::abs(glm::determinant(volume1->getBasis()));

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

                if (enableSubdivs) {
                    auto triangleCallback = [&](dvec2 p1, dvec2 p2, int subdivision) {
                        auto extent = dvec2{p2.x - p1.x, p2.y - p1.y};

                        // FIXME: how to handle zero-sized rects? Set to size = 1?
                        extent = glm::max(extent, histPixelSize);

                        const double density =
                            volumeVoxel * pow(2.0, -3 * subdivision) / (extent.x * extent.y);

                        if (extent.x < histPixelSize.x) {
                            double center = (p1.x + p2.x) * 0.5;
                            p1.x = center + histPixelSize.x * 0.5;
                            p2.x = center + histPixelSize.x * 0.5;
                        }
                        if (extent.y < histPixelSize.y) {
                            double center = (p1.y + p2.y) * 0.5;
                            p1.y = center + histPixelSize.y * 0.5;
                            p2.y = center + histPixelSize.y * 0.5;
                        }
                        addVertices(p1, p2, static_cast<float>(density));

                        min = glm::min(min, p1);
                        max = glm::max(max, p2);
                    };

                    util::IndexMapper<3> indexMapper{dims};
                    std::array<double, 8> scalars1{};
                    std::array<double, 8> scalars2{};
                    ivec3 pos{};
                    for (pos.z = 0; pos.z < dims.z - 1; ++pos.z) {
                        if (pos.z & 8) {
                            if (stop) return nullptr;
                            progress(pos.z, dims.z - 1);
                        }
                        for (pos.y = 0; pos.y < dims.y - 1; ++pos.y) {
                            for (pos.x = 0; pos.x < dims.x - 1; ++pos.x) {
                                for (size_t i = 0; i < 8; ++i) {
                                    const size_t index = volumeIm(pos + detail::offsets[i]);
                                    scalars1[i] = map1.mapFromDataToNormalized(
                                        detail::getVoxelValue(src1, index, channel1));
                                    scalars2[i] = map2.mapFromDataToNormalized(
                                        detail::getVoxelValue(src2, index, channel2));
                                }

                                detail::splitCell(detail::Interpolator(std::move(scalars1)),
                                                  detail::Interpolator(std::move(scalars2)),
                                                  dvec3{0}, errorThreshold, triangleCallback, 0);
                            }
                        }
                    }
                } else {

                    auto projectedRectangle = [&, indexMapper = util::IndexMapper<3>{dims}](
                                                  const ivec3& pos) -> std::tuple<dvec2, dvec2> {
                        dvec2 min{detail::getVoxelValue(src1, volumeIm(pos), channel1),
                                  detail::getVoxelValue(src2, volumeIm(pos), channel2)};
                        dvec2 max{min};
                        for (const ivec3& offset : detail::offsets | std::ranges::views::drop(1)) {
                            const size_t index = volumeIm(pos + offset);
                            const dvec2 v{detail::getVoxelValue(src1, index, channel1),
                                          detail::getVoxelValue(src2, index, channel2)};
                            min = glm::min(min, v);
                            max = glm::max(max, v);
                        }
                        return {dvec2{map1.mapFromDataToNormalized(min.x),
                                      map2.mapFromDataToNormalized(min.y)},
                                dvec2{map1.mapFromDataToNormalized(max.x),
                                      map2.mapFromDataToNormalized(max.y)}};
                    };

                    ivec3 pos{};
                    for (pos.z = 0; pos.z < dims.z - 1; ++pos.z) {
                        if (pos.z & 8) {
                            if (stop) return nullptr;
                            progress(pos.z, dims.z - 1);
                        }
                        for (pos.y = 0; pos.y < dims.y - 1; ++pos.y) {
                            for (pos.x = 0; pos.x < dims.x - 1; ++pos.x) {
                                const auto&& [p1, p2] = projectedRectangle(pos);
                                auto extent = dvec2{p2.x - p1.x, p2.y - p1.y};

                                // FIXME: how to handle zero-sized rects? Set to size = 1?
                                extent = glm::max(extent, histPixelSize);

                                const double density = volumeVoxel / (extent.x * extent.y);
                                addVertices(p1, p2, static_cast<float>(density));

                                min = glm::min(min, p1);
                                max = glm::max(max, p2);
                            }
                        }
                    }
                }

                auto mesh = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);
                // map [0,1] coords to [-1,1]
                mat4 m = glm::scale(vec3{2.0f});
                m[3] = vec4{-1.0f, -1.0f, 0.0, 1.0f};
                mesh->setModelMatrix(m);

                if (!densities.empty()) {
                    LogInfoCustom("createDensityMesh", "Triangles min/max: " << min << ", " << max);
                    const auto [minSigma, maxSigma] = std::ranges::minmax_element(densities);
                    LogInfoCustom("createDensityMesh",
                                  "Density min/max: " << *minSigma << ", " << *maxSigma);
                    std::transform(densities.begin(), densities.end(), densities.begin(),
                                   [max = *maxSigma](auto v) { return v / max; });

                    LogInfoCustom("createDensityMesh", "Num triangles: " << positions.size() / 3);

                    mesh->addBuffer(BufferType::PositionAttrib,
                                    util::makeBuffer(std::move(positions)));
                    mesh->addBuffer(BufferType::ScalarMetaAttrib,
                                    util::makeBuffer(std::move(densities)));
                }
                return mesh;
            });

    return mesh;
}

}  // namespace detail

void ContinuousHistogram::process() {
    auto volume1 = inport1_.getData();
    auto volume2 = inport2_.getData();

    if (volume1->getDimensions() != volume2->getDimensions()) {
        throw Exception(IVW_CONTEXT, "Volume dimensions must match, got {} and {}",
                        volume1->getDimensions(), volume2->getDimensions());
    }

    if (channel1_.getSelectedIndex() >= volume1->getDataFormat()->getComponents()) {
        throw Exception(IVW_CONTEXT, "Channel 1 is greater than the available channels {} >= {}",
                        channel1_.getSelectedIndex() + 1,
                        volume1->getDataFormat()->getComponents());
    }
    if (channel2_.getSelectedIndex() >= volume2->getDataFormat()->getComponents()) {
        throw Exception(IVW_CONTEXT, "Channel 2 is greater than the available channels {} >= {}",
                        channel1_.getSelectedIndex() + 1,
                        volume2->getDataFormat()->getComponents());
    }

    const size2_t histogramSize{static_cast<size_t>(std::max(histogramResolution_.get(), 1))};

    if (!mesh_ || inport1_.isChanged() || inport2_.isChanged() ||
        util::any_of(
            util::ref<Property>(channel1_, channel2_, enableSubdivisions_, errorThreshold_),
            &Property::isModified)) {

        const auto calc =
            [histogramSize, volume1 = inport1_.getData(), channel1 = channel1_.getSelectedIndex(),
             volume2 = inport2_.getData(), channel2 = channel2_.getSelectedIndex(),
             subdivs = enableSubdivisions_.get(), threshold = errorThreshold_.get()](
                pool::Stop stop, pool::Progress progress) -> std::shared_ptr<const Mesh> {
            return detail::createDensityMesh(stop, progress, histogramSize, volume1, channel1,
                                             volume2, channel2, subdivs, threshold);
        };

        dispatchOne(calc, [this](std::shared_ptr<const Mesh> result) {
            mesh_ = result;
            auto layer = renderDensityMesh(mesh_);
            outport_.setData(layer);
            newResults();
        });
    }

    auto layer = renderDensityMesh(mesh_);
    outport_.setData(layer);
}

std::shared_ptr<Layer> ContinuousHistogram::renderDensityMesh(std::shared_ptr<const Mesh> mesh) {
    if (!mesh) return nullptr;

    const LayerConfig newConfig{
        .dimensions = size2_t{static_cast<size_t>(std::max(histogramResolution_.get(), 1))},
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

    auto& shader = shaders_.getShader(*mesh);

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

        utilgl::setShaderUniforms(shader, *mesh, "geometry");
        MeshDrawerGL::DrawObject drawer{mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo()};
        drawer.draw();
    }
    return layer;
}

}  // namespace inviwo
