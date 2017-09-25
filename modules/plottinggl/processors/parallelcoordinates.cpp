/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/plottinggl/processors/parallelcoordinates.h>
#include <modules/plottinggl/plottingglmodule.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/util/imagesampler.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/io/datareaderfactory.h>

namespace inviwo {

namespace plot {

static const float handleW = 40;
static const float handleH = 20;

namespace {

template <typename T>
struct Axis : public ParallelCoordinates::AxisBase {
    using FloatType = std::conditional_t<std::is_same<float, T>::value, float, double>;
    using MinMaxPropType = MinMaxProperty<FloatType>;
    MinMaxPropType *range_;
    const std::vector<T> *dataVector_;

    Axis(size_t columnId, std::string name, BoolCompositeProperty *boolCompositeProperty,
         MinMaxPropType *range, BoolProperty *usePercentiles,
         std::shared_ptr<const BufferBase> buffer, const std::vector<T> *dataVector)
        : AxisBase(columnId, name, boolCompositeProperty, usePercentiles, buffer)
        , range_(range)
        , dataVector_(dataVector) {
        auto copy = *dataVector;
        std::sort(copy.begin(), copy.end());
        p0_ = copy.front();
        p25_ = copy[copy.size() * 0.25];
        p75_ = copy[copy.size() * 0.75];
        p100_ = copy.back();
    }
    virtual ~Axis() = default;

    virtual float getNormalized(float v) const override {
        if (range_->getRangeMax() == range_->getRangeMin()) {
            return 0.5f;
        }

        if (usePercentiles_ && usePercentiles_->get()) {
            if (v <= p0_) {
                return 0;
            }
            if (v >= p100_) {
                return 1;
            }
            float minV, maxV;
            float o, r;
            if (v < p25_) {
                minV = p0_;
                maxV = p25_;
                o = 0;
                r = 0.25f;
            } else if (v < p75_) {
                minV = p25_;
                maxV = p75_;
                o = 0.25;
                r = 0.5f;
            } else {
                minV = p75_;
                maxV = p100_;
                o = 0.75f;
                r = 0.25f;
            }

            float t = (v - minV) / (maxV - minV);
            return o + t * r;
        } else {
            return (v - range_->getRangeMin()) / (range_->getRangeMax() - range_->getRangeMin());
        }
    }
    virtual float getValue(float v) const override {
        if (usePercentiles_ && usePercentiles_->get()) {
            if (v <= 0)
                return p0_;
            else if (v >= 1)
                return p100_;
            else if (v < 0.25f) {
                v /= 0.25f;
                return p0_ + v * (p25_ - p0_);
            } else if (v < 0.75f) {
                v -= 0.25f;
                v /= 0.5f;
                return p25_ + v * (p75_ - p25_);
            } else {
                v -= 0.75f;
                v /= 0.25f;
                return p75_ + v * (p100_ - p75_);
            }
        } else {
            return v * (range_->getRangeMax() - range_->getRangeMin()) + range_->getRangeMin();
        }
    }
    virtual float at(size_t idx) const override { return (*dataVector_)[idx]; }

    virtual vec2 getRange() const override { return vec2(range_->get()); }

    virtual float getRangeMin() const override { return range_->getRangeMin(); }
    virtual float getRangeMax() const override { return range_->getRangeMax(); }

    virtual void updateBrushing(std::unordered_set<size_t> &brushed) override {
        auto range = range_->get();
        auto &vec = *dataVector_;
        for (size_t i = 0; i < vec.size(); i++) {
            if (vec[i] + std::numeric_limits<float>::epsilon() < range.x ||
                vec[i] - std::numeric_limits<float>::epsilon() > range.y) {
                brushed.insert(i);
            }
        }
    }

    virtual void updateRange(bool upper, float y) override {
        auto range = range_->get();

        y = getValue(y);

        if (upper) {
            if (y < range.x) {
                y = range.x;
            }
            range.y = y;
        } else {
            if (y > range.y) {
                y = range.y;
            }
            range.x = y;
        }

        range_->set(range);
    }

    T p0_;
    T p25_;
    T p75_;
    T p100_;
};
}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ParallelCoordinates::processorInfo_{
    "org.inviwo.ParallelCoordinates",  // Class identifier
    "Parallel Coordinates",            // Display name
    "Plotting",                        // Category
    CodeState::Stable,                 // Code state
    "GL, Plotting",                    // Tags
};
const ProcessorInfo ParallelCoordinates::getProcessorInfo() const { return processorInfo_; }

ParallelCoordinates::ParallelCoordinates()
    : Processor()
    , dataFrame_("dataFrame")
    , brushingAndLinking_("brushingAndLinking")
    , outport_("outport")
    , axisProperties_("axis_", "Axis")
    , colors_("colors", "Colors")
    , axisColor_("axisColor", "Axis Color", vec4(.6f, .6f, .6f, 1))
    , handleColor_("handleColor", "Handle Color", vec4(.6f, .6f, .6f, 1))
    , tf_("tf", "Line Color")
    , tfSelection_("tfSelection", "Selection Color")

    , filteringOptions_("filteringOptions", "Filtering Options")
    , showFiltered_("showFiltered", "Show Filtered", false)
    , filterColor_("filterColor", "Filter Color", vec4(.6f, .6f, .6f, 1.f))
    , filterIntensity_("filterIntensity", "Filter Intensity", 0.5f, 0.01f, 1.0f, 0.001f)

    , blendMode_("blendMode", "Blend Mode")
    , alpha_("alpha", "Alpha", 0.9f)
    , falllofPower_("falllofPower", "Falloff Power", 2.0f, 0.01f, 10.f, 0.01f)
    , lineWidth_("lineWidth", "Line Width", 7.0f, 1.0f, 10.0f)
    , selectedLineWidth_("selectedLineWidth", "Line Width (selected lines)", 3.0f, 1.0f, 10.0f)

    , handleSize_("handleSize", "Handle Size", vec2(handleW, handleH), vec2(1), vec2(100), vec2(1))
    
    , margins_("margins", "Margins", 0, 0, 0, 0)
    
    , selectedAxisName_("selectedAxis", "Selected Axis", "", InvalidationLevel::Valid)
    , selectedAxisId_("selectedAxisId", "Selected AxisID", 0, 0, 100, 1, InvalidationLevel::Valid)
    , selectedColorAxis_("selectedColorAxis", "Selected Color Axis", dataFrame_, false, 1)

    , text_("labels", "Labels")
    , labelPosition_("labelPosition", "Label Position")
    , showValue_("showValue", "Display min/max", true)
    , color_("color_", "Color", vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
             InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , fontSize_("fontSize", "Font size")
    , valuesFontSize_("valuesFontSize", "Font size for min/max")

    , lineShader_("pcp_lines.vert", "pcp_lines.geom", "pcp_lines.frag")
    , axisShader_("pcp_axis.vert", "pcp_axis.frag")
    , handleShader_("pcp_handle.vert", "pcp_handle.frag")

    , linePicking_(this, 1, [&](PickingEvent *p) { linePicked(p); })
    , handlePicking_(this, 1, [&](PickingEvent *p) { handlePicked(p); })

    , textRenderer_()

    , handleImg_(nullptr)
    , recreateLines_(true)
    , textCacheDirty_(true)
    , brushingDirty_(true)  // needs to be true after deserialization
    , extraMarginsLabel_(0, 0, 0, 0)
    , extraMarginsValues_(0, 0, 0, 0)
    , extraMarginsHandles_(handleH, handleW / 2, handleH, handleW / 2)
{
    addPort(dataFrame_);
    addPort(brushingAndLinking_);
    addPort(outport_);

    axisColor_.setSemantics(PropertySemantics::Color);
    handleColor_.setSemantics(PropertySemantics::Color);
    filterColor_.setSemantics(PropertySemantics::Color);

    blendMode_.addOption("additive", "Additive", BlendMode::Additive);
    blendMode_.addOption("subractive", "Subractive", BlendMode::Sutractive);
    blendMode_.addOption("regular", "Regular", BlendMode::Regular);
    blendMode_.addOption("noblend", "None", BlendMode::None);
    blendMode_.setSelectedIndex(2);
    blendMode_.setCurrentStateAsDefault();

    addProperty(margins_);
    addProperty(colors_);
    colors_.addProperty(selectedColorAxis_);
    colors_.addProperty(tfSelection_);
    colors_.addProperty(alpha_);
    colors_.addProperty(falllofPower_);
    colors_.addProperty(axisColor_);
    colors_.addProperty(handleColor_);
    colors_.addProperty(tf_);
    colors_.addProperty(blendMode_);
    addProperty(lineWidth_);
    addProperty(selectedLineWidth_);
    addProperty(axisProperties_);

    addProperty(text_);
    text_.addProperty(labelPosition_);
    text_.addProperty(showValue_);
    text_.addProperty(color_);
    text_.addProperty(fontSize_);
    text_.addProperty(valuesFontSize_);

    axisProperties_.setCollapsed(true);
    axisProperties_.onChange([&]() { recreateLines_ = true; });

    addProperty(selectedAxisName_);
    addProperty(selectedAxisId_);

    addProperty(handleSize_);

    filteringOptions_.addProperty(showFiltered_);
    filteringOptions_.addProperty(filterColor_);
    filteringOptions_.addProperty(filterIntensity_);
    addProperty(filteringOptions_);

    labelPosition_.addOption("none", "None", LabelPosition::None);
    labelPosition_.addOption("above", "Above", LabelPosition::Above);
    labelPosition_.addOption("below", "Below", LabelPosition::Below);
    labelPosition_.setSelectedIndex(2);

    TransferFunction tf;
    tf.clearPoints();
    tf.addPoint(vec2(0, 1), vec4(1, 0, 0, 1));
    tf.addPoint(vec2(0.5, 1), vec4(1, 1, 0, 1));
    tf.addPoint(vec2(1, 1), vec4(0, 1, 0, 1));
    tf_.set(tf);
    tf_.setCurrentStateAsDefault();

    tf.clearPoints();
    tf.addPoint(vec2(0.5, 1), vec4(1, 0, 0, 1));
    tfSelection_.set(tf);
    tfSelection_.setCurrentStateAsDefault();

    handle_ = util::make_unique<SimpleMesh>(DrawType::Triangles, ConnectivityType::Strip);
    auto handleMesh = dynamic_cast<SimpleMesh *>(handle_.get());
    handleMesh->addVertex(vec3(0, 0, 0), vec3(0, 0, 0), vec4(0, 0, 0, 0));
    handleMesh->addVertex(vec3(0, 1, 0), vec3(0, 1, 0), vec4(0, 0, 0, 0));
    handleMesh->addVertex(vec3(1, 0, 0), vec3(1, 0, 0), vec4(0, 0, 0, 0));
    handleMesh->addVertex(vec3(1, 1, 0), vec3(1, 1, 0), vec4(0, 0, 0, 0));
    handleMesh->addIndices(0, 1, 2, 3);

    axis_ = util::make_unique<SimpleMesh>(DrawType::Lines, ConnectivityType::None);
    auto axisMesh = dynamic_cast<SimpleMesh *>(axis_.get());
    axisMesh->addVertex(vec3(0, 0, 0), vec3(0, 0, 0), vec4(1, 0, 0, 1));
    axisMesh->addVertex(vec3(0, 1, 0), vec3(0, 1, 0), vec4(0, 1, 0, 1));
    axisMesh->addIndices(0, 1);

    std::vector<int> fontSizes = {8, 10, 11, 12, 14, 16, 20, 24, 28, 36, 48, 60, 72, 96};
    for (auto size : fontSizes) {
        std::string str = toString(size);
        fontSize_.addOption(str, str, size);
        valuesFontSize_.addOption(str, str, size);
    }
    fontSize_.setSelectedIndex(6);
    fontSize_.setCurrentStateAsDefault();
    valuesFontSize_.setSelectedIndex(4);
    valuesFontSize_.setCurrentStateAsDefault();

    fontSize_.onChange([this]() { textCacheDirty_ = true; });
    valuesFontSize_.onChange([this]() { textCacheDirty_ = true; });

    auto app = InviwoApplication::getPtr();
    auto module = app->getModuleByType<PlottingGLModule>();
    auto path = module->getPath(ModulePath::Images);
    std::string filename = path + "/pcp_handle.png";
    auto factory = app->getDataReaderFactory();
    if (auto reader = factory->getReaderForTypeAndExtension<Layer>("png")) {
        auto outLayer = reader->readData(filename);
        auto ram = outLayer->getRepresentation<LayerRAM>();
        outLayer->setDataFormat(ram->getDataFormat());

        handleImg_ = std::make_shared<Image>(outLayer);
        handleImg_->getRepresentation<ImageRAM>();
    } else {
        LogError("Failed to read handle image");
    }

    lineShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidOutput); });
    axisShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidOutput); });
    handleShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidOutput); });

    handleSize_.onChange([&]() {
        extraMarginsHandles_ = vec4(handleSize_.get().y, handleSize_.get().x / 2,
                                    handleSize_.get().y, handleSize_.get().x / 2);
    });

    dataFrame_.onChange([&]() {
        createOrUpdateProperties();
        recreateLines_ = true;
        textCacheDirty_ = true;
    });
    selectedColorAxis_.onChange([&]() { recreateLines_ = true; });

    setAllPropertiesCurrentStateAsDefault();
}

ParallelCoordinates::~ParallelCoordinates() {}

void ParallelCoordinates::process() {
    auto dims = outport_.getDimensions();

    if (brushingDirty_) {
        updateBrushing();
    }

    if (!lines_ || recreateLines_) {
        buildLineMesh();
    }
    if (!handleDrawer_) {
        handleDrawer_ =
            getNetwork()->getApplication()->getMeshDrawerFactory()->create(handle_.get());
    }
    if (!axisDrawer_) {
        axisDrawer_ = getNetwork()->getApplication()->getMeshDrawerFactory()->create(axis_.get());
    }

    std::vector<AxisBase *> enabledAxis;
    for (auto &p : axisVector_) {
        if (p->property_->isChecked()) {
            enabledAxis.push_back(p.get());
        }
    }

    vec4 backgroundColor(0.0);
    if (blendMode_.get() == BlendMode::Sutractive) {
        backgroundColor = vec4(1.0f);
    }

    utilgl::ClearColor clearColor(backgroundColor);

    utilgl::activateAndClearTarget(outport_, ImageType::ColorPicking);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);

    buildTextCache(dims, enabledAxis);

    extraMargins_ = extraMarginsHandles_;
    if (labelPosition_.get() != LabelPosition::None) {
        extraMargins_.x += extraMarginsLabel_.x;
        extraMargins_.z += extraMarginsLabel_.z;
        extraMargins_.y = std::max(extraMargins_.y, extraMarginsLabel_.y);
        extraMargins_.w = std::max(extraMargins_.w, extraMarginsLabel_.w);
    }
    if (showValue_.get()) {
        extraMargins_.y = std::max(extraMargins_.y, extraMarginsValues_.y);
        extraMargins_.w = std::max(extraMargins_.w, extraMarginsValues_.w);
    }

    drawAxis(dims, enabledAxis, extraMargins_);
    drawLines(dims, enabledAxis, extraMargins_);
    drawHandles(dims, enabledAxis, extraMargins_);
    renderText(dims, enabledAxis, extraMargins_);

    utilgl::deactivateCurrentTarget();
}

void ParallelCoordinates::createOrUpdateProperties() {
    if (dataFrame_.hasData()) {
        auto data = dataFrame_.getData();
        if (!data->getSize()) return;
        axisVector_.clear();

        for (auto &p : axisProperties_.getProperties()) {
            p->setVisible(false);
        }

        size_t col = 0;

        // start at 1 to skip index buffer
        for (size_t i = 1; i < data->getNumberOfColumns(); i++) {
            auto c = data->getColumn(i);

            c->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Scalars>([&](auto ram) -> void {
                    // auto ram = buf->getRAMRepresentation();
                    using T = typename util::PrecsionValueType<decltype(ram)>;
                    using AxisT = Axis<T>;
                    auto &dataVector = ram->getDataContainer();

                    auto minMax = std::minmax_element(dataVector.begin(), dataVector.end());
                    T minV = *minMax.first;
                    T maxV = *minMax.second;

                    if (minV != minV) {
                        minV = 0;
                    }

                    std::string displayName = c->getHeader();
                    std::string identifier = displayName;  // remove non alpha num

                    util::erase_remove_if(identifier, [](char cc) {
                        return !(cc >= -1) || !(std::isalnum(cc) || cc == '_' || cc == '-');
                    });

                    auto prop = dynamic_cast<BoolCompositeProperty *>(
                        axisProperties_.getPropertyByIdentifier(identifier));

                    auto props = [&]() {
                        if (prop) {
                            prop->setVisible(true);

                            auto rangeProp = dynamic_cast<typename AxisT::MinMaxPropType *>(
                                prop->getPropertyByIdentifier("range"));

                            rangeProp->setRangeMin(minV);
                            rangeProp->setRangeMax(maxV);

                            auto usePercentiles = dynamic_cast<BoolProperty *>(
                                prop->getPropertyByIdentifier("usePercentiles"));

                            rangeProp->onChange([&]() { this->updateBrushing(); });

                            return std::make_pair(rangeProp, usePercentiles);
                        } else {

                            auto prop1 = util::make_unique<BoolCompositeProperty>(
                                identifier, displayName, true);

                            prop1->setCollapsed(true);
                            prop1->getBoolProperty()->setSerializationMode(
                                PropertySerializationMode::All);
                            prop1->setSerializationMode(PropertySerializationMode::All);

                            prop1->setChecked(true);

                            auto rangeProp = std::make_unique<typename AxisT::MinMaxPropType>(
                                "range", "Range", static_cast<typename AxisT::FloatType>(minV),
                                static_cast<typename AxisT::FloatType>(maxV),
                                static_cast<typename AxisT::FloatType>(minV),
                                static_cast<typename AxisT::FloatType>(maxV));

                            rangeProp->setSerializationMode(PropertySerializationMode::All);
                            rangeProp->onChange([&]() { this->updateBrushing(); });

                            auto usePercentiles = std::make_unique<BoolProperty>(
                                "usePercentiles", "Use Percentiles", false);
                            usePercentiles->setSerializationMode(PropertySerializationMode::All);

                            prop1->addProperty(rangeProp.get());
                            prop1->addProperty(usePercentiles.get());
                            axisProperties_.addProperty(prop1.get());

                            prop = prop1.release();
                            return std::make_pair(rangeProp.release(), usePercentiles.release());
                        }
                    }();

                    axisVector_.emplace_back(new AxisT(col++, c->getHeader(), prop, props.first,
                                                       props.second, c->getBuffer(), &dataVector));

                });
        }
        handlePicking_.resize(axisVector_.size() * 2);
    }
}

void ParallelCoordinates::buildLineMesh() {
    lines_ = util::make_unique<BasicMesh>();

    std::vector<AxisBase *> enabledAxis;
    for (auto &p : axisVector_) {
        if (p->property_->isChecked()) {
            enabledAxis.push_back(p.get());
        }
    }

    if (!enabledAxis.size()) {
        auto mesh = static_cast<BasicMesh *>(lines_.get());
        linesDrawer_ = std::make_unique<MeshDrawerGL>(mesh);
        recreateLines_ = false;
        LogWarn("DataFrame is empty. No axis to draw.");
        return;
    }

    auto numberOfAxis = enabledAxis.size();
    auto numberOfLines = dataFrame_.getData()->getNumberOfRows();

    auto mesh = static_cast<BasicMesh *>(lines_.get());
    mesh->reserveIndexBuffers(numberOfLines);

    std::vector<BasicMesh::Vertex> vertices;
    vertices.reserve(numberOfAxis * numberOfLines);

    linePicking_.resize(dataFrame_.getData()->getNumberOfRows());

    auto sampler = tf_.get();

    auto colorAxisId = selectedColorAxis_.get();

    if (colorAxisId > 0) colorAxisId--;

    auto colorAxes = axisVector_[glm::clamp(colorAxisId, 0, (int)axisVector_.size() - 1)].get();
    auto selectionAxes =
        axisVector_[std::min(selectedAxisId_.get(), (int)axisVector_.size() - 1)].get();

    float dx = 1.0f / (numberOfAxis - 1);
    for (size_t i = 0; i < numberOfLines; i++) {
        //   if (brushingAndLinking_.isFiltered(i)) continue;
        auto ib = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::Strip);
        auto &ivVector = ib->getDataContainer();
        ivVector.reserve(enabledAxis.size());
        size_t col = 0;

        float valueForColor = colorAxes->getNormalizedAt(i);
        float valueForSelectionColor = selectionAxes->getNormalizedAt(i);

        vec3 texCoord(valueForColor, valueForSelectionColor, 0.0f);
        auto color = sampler.sample(valueForColor);
        if (blendMode_.get() == BlendMode::Sutractive) {
            color.r = 1 - color.r;
            color.g = 1 - color.g;
            color.b = 1 - color.b;
        }
        vec3 pickColor = linePicking_.getColor(i);

        for (auto &axes : enabledAxis) {
            vec3 pos(col++ * dx, axes->getNormalizedAt(i), 0);
            // vec3 pos(col++ * dx, std::log(axes->getNormalizedAt(i) + 1) / std::log(2), 0);
            ivVector.push_back(static_cast<glm::uint32_t>(vertices.size()));
            vertices.push_back({pos, pickColor, texCoord, color});
        }
    }
    mesh->addVertices(vertices);

    linesDrawer_ = std::make_unique<MeshDrawerGL>(mesh);
    recreateLines_ = false;
}

void ParallelCoordinates::drawAxis(size2_t size, std::vector<AxisBase *> enabledAxis,
                                   vec4 extraMargins) {
    axisShader_.activate();

    axisShader_.setUniform("dims", ivec2(size));
    axisShader_.setUniform("spacing", margins_.getAsVec4() + extraMargins);

    float dx = 1.0f / (enabledAxis.size() - 1);

    for (size_t i = 0; i < enabledAxis.size(); i++) {
        float x = i * dx;
        axisShader_.setUniform("x", x);
        axisShader_.setUniform("color", axisColor_.get());
        axisDrawer_->draw();
    }

    // Draw axis

    axisShader_.deactivate();
}

void ParallelCoordinates::drawHandles(size2_t size, std::vector<AxisBase *> enabledAxis,
                                      vec4 extraMargins) {
    utilgl::GlBoolState blendOn(GL_BLEND, true);
    utilgl::BlendModeEquationState blendEq(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);

    handleShader_.activate();

    // Draw axis
    handleShader_.setUniform("dims", ivec2(size));
    handleShader_.setUniform("spacing", margins_.getAsVec4() + extraMargins);

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(handleShader_, cont, *(handleImg_.get()), "tex",
                               ImageType::AllLayers);

    float dx = 1.0f / (enabledAxis.size() - 1);
    size_t i = 0;
    for (auto axes : enabledAxis) {
        float x = i++ * dx;
        auto pickingID = axes->columnId_ * 2;

        handleShader_.setUniform("color", handleColor_.get());
        handleShader_.setUniform("x", x);
        handleShader_.setUniform("w", handleSize_.get().x);
        handleShader_.setUniform("h", handleSize_.get().y);
        handleShader_.setUniform("y", axes->getNormalized(axes->getRange().x));
        handleShader_.setUniform("flipped", 0);
        handleShader_.setUniform("pickColor", handlePicking_.getColor(pickingID + 0));

        handleDrawer_->draw();

        handleShader_.setUniform("y", axes->getNormalized(axes->getRange().y));
        handleShader_.setUniform("flipped", 1);
        handleShader_.setUniform("pickColor", handlePicking_.getColor(pickingID + 1));

        handleDrawer_->draw();
    }

    handleShader_.deactivate();
}

void ParallelCoordinates::drawLines(size2_t size, std::vector<AxisBase *> enabledAxis,
                                    vec4 extraMargins) {
    lineShader_.activate();
    lineShader_.setUniform("spacing", margins_.getAsVec4() + extraMargins);

    auto state = [&]() {

        switch (blendMode_.get()) {
            case BlendMode::Additive:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, true),
                    utilgl::BlendModeEquationState(GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD));
            case BlendMode::Sutractive:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, true),
                    utilgl::BlendModeEquationState(GL_SRC_ALPHA, GL_ONE, GL_FUNC_REVERSE_SUBTRACT));
            case BlendMode::Regular:
                return std::make_tuple(utilgl::GlBoolState(GL_DEPTH_TEST, false),
                                       utilgl::GlBoolState(GL_BLEND, true),
                                       utilgl::BlendModeEquationState(
                                           GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD));
            case BlendMode::None:
            default:
                return std::make_tuple(
                    utilgl::GlBoolState(GL_DEPTH_TEST, false), utilgl::GlBoolState(GL_BLEND, false),
                    utilgl::BlendModeEquationState(GL_NONE, GL_NONE, GL_FUNC_ADD));
        };

    }();

    // Draw lines

    TextureUnitContainer unit, unit1;
    utilgl::bindAndSetUniforms(lineShader_, unit, tf_);
    utilgl::bindAndSetUniforms(lineShader_, unit1, tfSelection_);

    bool enableBlending =
        (blendMode_.get() == BlendMode::Additive || blendMode_.get() == BlendMode::Sutractive ||
         blendMode_.get() == BlendMode::Regular);

    lineShader_.setUniform("additiveBlend", enableBlending);
    lineShader_.setUniform("alpha", alpha_.get());
    lineShader_.setUniform("falllofPower", falllofPower_.get());
    lineShader_.setUniform("lineWidth", lineWidth_.get());
    lineShader_.setUniform("selectedLineWidth", selectedLineWidth_.get());
    lineShader_.setUniform("dims", ivec2(size));

    lineShader_.setUniform("subtractiveBelnding",
                           blendMode_.get() == BlendMode::Sutractive ? 1 : 0);
    lineShader_.setUniform("filterColor", filterColor_.get());
    lineShader_.setUniform("filterIntensity", filterIntensity_.get());

    auto numLines = lines_->getIndexBuffers().size();

    auto drawObject = linesDrawer_->getDrawObject();

    auto iCol = dataFrame_.getData()->getIndexColumn();
    auto &indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    std::vector<size_t> filteredIndices;
    std::vector<size_t> selectIndices;

    lineShader_.setUniform("selected", 0);
    lineShader_.setUniform("filtered", 0);
    for (size_t i = 0; i < numLines; i++) {
        if (brushingAndLinking_.isFiltered(indexCol[i])) {
            if (showFiltered_) {
                filteredIndices.push_back(i);
            }
            continue;
        }
        if (brushingAndLinking_.isSelected(indexCol[i])) {
            selectIndices.push_back(i);
            continue;
        }
        drawObject.draw(i);
    }

    if (showFiltered_) {
        lineShader_.setUniform("selected", 0);
        lineShader_.setUniform("filtered", 1);
        for (const auto &i : filteredIndices) {
            drawObject.draw(i);
        }
    }

    lineShader_.setUniform("selected", 1);
    lineShader_.setUniform("filtered", 0);
    for (const auto &i : selectIndices) {
        if (brushingAndLinking_.isFiltered(indexCol[i])) continue;
        drawObject.draw(i);
    }

    lineShader_.deactivate();
}

static const size_t valLabelXoffest = 16;
void ParallelCoordinates::buildTextCache(size2_t size, std::vector<AxisBase *> enabledAxis) {
    auto pos = labelPosition_.get();
    if (textCacheDirty_) {
        textCacheDirty_ = false;
        extraMarginsLabel_ = vec4(0);

        size_t i = 0;
        for (auto &axes : axisVector_) {
            std::string minV = toString(axes->getRangeMin());
            std::string maxV = toString(axes->getRangeMax());

            axes->labelTexture_ = util::createTextTexture(
                textRenderer_, axes->name_, fontSize_.get(), color_.get(), axes->labelTexture_);
            axes->minValTexture_ = util::createTextTexture(
                textRenderer_, minV, valuesFontSize_.get(), color_.get(), axes->minValTexture_);
            axes->maxValTexture_ = util::createTextTexture(
                textRenderer_, maxV, valuesFontSize_.get(), color_.get(), axes->maxValTexture_);

            size2_t labelSize = axes->labelTexture_->getDimensions();
            size2_t minVLabelSize = axes->minValTexture_->getDimensions();
            size2_t maxVLabelSize = axes->maxValTexture_->getDimensions();

            if (i == 0) {
                if (pos == LabelPosition::Above) {
                    extraMarginsLabel_.x = labelSize.y;
                } else {
                    extraMarginsLabel_.z = labelSize.y;
                }
                extraMarginsLabel_.w = labelSize.x / 2.0f;
            }
            if (i == enabledAxis.size() - 1) {
                extraMarginsLabel_.y = labelSize.x / 2.0f;
                extraMarginsValues_.y =
                    std::max(minVLabelSize.x + valLabelXoffest, maxVLabelSize.x + valLabelXoffest);
            }
            i++;
        }
    }
}

void ParallelCoordinates::renderText(size2_t outputsize, std::vector<AxisBase *> enabledAxis,
                                     vec4 extraMargins) {
    auto pos = labelPosition_.get();

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto outputSizeWOMarings = outputsize;
    outputSizeWOMarings.x -=
        margins_.getRight() + margins_.getLeft() + extraMargins.y + extraMargins.w;
    outputSizeWOMarings.y -= margins_.getTop() + margins_.getBottom();

    float dx = 1.0f / (enabledAxis.size() - 1);

    if (showValue_) {
        textRenderer_.setFontSize(valuesFontSize_.get());
        // Draw min and max values
        size_t i = 0;

        for (auto axes : enabledAxis) {
            float x = i++ * dx;
            ivec2 textPos(0);
            textPos.x =
                x * outputSizeWOMarings.x + margins_.getLeft() + extraMargins.w + valLabelXoffest;
            if (pos == LabelPosition::Below) {
                textPos.y = extraMarginsLabel_.z;
            }

            textureRenderer_.render(axes->minValTexture_, textPos, outputsize);

            textPos.y = outputsize.y - axes->maxValTexture_->getDimensions().y;
            if (pos == LabelPosition::Above) {
                textPos.y -= extraMarginsLabel_.x;
            }
            textureRenderer_.render(axes->maxValTexture_, textPos, outputsize);
        }
    }

    if (pos != LabelPosition::None) {
        textRenderer_.setFontSize(fontSize_.get());
        // Draw header labels

        size_t i = 0;

        for (auto axes : enabledAxis) {
            float x = i++ * dx;

            vec2 size(axes->labelTexture_->getDimensions());

            ivec2 textPos(0);
            textPos.x =
                x * outputSizeWOMarings.x + margins_.getLeft() + extraMargins.w - size.x / 2.0f;

            if (pos == LabelPosition::Above) {
                textPos.y = outputsize.y - size.y;
            }

            textureRenderer_.render(axes->labelTexture_, textPos, outputsize);
        }
    }
}

void ParallelCoordinates::linePicked(PickingEvent *p) {
    if (auto mouseEvent = p->getEventAs<MouseEvent>()) {
        if (!(mouseEvent->buttonState() & MouseButton::Left)) return;
        if (mouseEvent->state() != MouseState::Press) return;

        auto iCol = dataFrame_.getData()->getIndexColumn();
        auto &indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

        auto id = p->getPickedId();
        brushingAndLinking_.sendSelectionEvent({indexCol[id]});
        mouseEvent->markAsUsed();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void ParallelCoordinates::handlePicked(PickingEvent *p) {
    if (auto mouseEvent = p->getEventAs<MouseEvent>()) {
        if (!(mouseEvent->buttonState() & MouseButton::Left)) return;
        if (mouseEvent->state() != MouseState::Move) return;
        auto canvasSize = outport_.getDimensions();
        auto pickedID = p->getPickedId();

        auto axisID = pickedID / 2;
        bool upper = pickedID % 2 == 1;

        auto marigins = margins_.getAsVec4() + extraMargins_;

        auto newY =
            (mouseEvent->pos().y - marigins[2]) / (canvasSize.y - marigins[2] - marigins[0]);
        newY = glm::clamp(newY, 0.0, 1.0);

        // auto axis = axisVector_[axisID].get();
        selectedAxisId_.set(static_cast<int>(axisID));

        axisVector_[axisID]->updateRange(upper, newY);
        mouseEvent->markAsUsed();
    }
}

void ParallelCoordinates::updateBrushing() {
    brushingDirty_ = false;
    std::unordered_set<size_t> brushed;

    for (auto &axes : axisVector_) {
        axes->updateBrushing(brushed);
    }

    std::unordered_set<size_t> brushedID;
    auto iCol = dataFrame_.getData()->getIndexColumn();
    auto &indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    std::for_each(brushed.begin(), brushed.end(),
                  [&](const auto &id) { brushedID.insert(indexCol[id]); });

    brushingAndLinking_.sendFilterEvent(brushedID);
}

}  // namespace plot

}  // namespace inviwo
