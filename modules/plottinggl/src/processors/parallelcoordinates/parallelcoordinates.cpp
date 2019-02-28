/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h>
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

#include <modules/base/algorithm/dataminmax.h>

#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/io/datareaderfactory.h>

#include <modules/plotting/utils/statsutils.h>
#include <modules/plotting/datastructures/dataframeutil.h>
#include <inviwo/core/util/utilities.h>

namespace inviwo {

namespace plot {

static const float handleW = 40;
static const float handleH = 20;

static const size_t handleCaptionMargin = 2;  // Distance between caption text and handle

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ParallelCoordinates::processorInfo_{
    "org.inviwo.ParallelCoordinates",  // Class identifier
    "Parallel Coordinates",            // Display name
    "Plotting",                        // Category
    CodeState::Stable,                 // Code state
    "GL, Plotting",                    // Tags
};
const ProcessorInfo ParallelCoordinates::getProcessorInfo() const { return processorInfo_; }

void ParallelCoordinates::invokeEvent(Event *event) {
    Processor::invokeEvent(event);
    if (event->getAs<ResizeEvent>()) {
        updateAxesLayout();
    }
}

ParallelCoordinates::ParallelCoordinates()
    : Processor()
    , dataFrame_("dataFrame")
    , brushingAndLinking_("brushingAndLinking")
    , outport_("outport")
    , axisProperties_("axisProps_", "Axis")
    , colors_("colors", "Colors")
    , axisColor_("axisColor", "Axis Color", vec4(.3f, .3f, .3f, 1))
    , axisHoverColor_("axisHoverColor", "Axis Hover Color", vec4(.6f, .6f, .6f, 1))
    , axisSelectedColor_("axisSelectedColor", "Axis Selected Color", vec4(.8f, .8f, .8f, 1))
    , handleBaseColor_("handleColor", "Handle Color (Not filtering)", vec4(.92f, .92f, .92f, 1))
    , handleFilteredColor_("handleFilteredColor", "Handle Color (When filtering)",
                           vec4(.5f, .5f, .5f, 1))
    , tf_("tf", "Line Color")
    , tfSelection_("tfSelection", "Selection Color")
    , enableHoverColor_("enableHoverColor", "Enable Hover Color", true)

    , filteringOptions_("filteringOptions", "Filtering Options")
    , showFiltered_("showFiltered", "Show Filtered", false)
    , filterColor_("filterColor", "Filter Color", vec4(.6f, .6f, .6f, 0.2f))
    , filterIntensity_("filterIntensity", "Filter Intensity", 0.7f, 0.01f, 1.0f, 0.001f)

    , resetHandlePositions_("resetHandlePositions", "Reset Handle Positions")

    , blendMode_("blendMode", "Blend Mode")
    , alpha_("alpha", "Alpha", 0.9f)
    , filterAlpha_("filterAlpha", "Filter Alpha", 0.2f)
    , falllofPower_("falllofPower", "Falloff Power", 2.0f, 0.01f, 10.f, 0.01f)
    , lineWidth_("lineWidth", "Line Width", 7.0f, 1.0f, 10.0f)
    , selectedLineWidth_("selectedLineWidth", "Line Width (selected lines)", 3.0f, 1.0f, 10.0f)

    , handleSize_("handleSize", "Handle Size", vec2(handleW, handleH), vec2(1), vec2(100), vec2(1))

    , margins_("margins", "Margins", 0, 0, 0, 0)
    , autoMargins_("autoMargins", "Auto")

    , selectedColorAxis_("selectedColorAxis", "Selected Color Axis", dataFrame_, false, 1)

    , text_("labels", "Labels")
    , labelPosition_("labelPosition", "Label Position")
    , showValue_("showValue", "Display min/max", true)
    , color_("color_", "Color", vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
             InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , fontSize_("fontSize", "Font size")
    , valuesFontSize_("valuesFontSize", "Font size for min/max")

    , lineShader_("pcp_lines.vert", "pcp_lines.geom", "pcp_lines.frag")
    , axisShader_("pcp_axis.vert", "pcp_axis.geom", "pcp_axis.frag")
    , handleShader_("pcp_handle.vert", "pcp_handle.frag")

    , linePicking_(this, 1, [&](PickingEvent *p) { linePicked(p); })
    , axisPicking_(this, 1, [&](PickingEvent *p) { axisPicked(p); })
    , handlePicking_(this, 1, [&](PickingEvent *p) { handlePicked(p); })

    , handleImg_(nullptr)
    , recreateLines_(true)
    , brushingDirty_(true)  // needs to be true after deserialization
{
    addPort(dataFrame_);
    addPort(brushingAndLinking_);
    addPort(outport_);

    axisColor_.setSemantics(PropertySemantics::Color);
    axisHoverColor_.setSemantics(PropertySemantics::Color);
    axisSelectedColor_.setSemantics(PropertySemantics::Color);
    handleBaseColor_.setSemantics(PropertySemantics::Color);
    handleFilteredColor_.setSemantics(PropertySemantics::Color);
    filterColor_.setSemantics(PropertySemantics::Color);

    blendMode_.addOption("additive", "Additive", BlendMode::Additive);
    blendMode_.addOption("subractive", "Subractive", BlendMode::Sutractive);
    blendMode_.addOption("regular", "Regular", BlendMode::Regular);
    blendMode_.addOption("noblend", "None", BlendMode::None);
    blendMode_.setSelectedIndex(2);
    blendMode_.setCurrentStateAsDefault();

    addProperty(margins_);
    margins_.addProperty(autoMargins_);
    addProperty(colors_);
    colors_.addProperty(selectedColorAxis_);
    colors_.addProperty(tfSelection_);
    colors_.addProperty(enableHoverColor_);
    colors_.addProperty(alpha_);
    colors_.addProperty(falllofPower_);
    colors_.addProperty(axisColor_);
    colors_.addProperty(axisHoverColor_);
    colors_.addProperty(axisSelectedColor_);
    colors_.addProperty(handleBaseColor_);
    colors_.addProperty(handleFilteredColor_);
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
    text_.onChange([&]() { autoMargins_.pressButton(); });

    axisProperties_.setCollapsed(true);
    axisProperties_.onChange([&]() { recreateLines_ = true; });

    addProperty(handleSize_);

    filteringOptions_.addProperty(showFiltered_);
    filteringOptions_.addProperty(filterColor_);
    filteringOptions_.addProperty(filterIntensity_);
    filteringOptions_.addProperty(filterAlpha_);
    addProperty(filteringOptions_);

    filterColor_.onChange([&]() { filterAlpha_.set(filterColor_.get().w); });
    filterAlpha_.onChange([&]() {
        auto color = filterColor_.get();
        filterColor_.set(vec4(color.x, color.y, color.z, filterAlpha_.get()));
    });

    addProperty(resetHandlePositions_);

    labelPosition_.addOption("none", "None", LabelPosition::None);
    labelPosition_.addOption("above", "Above", LabelPosition::Above);
    labelPosition_.addOption("below", "Below", LabelPosition::Below);
    labelPosition_.setSelectedIndex(2);

    showValue_.onChange([&]() { autoMargins_.pressButton(); });

    autoMargins_.onChange([&]() {
        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;

        float leftLabelWidth = 0;
        float rightLabelWidth = 0;
        size_t maxLabelHeight = 0;

        for (auto &elem : axisVector_) {
            auto p = std::get<0>(elem);
            if (p->isChecked()) {
                const auto &renderer = std::get<2>(elem);
                const auto &axisProp = std::get<1>(elem);

                // Label offset by tick mark
                auto tickMarkOffset = axisProp->labels_.font_.anchorPos_.get().x;
                auto labelX = showValue_.get()
                                  ? (tickMarkOffset + renderer->getLabelAtlasTexture()->getWidth())
                                  : 0;
                if (labelPosition_.get() != LabelPosition::None) {
                    // Caption might stick out next to the handle
                    auto captionX = static_cast<float>(0.5f*renderer->getCaptionTextSize().x);
                    if (leftLabelWidth == 0) {
                        leftLabelWidth = std::max(labelX, captionX);
                    }
                    rightLabelWidth = std::max(labelX, captionX);
                    maxLabelHeight = std::max(
                        maxLabelHeight, 2 *renderer->getCaptionTextSize().y + handleCaptionMargin);
                }
                if (showValue_.get()) {
                    right = labelX;
                }
            }
        }
        if (labelPosition_.get() == LabelPosition::Above) {
            top = static_cast<float>(maxLabelHeight);
        } else if (labelPosition_.get() == LabelPosition::Below) {
            bottom = static_cast<float>(maxLabelHeight);
        }

        left = std::max(leftLabelWidth, handleSize_.get().x / 2.f);
        right = std::max(rightLabelWidth, right + handleSize_.get().x / 2.f);
        top += handleSize_.get().y;
        bottom += handleSize_.get().y;
        margins_.setMargins(top + 1, right + 1, bottom + 1, left + 1);
        // plus 1 to avoid text at the directly at the borders

        updateAxesLayout();
    });

    TransferFunction tf;
    tf.clear();
    tf.add(0.0, vec4(1, 0, 0, 1));
    tf.add(0.5, vec4(1, 1, 0, 1));
    tf.add(1.0, vec4(0, 1, 0, 1));
    tf_.set(tf);
    tf_.setCurrentStateAsDefault();

    tf.clear();
    tf.add(0.5, vec4(1, 0, 0, 1));
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

    fontSize_.onChange([this]() { updateAxesLayout(); });
    valuesFontSize_.onChange([this]() { updateAxesLayout(); });

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

    dataFrame_.onChange([&]() {
        createOrUpdateProperties();
        recreateLines_ = true;
    });
    selectedColorAxis_.onChange([&]() { recreateLines_ = true; });

    resetHandlePositions_.onChange([&]() {
        for (auto &elem : axisVector_) {
            auto axis = std::get<0>(elem);
            axis->moveHandle(true, std::numeric_limits<double>::max());
            axis->moveHandle(false, std::numeric_limits<double>::lowest());
        }
    });

    setAllPropertiesCurrentStateAsDefault();
}

ParallelCoordinates::~ParallelCoordinates() {}

void ParallelCoordinates::process() {
    auto dims = outport_.getDimensions();

    std::vector<ColumnAxis *> enabledAxis;
    for (auto &elem : axisVector_) {
        auto axis = std::get<0>(elem);
        if (axis->isChecked()) {
            enabledAxis.push_back(&elem);
        }
    }

    if (brushingDirty_) {
        updateBrushing();
    }

    if (!lines_ || recreateLines_) {
        buildLineMesh(enabledAxis);
    }
    if (!handleDrawer_) {
        handleDrawer_ =
            getNetwork()->getApplication()->getMeshDrawerFactory()->create(handle_.get());
    }
    if (!axisDrawer_) {
        axisDrawer_ = getNetwork()->getApplication()->getMeshDrawerFactory()->create(axis_.get());
    }

    vec4 backgroundColor(0.0);
    if (blendMode_.get() == BlendMode::Sutractive) {
        backgroundColor = vec4(1.0f);
    }

    utilgl::ClearColor clearColor(backgroundColor);

    utilgl::activateAndClearTarget(outport_, ImageType::ColorPicking);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);


    drawLines(dims);
    drawAxis(dims, enabledAxis);
    drawHandles(dims, enabledAxis);

    utilgl::deactivateCurrentTarget();
}

void ParallelCoordinates::createOrUpdateProperties() {
    axisVector_.clear();
    for (auto &p : axisProperties_.getProperties()) {
        p->setVisible(false);
    }
    if (dataFrame_.hasData()) {
        auto data = dataFrame_.getData();
        if (!data->getNumberOfRows()) return;
        if (!data->getNumberOfColumns()) return;

        axisPicking_.resize(data->getNumberOfColumns());
        for (size_t i = 0; i < data->getNumberOfColumns(); i++) {
            auto c = data->getColumn(i);
            std::string displayName = c->getHeader();
            std::string identifier = util::stripIdentifier(displayName);
            // Create axis for filtering
            auto prop = [&]() -> ParallelCoordinatesAxisSettingsProperty * {
                if (auto p = axisProperties_.getPropertyByIdentifier(identifier)) {
                    if (auto pcasp = dynamic_cast<ParallelCoordinatesAxisSettingsProperty *>(p)) {
                        return pcasp;
                    }
                    throw inviwo::Exception(
                        "Failed to convert property to "
                        "ParallelCoordinatesAxisSettingsProperty");
                } else {
                    auto newProp = std::make_unique<ParallelCoordinatesAxisSettingsProperty>(
                        identifier, displayName);
                    auto ptr = newProp.get();
                    axisProperties_.addProperty(newProp.release());
                    return ptr;
                }
            }();
            // Name will be empty string first time this is called
            if (prop->name_.empty()) {
                prop->name_ = c->getHeader();
                prop->range.onChange([&]() { this->updateBrushing(); });
            }
            prop->columnId_ = axisVector_.size();
            prop->setVisible(true);
            prop->updateFromColumn(c);

            // Create axis for rendering
            auto categoricalColumn = dynamic_cast<const CategoricalColumn *>(c.get());
            std::unique_ptr<AxisProperty> axisProp(
                (categoricalColumn != nullptr)
                    ? new CategoricalAxisProperty(identifier, displayName,
                                                  categoricalColumn->getCategories(),
                                                  AxisProperty::Orientation::Vertical)
                    : new AxisProperty(identifier, displayName,
                                       AxisProperty::Orientation::Vertical));
            axisProp->caption_.title_ = c->getHeader();
            axisProp->ticks_.minorTicks_.style_.setSelectedValue(TickStyle::None);
            // Horizontal caption for vertical axis
            axisProp->caption_.rotation_.set(270.f);
            axisProp->caption_.position_.set(0.f);
            axisProp->caption_.setChecked(labelPosition_.getSelectedValue() != LabelPosition::None);
            axisProp->range_.set(dvec2{prop->range.getRangeMin(), prop->range.getRangeMax()},
                                 dvec2{prop->range.getRangeMin(), prop->range.getRangeMax()},
                                 0.1 * (prop->range.getRangeMax() - prop->range.getRangeMin()), 0);

            auto renderer = std::make_unique<AxisRenderer>(*axisProp);

            renderer->setAxisPickingId(axisPicking_.getPickingId(i));
            axisVector_.emplace_back(
                std::make_tuple(prop, std::move(axisProp), std::move(renderer)));
        }
        updateAxesLayout();
        handlePicking_.resize(axisVector_.size() * 2);
    }
}

void ParallelCoordinates::buildLineMesh(const std::vector<ColumnAxis *> &enabledAxis) {
    lines_ = util::make_unique<BasicMesh>();

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

    linePicking_.resize(numberOfLines);

    auto sampler = tf_.get();

    auto colorAxisId = selectedColorAxis_.get();

    auto colorAxes =
        std::get<0>(axisVector_[glm::clamp(colorAxisId, 0, (int)axisVector_.size() - 1)]);

    float dx = 1.0f / (numberOfAxis - 1);
    for (size_t i = 0; i < numberOfLines; i++) {
        //   if (brushingAndLinking_.isFiltered(i)) continue;
        auto ib = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::Strip);
        auto &ivVector = ib->getDataContainer();
        ivVector.reserve(enabledAxis.size());
        size_t col = 0;

        float valueForColor = static_cast<float>(colorAxes->getNormalizedAt(i));
        vec3 texCoord(valueForColor);
        auto color = sampler.sample(valueForColor);
        if (blendMode_.get() == BlendMode::Sutractive) {
            color.r = 1 - color.r;
            color.g = 1 - color.g;
            color.b = 1 - color.b;
        }
        vec3 pickColor = linePicking_.getColor(i);

        for (auto &elem : enabledAxis) {
            auto axes = std::get<0>(*elem);
            vec3 pos(col++ * dx, axes->getNormalizedAt(i), 0);
            ivVector.push_back(static_cast<glm::uint32_t>(vertices.size()));
            vertices.push_back({pos, pickColor, texCoord, color});
        }
    }
    mesh->addVertices(vertices);

    linesDrawer_ = std::make_unique<MeshDrawerGL>(mesh);
    recreateLines_ = false;
}

void ParallelCoordinates::drawAxis(size2_t size, const std::vector<ColumnAxis *> &enabledAxis) {
    const size2_t lowerLeft(margins_.getLeft(), margins_.getBottom());
    const size2_t upperRight(size.x - 1 - margins_.getRight(), size.y - 1 - margins_.getTop());

    const auto padding = 0;
    float dx = 1.0f / (enabledAxis.size() - 1);

    for (size_t i = 0; i < enabledAxis.size(); i++) {
        auto &axis(std::get<1>(*enabledAxis[i]));
        auto &renderer(std::get<2>(*enabledAxis[i]));
        auto columnId = std::get<0>(*enabledAxis[i])->columnId_;
        if (hoveredAxis_ == columnId) {
            axis->color_.set(axisHoverColor_.get());
            axis->ticks_.majorTicks_.color_.set(axisHoverColor_.get());
            axis->ticks_.minorTicks_.color_.set(axisHoverColor_.get());
            axis->width_.set(4);
        } else if (brushingAndLinking_.isColumnSelected(columnId)) {
            axis->color_.set(axisSelectedColor_.get());
            axis->ticks_.majorTicks_.color_.set(axisSelectedColor_.get());
            axis->ticks_.minorTicks_.color_.set(axisSelectedColor_.get());
            axis->width_.set(6);
        } else {
            axis->color_.set(axisColor_.get());
            axis->ticks_.majorTicks_.color_.set(axisColor_.get());
            axis->ticks_.minorTicks_.color_.set(axisColor_.get());
            axis->width_.set(2);
        }
        auto x = static_cast<size_t>(i * dx * (upperRight.x - lowerLeft.x));

        renderer->render(size, lowerLeft + size2_t(x, padding),
                         size2_t(lowerLeft.x + x, upperRight.y - padding));
    }
}

void ParallelCoordinates::drawHandles(size2_t size, const std::vector<ColumnAxis *> &enabledAxis) {
    utilgl::GlBoolState blendOn(GL_BLEND, true);
    utilgl::BlendModeEquationState blendEq(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);

    handleShader_.activate();

    // Draw axis
    handleShader_.setUniform("dims", ivec2(size));
    handleShader_.setUniform("spacing", margins_.getAsVec4());

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(handleShader_, cont, *(handleImg_.get()), "tex",
                               ImageType::AllLayers);

    const auto filteredColor = handleFilteredColor_.get();
    const auto notFilteredColor = handleBaseColor_.get();
    const size2_t lowerLeft(margins_.getLeft(), margins_.getBottom());
    const size2_t upperRight(size.x - 1 - margins_.getRight(), size.y - 1 - margins_.getTop());
    float dx = 1.0f / (enabledAxis.size() - 1);
    size_t i = 0;
    for (auto elem : enabledAxis) {
        auto axes = std::get<0>(*elem);
        float x = std::floor(i++ * dx * (upperRight.x - lowerLeft.x) + lowerLeft.x);
        float y = std::floor(axes->getNormalized(axes->range.get().x) * (upperRight.y - lowerLeft.y)  + lowerLeft.y);
        auto pickingID = axes->columnId_ * 2;
        x = x / (size.x / 2.f) - 1.f;
        y = y / (size.y / 2.f) - 1.f;
        // lower
        handleShader_.setUniform("color", axes->lowerBrushed_ ? filteredColor : notFilteredColor);
        handleShader_.setUniform("x", x);
        handleShader_.setUniform("w", handleSize_.get().x);
        handleShader_.setUniform("h", handleSize_.get().y);
        handleShader_.setUniform("y", y);
        handleShader_.setUniform("flipped", 0);
        handleShader_.setUniform("pickColor", handlePicking_.getColor(pickingID + 0));

        handleDrawer_->draw();
        y = std::floor(axes->getNormalized(axes->range.get().y) * (upperRight.y - lowerLeft.y)  + lowerLeft.y);
                y = y / (size.y / 2.f) - 1.f;
        handleShader_.setUniform("color", axes->upperBrushed_ ? filteredColor : notFilteredColor);
        handleShader_.setUniform("y", y);
        handleShader_.setUniform("flipped", 1);
        handleShader_.setUniform("pickColor", handlePicking_.getColor(pickingID + 1));

        handleDrawer_->draw();
    }

    handleShader_.deactivate();
}

void ParallelCoordinates::drawLines(size2_t size) {
    lineShader_.activate();
    lineShader_.setUniform("spacing", margins_.getAsVec4());

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
    lineShader_.setUniform("filteredAlpha", filterAlpha_.get());
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

    std::vector<size_t> selectIndices;

    lineShader_.setUniform("selected", 0);
    lineShader_.setUniform("hovering", 0);
    if (showFiltered_) {
        lineShader_.setUniform("filtered", 1);
        for (size_t i = 0; i < numLines; i++) {
            if (brushingAndLinking_.isFiltered(indexCol[i])) drawObject.draw(i);
        }
    }

    lineShader_.setUniform("filtered", 0);
    for (size_t i = 0; i < numLines; i++) {
        if (brushingAndLinking_.isFiltered(indexCol[i])) {
            continue;
        }
        if (brushingAndLinking_.isSelected(indexCol[i])) {
            selectIndices.push_back(i);
            continue;
        }
        drawObject.draw(i);
    }

    lineShader_.setUniform("selected", 1);
    lineShader_.setUniform("filtered", 0);
    for (const auto &i : selectIndices) {
        if (brushingAndLinking_.isFiltered(indexCol[i])) continue;
        drawObject.draw(i);
    }

    lineShader_.setUniform("hovering", 1);
    lineShader_.setUniform("selected", 0);
    lineShader_.setUniform("filtered", 0);
    if (hoveredLine_ != -1 && !brushingAndLinking_.isFiltered(hoveredLine_))
        drawObject.draw(hoveredLine_);

    lineShader_.deactivate();
}

void ParallelCoordinates::linePicked(PickingEvent *p) {
    if (auto df = dataFrame_.getData()) {
        // Show tooltip about current line
        if (p->getHoverState() == PickingHoverState::Move ||
            p->getHoverState() == PickingHoverState::Enter) {
            p->setToolTip(dataframeutil::createToolTipForRow(*df, p->getPickedId()));
            if (enableHoverColor_.get()) {
                hoveredLine_ = static_cast<int>(p->getPickedId());
                invalidate(InvalidationLevel::InvalidOutput);
            }
        } else {
            p->setToolTip("");
            hoveredLine_ = -1;
            invalidate(InvalidationLevel::InvalidOutput);
        }
    }

    if (p->getState() == PickingState::Updated && p->getPressState() == PickingPressState::Press &&
        p->getPressItem() == PickingPressItem::Primary) {

        auto iCol = dataFrame_.getData()->getIndexColumn();
        auto &indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

        auto id = p->getPickedId();
        if (brushingAndLinking_.isSelected(indexCol[id])) {
            brushingAndLinking_.sendSelectionEvent({});
        } else {
            brushingAndLinking_.sendSelectionEvent({indexCol[id]});
        }

        p->markAsUsed();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void ParallelCoordinates::axisPicked(PickingEvent *p) {
    const auto pickedID = p->getPickedId();

    if (p->getHoverState() == PickingHoverState::Move ||
        p->getHoverState() == PickingHoverState::Enter) {
        hoveredAxis_ = static_cast<int>(pickedID);
        invalidate(InvalidationLevel::InvalidOutput);
    } else {
        hoveredAxis_ = -1;
        invalidate(InvalidationLevel::InvalidOutput);
    }

    if (p->getState() == PickingState::Updated && p->getPressState() == PickingPressState::Press &&
        p->getPressItem() == PickingPressItem::Primary) {

        if (brushingAndLinking_.isColumnSelected(pickedID)) {
            brushingAndLinking_.sendColumnSelectionEvent({});
        } else {
            brushingAndLinking_.sendColumnSelectionEvent({pickedID});
        }
        p->markAsUsed();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void ParallelCoordinates::handlePicked(PickingEvent *p) {
    const auto pickedID = p->getPickedId();
    const auto axisID = pickedID / 2;
    const bool upper = pickedID % 2 == 1;
    if (p->getHoverState() == PickingHoverState::Move ||
        p->getHoverState() == PickingHoverState::Enter) {
        auto axis = std::get<0>(axisVector_[axisID]);
        const auto rangeValue =
            axis->getValue(upper ? axis->range.getRangeMax() : axis->range.getRangeMin());
        p->setToolTip(std::to_string(rangeValue));
    } else {
        p->setToolTip("");
    }

    if (p->getState() == PickingState::Updated && p->getPressState() == PickingPressState::Move &&
        p->getPressItems().count(PickingPressItem::Primary)) {
        // move axis range handle
        auto canvasSize = outport_.getDimensions();
        auto marigins = margins_.getAsVec4();

        const auto pos = p->getPosition() * dvec2(p->getCanvasSize());

        auto newY = (pos.y - marigins[2]) / (canvasSize.y - marigins[2] - marigins[0]);
        newY = glm::clamp(newY, 0.0, 1.0);
        auto axis = std::get<0>(axisVector_[axisID]);
        axis->moveHandle(upper, newY);
        p->markAsUsed();
    }
}

void ParallelCoordinates::updateBrushing() {
    brushingDirty_ = false;
    std::unordered_set<size_t> brushed;

    for (auto &elem : axisVector_) {
        auto axis = std::get<0>(elem);
        axis->updateBrushing(brushed);
    }

    std::unordered_set<size_t> brushedID;
    auto iCol = dataFrame_.getData()->getIndexColumn();
    auto &indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    std::for_each(brushed.begin(), brushed.end(),
                  [&](const auto &id) { brushedID.insert(indexCol[id]); });

    brushingAndLinking_.sendFilterEvent(brushedID);
}

void ParallelCoordinates::updateAxesLayout() {
    auto firstVisible = true;
    for (auto &p : axisVector_) {
        auto &prop = std::get<1>(p);
        // Place labels on the left side of the first
        // and right side for the others
        if (firstVisible && prop->visible_) {
          prop->placement_.set(AxisProperty::Placement::Outside);
          firstVisible = false;
        } else {
          prop->placement_.set(AxisProperty::Placement::Inside);
        }
        prop->labels_.setChecked(showValue_.get());
        prop->labels_.color_.set(color_.get());
        prop->labels_.font_.fontSize_.set(valuesFontSize_.get());

        prop->color_.set(axisColor_.get());
        prop->caption_.color_.set(color_.get());
        prop->caption_.font_.fontSize_.set(fontSize_.get());
        prop->caption_.setChecked(labelPosition_.get() != LabelPosition::None);
        const auto &renderer = std::get<2>(p);

        // Horizontal offset is given in pixels and since we are using vertical alignment
        // it is the height of the text
        float x = (0.f - 0.5f*renderer->getCaptionTextSize().y);
        // Vertical offset is given with respect to axis length
        auto axisLength = outport_.getDimensions().y - margins_.getTop() - margins_.getBottom();
        float y = (renderer->getCaptionTextSize().y/2 + handleSize_.get().y + handleCaptionMargin) /
                  static_cast<float>(axisLength);

        if (labelPosition_.get() == LabelPosition::Above) {
            y += 1.f;
        } else if (labelPosition_.get() == LabelPosition::Below) {
            y = -y;
        }
        // Horizontal offset, clamp to pixel position to avoid blur
        prop->caption_.offset_.set(static_cast<int>(x), x - 10.f, x + 10.f, 1.f);
        // Vertical offset
        prop->caption_.position_.set(y, y - 0.1f, y + .1f, 0.05f);
    }
}

}  // namespace plot

}  // namespace inviwo
