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

    , textRenderer_()

    , handleImg_(nullptr)
    , recreateLines_(true)
    , textCacheDirty_(true)
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

    autoMargins_.onChange([&]() {
        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;

        float leftLabelWidth = 0;
        float rightLabelWidth = 0;
        size_t maxLabelHeight = 0;

        for (auto &p : axisVector_) {
            if (p->isChecked()) {
                if (labelPosition_.get() != LabelPosition::None) {
                    if (leftLabelWidth == 0) {
                        leftLabelWidth = p->labelTexture_->getWidth() / 2.f;
                    }
                    rightLabelWidth = p->labelTexture_->getWidth() / 2.f;
                    maxLabelHeight = std::max(maxLabelHeight, p->labelTexture_->getHeight());
                }
                if (showValue_.get()) {
                    right = static_cast<float>(p->minValTexture_->getWidth());
                    right = static_cast<float>(p->maxValTexture_->getWidth());
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

    dataFrame_.onChange([&]() {
        createOrUpdateProperties();
        recreateLines_ = true;
        textCacheDirty_ = true;
    });
    selectedColorAxis_.onChange([&]() { recreateLines_ = true; });

    resetHandlePositions_.onChange([&]() {
        for (auto axis : axisVector_) {
            axis->moveHandle(true, std::numeric_limits<double>::max());
            axis->moveHandle(false, std::numeric_limits<double>::lowest());
        }
    });

    setAllPropertiesCurrentStateAsDefault();
}

ParallelCoordinates::~ParallelCoordinates() {}

void ParallelCoordinates::process() {
    auto dims = outport_.getDimensions();

    std::vector<ParallelCoordinatesAxisSettingsProperty *> enabledAxis;
    for (auto &p : axisVector_) {
        if (p->isChecked()) {
            enabledAxis.push_back(p);
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

    buildTextCache(enabledAxis);

    drawAxis(dims, enabledAxis);
    drawLines(dims);
    drawHandles(dims, enabledAxis);
    renderText(dims, enabledAxis);

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

        for (size_t i = 0; i < data->getNumberOfColumns(); i++) {
            auto c = data->getColumn(i);
            std::string displayName = c->getHeader();
            std::string identifier = util::stripIdentifier(displayName);

            auto prop = [&]() -> ParallelCoordinatesAxisSettingsProperty * {
                if (auto p = axisProperties_.getPropertyByIdentifier(identifier)) {
                    if (auto pcasp = dynamic_cast<ParallelCoordinatesAxisSettingsProperty *>(p)) {
                        return pcasp;
                    }
                    throw inviwo::Exception(
                        "Failed to convert property to ParallelCoordinatesAxisSettingsProperty");
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

            axisVector_.push_back(prop);
            prop->setVisible(true);
            prop->updateFromColumn(c);
        }
        handlePicking_.resize(axisVector_.size() * 2);
    }
}

void ParallelCoordinates::buildLineMesh(
    const std::vector<ParallelCoordinatesAxisSettingsProperty *> &enabledAxis) {
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

    axisPicking_.resize(axisVector_.size());
    linePicking_.resize(numberOfLines);

    auto sampler = tf_.get();

    auto colorAxisId = selectedColorAxis_.get();

    auto colorAxes = axisVector_[glm::clamp(colorAxisId, 0, (int)axisVector_.size() - 1)];

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

        for (auto &axes : enabledAxis) {
            vec3 pos(col++ * dx, axes->getNormalizedAt(i), 0);
            ivVector.push_back(static_cast<glm::uint32_t>(vertices.size()));
            vertices.push_back({pos, pickColor, texCoord, color});
        }
    }
    mesh->addVertices(vertices);

    linesDrawer_ = std::make_unique<MeshDrawerGL>(mesh);
    recreateLines_ = false;
}

void ParallelCoordinates::drawAxis(
    size2_t size, const std::vector<ParallelCoordinatesAxisSettingsProperty *> &enabledAxis) {
    axisShader_.activate();

    axisShader_.setUniform("dims", ivec2(size));
    axisShader_.setUniform("spacing", margins_.getAsVec4());
    axisShader_.setUniform("color", axisColor_.get());
    axisShader_.setUniform("hoverColor", axisHoverColor_.get());
    axisShader_.setUniform("selectedColor", axisSelectedColor_.get());

    float dx = 1.0f / (enabledAxis.size() - 1);

    size_t axisCounter = 0;
    size_t activeAxisCounter = 0;
    for (auto &p : axisVector_) {
        if (p->isChecked()) {
            float x = activeAxisCounter * dx;
            axisShader_.setUniform("x", x);
            axisShader_.setUniform("hover", 0);
            axisShader_.setUniform("selected", 0);
            if (hoveredAxis_ == axisCounter) axisShader_.setUniform("hover", 1);
            if (brushingAndLinking_.isColumnSelected(axisCounter))
                axisShader_.setUniform("selected", 1);

            axisShader_.setUniform("pickColor", axisPicking_.getColor(axisCounter));
            axisDrawer_->draw();
            activeAxisCounter++;
        }
        axisCounter++;
    }

    // Draw axis

    axisShader_.deactivate();
}

void ParallelCoordinates::drawHandles(
    size2_t size, const std::vector<ParallelCoordinatesAxisSettingsProperty *> &enabledAxis) {
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

    float dx = 1.0f / (enabledAxis.size() - 1);
    size_t i = 0;
    for (auto axes : enabledAxis) {
        float x = i++ * dx;
        auto pickingID = axes->columnId_ * 2;

        // lower
        handleShader_.setUniform("color", axes->lowerBrushed_ ? filteredColor : notFilteredColor);
        handleShader_.setUniform("x", x);
        handleShader_.setUniform("w", handleSize_.get().x);
        handleShader_.setUniform("h", handleSize_.get().y);
        handleShader_.setUniform("y", static_cast<float>(axes->getNormalized(axes->range.get().x)));
        handleShader_.setUniform("flipped", 0);
        handleShader_.setUniform("pickColor", handlePicking_.getColor(pickingID + 0));

        handleDrawer_->draw();

        handleShader_.setUniform("color", axes->upperBrushed_ ? filteredColor : notFilteredColor);
        handleShader_.setUniform("y", static_cast<float>(axes->getNormalized(axes->range.get().y)));
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

void ParallelCoordinates::buildTextCache(
    const std::vector<ParallelCoordinatesAxisSettingsProperty *> &enabledAxis) {
    if (textCacheDirty_) {
        textCacheDirty_ = false;

        for (auto &axes : enabledAxis) {
            std::string minV = toString(axes->range.getRange().x);
            std::string maxV = toString(axes->range.getRange().y);

            textRenderer_.setFontSize(fontSize_);
            axes->labelTexture_ =
                util::createTextTexture(textRenderer_, axes->name_, color_, axes->labelTexture_);

            textRenderer_.setFontSize(valuesFontSize_);
            axes->minValTexture_ =
                util::createTextTexture(textRenderer_, minV, color_, axes->minValTexture_);
            axes->maxValTexture_ =
                util::createTextTexture(textRenderer_, maxV, color_, axes->maxValTexture_);
        }
    }
}

void ParallelCoordinates::renderText(
    size2_t outputsize, const std::vector<ParallelCoordinatesAxisSettingsProperty *> &enabledAxis) {
    auto pos = labelPosition_.get();

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const vec2 outputSizeWOMargins(
        static_cast<float>(outputsize.x) - (margins_.getRight() + margins_.getLeft()),
        static_cast<float>(outputsize.y) - (margins_.getTop() + margins_.getBottom()));

    float dx = 1.0f / (enabledAxis.size() - 1);

    size_t maxValHeight = 0;
    size_t maxLabelHeight = 0;
    for (auto axes : enabledAxis) {
        maxValHeight = std::max(maxValHeight, axes->minValTexture_->getHeight());
        maxValHeight = std::max(maxValHeight, axes->maxValTexture_->getHeight());
        maxLabelHeight = std::max(maxLabelHeight, axes->labelTexture_->getHeight());
    }

    if (showValue_) {
        textRenderer_.setFontSize(valuesFontSize_.get());
        // Draw min and max values
        size_t i = 0;

        for (auto axes : enabledAxis) {
            float x = i++ * dx;
            vec2 textPos(0);
            textPos.x = x * outputSizeWOMargins.x + margins_.getLeft() + handleSize_.get().x / 2;

            textPos.y = margins_.getBottom() - handleSize_.get().y;

            textureRenderer_.render(axes->minValTexture_, textPos, outputsize);

            textPos.y = outputsize.y - margins_.getTop() + handleSize_.get().y - maxValHeight;

            textureRenderer_.render(axes->maxValTexture_, ivec2(textPos), outputsize);
        }
    }

    if (pos != LabelPosition::None) {
        size_t i = 0;

        for (auto axes : enabledAxis) {
            float x = i++ * dx;

            vec2 size(axes->labelTexture_->getDimensions());

            vec2 textPos(0);
            textPos.x = x * outputSizeWOMargins.x + margins_.getLeft() - size.x / 2.0f;

            if (pos == LabelPosition::Above) {
                textPos.y = outputsize.y - margins_.getTop() + handleSize_.get().y;
            } else {
                textPos.y = margins_.getBottom() - handleSize_.get().y - maxLabelHeight -
                            1;  // Minus 1 for always being one pixel away from handle
            }

            textureRenderer_.render(axes->labelTexture_, ivec2(textPos), outputsize);
        }
    }
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
        const auto rangeValue =
            axisVector_[axisID]->getValue(upper ? axisVector_[axisID]->range.getRangeMax()
                                                : axisVector_[axisID]->range.getRangeMin());
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

        axisVector_[axisID]->moveHandle(upper, newY);
        p->markAsUsed();
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
