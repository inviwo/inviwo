/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <fancymeshrenderer/processors/fancymeshrenderer.h>

#include <modules/opengl/geometry/meshgl.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/opengl/image/layergl.h>

#include <sstream>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo FancyMeshRenderer::processorInfo_{
    "org.inviwo.FancyMeshRenderer",      // Class identifier
    "Fancy Mesh Renderer",                // Display name
    "Mesh Rendering",              // Category
    CodeState::Experimental,  // Code state
    Tags::GL,               // Tags
};
const ProcessorInfo FancyMeshRenderer::getProcessorInfo() const {
    return processorInfo_;
}

FancyMeshRenderer::FancyMeshRenderer()
    : Processor()
	, inport_("geometry")
	, imageInport_("imageInport")
	, outport_("image")
	, camera_("camera", "Camera", vec3(0.0f, 0.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f), &inport_)
	, centerViewOnGeometry_("centerView", "Center view on geometry")
	, setNearFarPlane_("setNearFarPlane", "Calculate Near and Far Plane")
	, resetViewParams_("resetView", "Reset Camera")
	, trackball_(&camera_)
	, lightingProperty_("lighting", "Lighting", &camera_)
	, layers_("layers", "Output Layers")
	, colorLayer_("colorLayer", "Color", true, InvalidationLevel::InvalidResources)
	, normalsLayer_("normalsLayer", "Normals (World Space)", false,
		InvalidationLevel::InvalidResources)
	, viewNormalsLayer_("viewNormalsLayer", "Normals (View space)", false,
		InvalidationLevel::InvalidResources)
    , forceOpaque_("forceOpaque", "Shade Opaque", false)
    , drawSilhouette_("drawSilhouette", "Draw Silhouette")
	, faceSettings_{true, false}
	, shader_("fancymeshrenderer.vert", "fancymeshrenderer.geom", "fancymeshrenderer.frag", false)
    , depthShader_("geometryrendering.vert", "depthOnly.frag", false)
	, needsRecompilation_(true)
    , originalMesh_(nullptr)
    , meshHasAdjacency_(false)
    , propUseIllustrationBuffer_("illustrationBuffer", "Use Illustration Buffer")
    , debugFragmentLists_(false)
    , propDebugFragmentLists_("debugFL", "Debug Fragment Lists")
{
    //Copied from the standard MeshRenderer

    //input and output ports
	addPort(inport_);
	addPort(imageInport_);
	addPort(outport_);
	imageInport_.setOptional(true);
    outport_.addResizeEventListener(&camera_);
    inport_.onChange(this, &FancyMeshRenderer::updateDrawers);

    //camera, light, 
	addProperty(camera_);
	centerViewOnGeometry_.onChange(this, &FancyMeshRenderer::centerViewOnGeometry);
	addProperty(centerViewOnGeometry_);
	setNearFarPlane_.onChange(this, &FancyMeshRenderer::setNearFarPlane);
	addProperty(setNearFarPlane_);
	resetViewParams_.onChange([this]() { camera_.resetCamera(); });
	addProperty(resetViewParams_);
	addProperty(lightingProperty_);
	addProperty(trackball_);
    camera_.setCollapsed(true);
    lightingProperty_.setCollapsed(true);
    trackball_.setCollapsed(true);

    //New properties
    addProperty(forceOpaque_);
    addProperty(drawSilhouette_);
    addProperty(propDebugFragmentLists_); //DEBUG, to be removed
    addProperty(alphaSettings_.container_);
    addProperty(edgeSettings_.container_);
	addProperty(faceSettings_[0].container_);
	addProperty(faceSettings_[1].container_);
    addProperty(propUseIllustrationBuffer_);
    addProperty(illustrationBufferSettings_.container_);

	//Callbacks
	shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    auto triggerRecompilation = [this]()
    {
        needsRecompilation_ = true;
        update();
    };
    auto triggerUpdate = [this]() {update(); };
    forceOpaque_.onChange(triggerRecompilation);
    drawSilhouette_.onChange([this]()
    {
        needsRecompilation_ = true;
        update();
        updateDrawers();
    });
    alphaSettings_.setCallbacks(triggerUpdate, triggerRecompilation);
    edgeSettings_.setCallbacks(triggerUpdate, triggerRecompilation);
    faceSettings_[0].setCallbacks(triggerUpdate, triggerRecompilation);
    faceSettings_[1].setCallbacks(triggerUpdate, triggerRecompilation);
    faceSettings_[1].frontPart_ = &faceSettings_[0];

    //DEBUG, to be removed
    propDebugFragmentLists_.onChange([this]() {debugFragmentLists_ = true; });

    //Will this be used in any scenario?
    //addProperty(layers_);
    //layers_.addProperty(colorLayer_);
    //layers_.addProperty(normalsLayer_);
    //layers_.addProperty(viewNormalsLayer_);

    //update visibility of properties
    update();

    //Compile depth-only shader
    //Why this is needed, see the end of process()
    depthShader_.build();
}

FancyMeshRenderer::AlphaSettings::AlphaSettings()
    : container_("alphaContainer", "Alpha")
    , enableUniform_("alphaUniform", "Uniform", true)
    , uniformScaling_("alphaUniformScaling", "Scaling", 0.5, 0, 1, 0.01)
    , enableAngleBased_("alphaAngleBased", "Angle-based", false)
    , angleBasedExponent_("alphaAngleBasedExponent", "Exponent", 1, 0, 5, 0.01)
    , enableNormalVariation_("alphaNormalVariation", "Normal variation", false)
    , normalVariationExponent_("alphaNormalVariationExponent", "Exponent", 1, 0, 5, 0.01)
    , enableDensity_("alphaDensity", "Density-based", false)
    , baseDensity_("alphaBaseDensity", "Base density", 1, 0, 2, 0.01)
    , densityExponent_("alphaDensityExponent", "Exponent", 1, 0, 5, 0.01)
    , enableShape_("alphaShape", "Shape-based", false)
    , shapeExponent_("alphaShapeExponent", "Exponent", 1, 0, 5, 0.01)
{
    container_.addProperty(enableUniform_);
    container_.addProperty(uniformScaling_);
    container_.addProperty(enableAngleBased_);
    container_.addProperty(angleBasedExponent_);
    container_.addProperty(enableNormalVariation_);
    container_.addProperty(normalVariationExponent_);
    container_.addProperty(enableDensity_);
    container_.addProperty(baseDensity_);
    container_.addProperty(densityExponent_);
    container_.addProperty(enableShape_);
    container_.addProperty(shapeExponent_);
}

void FancyMeshRenderer::AlphaSettings::setCallbacks(const std::function<void()>& triggerUpdate, const std::function<void()>& triggerRecompilation)
{
    enableUniform_.onChange(triggerRecompilation);
    enableAngleBased_.onChange(triggerRecompilation);
    enableNormalVariation_.onChange(triggerRecompilation);
    enableDensity_.onChange(triggerRecompilation);
    enableShape_.onChange(triggerRecompilation);
}

void FancyMeshRenderer::AlphaSettings::update()
{
    uniformScaling_.setVisible(enableUniform_.get());
    angleBasedExponent_.setVisible(enableAngleBased_.get());
    normalVariationExponent_.setVisible(enableNormalVariation_.get());
    baseDensity_.setVisible(enableDensity_.get());
    densityExponent_.setVisible(enableDensity_.get());
    shapeExponent_.setVisible(enableShape_.get());
}

FancyMeshRenderer::EdgeSettings::EdgeSettings()
    : container_("edges", "Edges")
    , edgeThickness_("edgesThickness", "Thickness", 2, 0.1, 10, 0.1)
    , depthDependent_("edgesDepth", "Depth dependent", false)
    , smoothEdges_("edgesSmooth", "Smooth edges", true)
{
    container_.addProperty(edgeThickness_);
    container_.addProperty(depthDependent_);
    container_.addProperty(smoothEdges_);
}

void FancyMeshRenderer::EdgeSettings::setCallbacks(const std::function<void()>& triggerUpdate, const std::function<void()>& triggerRecompilation)
{
    depthDependent_.onChange(triggerRecompilation);
    smoothEdges_.onChange(triggerRecompilation);
}

void FancyMeshRenderer::EdgeSettings::update()
{
    //do nothing
}

FancyMeshRenderer::HatchingSettings::HatchingSettings(const std::string& prefix)
    : mode_(prefix + "hatchingMode", "Hatching")
    , container_(prefix + "hatchingContainer", "Hatching Settings")
    , steepness_(prefix + "hatchingSteepness", "Steepness", 5, 1, 10)
    , baseFrequencyU_(prefix + "hatchingFrequencyU", "U-Frequency", 3, 1, 10)
    , baseFrequencyV_(prefix + "hatchingFrequencyV", "V-Frequency", 3, 1, 10)
    , color_(prefix + "hatchingColor", "Color", {0,0,0,1})
    , blendingMode_(prefix + "hatchingBlending", "Blending")
{
    //init properties
    mode_.addOption("off", "Off", HatchingMode::Off);
    mode_.addOption("u", "U", HatchingMode::U);
    mode_.addOption("v", "V", HatchingMode::V);
    mode_.addOption("uv", "UV", HatchingMode::UV);
    mode_.set(HatchingMode::Off);
    mode_.setCurrentStateAsDefault();
    color_.setSemantics(PropertySemantics::Color);
    blendingMode_.addOption("mult", "Multiplicative", HatchingBlendingMode::Multiplicative);
    blendingMode_.addOption("add", "Additive", HatchingBlendingMode::Additive);

    //add to container
    container_.addProperty(steepness_);
    container_.addProperty(baseFrequencyU_);
    container_.addProperty(baseFrequencyV_);
    container_.addProperty(color_);
    container_.addProperty(blendingMode_);
}

FancyMeshRenderer::FaceRenderSettings::FaceRenderSettings(bool frontFace)
    : frontFace_(frontFace)
    , prefix_(frontFace ? "front" : "back")
    , container_(prefix_ + "container", frontFace ? "Front Face" : "Back Face")
    , show_(prefix_ + "show", "Show", true)
    , sameAsFrontFace_(prefix_ + "same", "Same as Front Face")
    , copyFrontToBack_(prefix_ + "copy", "Copy Front to Back")
    , transferFunction_(prefix_ + "tf", "Transfer Function")
    , externalColor_(prefix_ + "extraColor", "Color", { 1, 0.3, 0.01, 1 })
    , colorSource_(prefix_ + "colorSource", "Color Source")
    , separateUniformAlpha_(prefix_ + "separateUniformAlpha", "Separate Uniform Alpha")
    , uniformAlpha_(prefix_ + "uniformAlpha", "Uniform Alpha", 0.5, 0, 1, 0.01)
    , normalSource_(prefix_ + "normalSource", "Normal Source")
    , shadingMode_(prefix_ + "shadingMode", "Shading Mode")
    , showEdges_(prefix_ + "showEdges", "Show Edges")
    , edgeColor_(prefix_ + "edgeColor", "Edge color", { 0, 0, 0, 1 })
    , edgeOpacity_(prefix_ + "edgeOpacity", "Edge Opacity", 0.5, 0, 2, 0.01)
    , hatching_(prefix_)
{
    //initialize combo boxes
	colorSource_.addOption("vertexColor", "VertexColor", ColorSource::VertexColor);
	colorSource_.addOption("tf", "Transfer Function", ColorSource::TransferFunction);
	colorSource_.addOption("external", "Constant Color", ColorSource::ExternalColor);
	colorSource_.set(ColorSource::ExternalColor);
	colorSource_.setCurrentStateAsDefault();
	externalColor_.setSemantics(PropertySemantics::Color);

	normalSource_.addOption("inputVertex", "Input: Vertex", NormalSource::InputVertex);
	normalSource_.addOption("inputTriangle", "Input: Triangle", NormalSource::InputTriangle);
	normalSource_.addOption("generateVertex", "Generate: Vertex", NormalSource::GenerateVertex);
	normalSource_.addOption("generateTriangle", "Generate: Triangle", NormalSource::GenerateTriangle);
	normalSource_.set(NormalSource::InputVertex);
	normalSource_.setCurrentStateAsDefault();

	shadingMode_.addOption("off", "Off", ShadingMode::Off);
	shadingMode_.addOption("phong", "Phong", ShadingMode::Phong);
	shadingMode_.addOption("pbr", "PBR", ShadingMode::Pbr);
	shadingMode_.set(ShadingMode::Off);
	shadingMode_.setCurrentStateAsDefault();

    edgeColor_.setSemantics(PropertySemantics::Color);

    //layouting, add the properties
	container_.addProperty(show_);
    if (!frontFace)
    {
        container_.addProperty(sameAsFrontFace_);
        container_.addProperty(copyFrontToBack_);
        copyFrontToBack_.onChange([this]() {copyFrontToBack(); });
    }
	container_.addProperty(colorSource_);
	container_.addProperty(transferFunction_);
	container_.addProperty(externalColor_);
	container_.addProperty(separateUniformAlpha_);
	container_.addProperty(uniformAlpha_);
	container_.addProperty(normalSource_);
	container_.addProperty(shadingMode_);
    container_.addProperty(showEdges_);
    container_.addProperty(edgeColor_);
    container_.addProperty(edgeOpacity_);
    container_.addProperty(hatching_.mode_);
    container_.addProperty(hatching_.container_);

    //set callbacks that will trigger update()
    auto triggerUpdate = [this]() {update(lastOpaque_); };
    show_.onChange(triggerUpdate);
    sameAsFrontFace_.onChange(triggerUpdate);
    colorSource_.onChange(triggerUpdate);
    separateUniformAlpha_.onChange(triggerUpdate);
    normalSource_.onChange(triggerUpdate);
    hatching_.mode_.onChange(triggerUpdate);
    hatching_.steepness_.onChange(triggerUpdate);
    hatching_.baseFrequencyU_.onChange(triggerUpdate);
    hatching_.baseFrequencyV_.onChange(triggerUpdate);
    hatching_.color_.onChange(triggerUpdate);
    hatching_.blendingMode_.onChange(triggerUpdate);
}

void FancyMeshRenderer::FaceRenderSettings::copyFrontToBack()
{
    transferFunction_.set(frontPart_->transferFunction_.get());
    externalColor_.set(frontPart_->externalColor_.get());
    externalColor_.set(frontPart_->externalColor_.get());
    colorSource_.set(frontPart_->colorSource_.get());
    separateUniformAlpha_.set(frontPart_->separateUniformAlpha_.get());
    uniformAlpha_.set(frontPart_->uniformAlpha_.get());
    normalSource_.set(frontPart_->normalSource_.get());
    shadingMode_.set(frontPart_->shadingMode_.get());
    showEdges_.set(frontPart_->showEdges_.get());
    edgeColor_.set(frontPart_->edgeColor_.get());
    edgeOpacity_.set(frontPart_->edgeOpacity_.get());
    hatching_.mode_.set(frontPart_->hatching_.mode_.get());
    hatching_.steepness_.set(frontPart_->hatching_.steepness_.get());
    hatching_.baseFrequencyU_.set(frontPart_->hatching_.baseFrequencyU_.get());
    hatching_.baseFrequencyV_.set(frontPart_->hatching_.baseFrequencyV_.get());
    hatching_.color_.set(frontPart_->hatching_.color_.get());
    hatching_.blendingMode_.set(frontPart_->hatching_.blendingMode_.get());
}

void FancyMeshRenderer::FaceRenderSettings::update(bool opaque)
{
    lastOpaque_ = opaque;
    //fetch properties
    bool show = show_.get();
    bool show2 = show && !sameAsFrontFace_.get();
    ColorSource colorSource = colorSource_.get();
    bool separateUniformAlpha = separateUniformAlpha_.get();
    bool showEdges = showEdges_.get();
    bool hatching = hatching_.mode_.get() != HatchingMode::Off;

    //set visibility
    sameAsFrontFace_.setVisible(show);
    copyFrontToBack_.setVisible(show);
    colorSource_.setVisible(show2);
    transferFunction_.setVisible(show2 && colorSource == ColorSource::TransferFunction);
    externalColor_.setVisible(show2 && colorSource == ColorSource::ExternalColor);
    separateUniformAlpha_.setVisible(show2 && !opaque);
    uniformAlpha_.setVisible(show2 && !opaque && separateUniformAlpha);
    normalSource_.setVisible(show2);
    shadingMode_.setVisible(show2);
    showEdges_.setVisible(show2);
    edgeColor_.setVisible(show2 && showEdges);
    edgeOpacity_.setVisible(show2 && showEdges);
    hatching_.mode_.setVisible(show2);
    hatching_.container_.setVisible(show2 && hatching);
    hatching_.baseFrequencyU_.setVisible(hatching_.mode_.get() != HatchingMode::V);
    hatching_.baseFrequencyV_.setVisible(hatching_.mode_.get() != HatchingMode::U);
}

void FancyMeshRenderer::FaceRenderSettings::setCallbacks(const std::function<void()>& triggerUpdate, const std::function<void()>& triggerRecompilation)
{
    showEdges_.onChange(triggerRecompilation);
    colorSource_.onChange(triggerRecompilation);
    hatching_.mode_.onChange(triggerRecompilation);
}

FancyMeshRenderer::IllustrationBufferSettings::IllustrationBufferSettings()
    : container_("illustrationBufferContainer", "Illustration Buffer Settings")
    , edgeColor_("illustrationBufferEdgeColor", "Edge Color", vec3(0, 0, 0))
    , edgeStrength_("illustrationBufferEdgeStrength", "Edge Strength", 0.5, 0, 1, 0.01)
    , haloStrength_("illustrationBufferHaloStrength", "Halo Strength", 0.5, 0, 1, 0.01)
    , smoothingSteps_("illustrationBufferSmoothingSteps", "Smoothing Steps", 3, 0, 50, 1)
    , edgeSmoothing_("illustrationBufferEdgeSmoothing", "Edge Smoothing", 0.8, 0, 1, 0.01)
    , haloSmoothing_("illustrationBufferHaloSmoothing", "Halo Smoothing", 0.8, 0, 1, 0.01)
{
    edgeColor_.setSemantics(PropertySemantics::Color);
    container_.addProperty(edgeColor_);
    container_.addProperty(edgeStrength_);
    container_.addProperty(haloStrength_);
    container_.addProperty(smoothingSteps_);
    container_.addProperty(edgeSmoothing_);
    container_.addProperty(haloSmoothing_);
}

void FancyMeshRenderer::initializeResources() {
	
	//get number of layers, see compileShader()
	// first two layers (color and picking) are reserved
	int layerID = 2;
	if (normalsLayer_.get()) {
		++layerID;
	}
	if (viewNormalsLayer_.get()) {
		++layerID;
	}

	// get a hold of the current output data
	auto prevData = outport_.getData();
	auto numLayers = static_cast<std::size_t>(layerID - 1); // Don't count picking
	if (prevData->getNumberOfColorLayers() != numLayers) {
		// create new image with matching number of layers
		auto image = std::make_shared<Image>(prevData->getDimensions(), prevData->getDataFormat());
		// update number of layers
		for (auto i = image->getNumberOfColorLayers(); i < numLayers; ++i) {
			image->addColorLayer(std::shared_ptr<Layer>(image->getColorLayer(0)->clone()));
		}

		outport_.setData(image);
	}

    if (!colorLayer_.get())
    {
        LogProcessorWarn("requesting alpha blending, but no color layer attached -> no output will be produced");
    }
}

void FancyMeshRenderer::update()
{
    //fetch all booleans
    bool opaque = forceOpaque_.get();

    //set top-level visibility
    alphaSettings_.container_.setVisible(!opaque);
    edgeSettings_.container_.setVisible(drawSilhouette_.get() ||
        faceSettings_[0].showEdges_.get() || faceSettings_[1].showEdges_.get());

    //update nested settings
    alphaSettings_.update();
    edgeSettings_.update();
    faceSettings_[0].update(opaque);
    faceSettings_[1].update(opaque);
    propUseIllustrationBuffer_.setVisible(!opaque);
    illustrationBufferSettings_.container_.setVisible(!opaque && propUseIllustrationBuffer_.get());
}

void FancyMeshRenderer::compileShader()
{
	if (!needsRecompilation_) return;

	// shading defines
	utilgl::addShaderDefines(shader_, lightingProperty_);

	if (colorLayer_.get()) {
		shader_.getFragmentShaderObject()->addShaderDefine("COLOR_LAYER");
	}
	else {
		shader_.getFragmentShaderObject()->removeShaderDefine("COLOR_LAYER");
	}

	// first two layers (color and picking) are reserved
	int layerID = 2;
	if (normalsLayer_.get()) {
		shader_.getFragmentShaderObject()->addShaderDefine("NORMALS_LAYER");
		shader_.getFragmentShaderObject()->addOutDeclaration("normals_out", layerID);
		++layerID;
	}
	else {
		shader_.getFragmentShaderObject()->removeShaderDefine("NORMALS_LAYER");
	}
	if (viewNormalsLayer_.get()) {
		shader_.getFragmentShaderObject()->addShaderDefine("VIEW_NORMALS_LAYER");
		shader_.getFragmentShaderObject()->addOutDeclaration("view_normals_out", layerID);
		++layerID;
	}
	else {
		shader_.getFragmentShaderObject()->removeShaderDefine("VIEW_NORMALS_LAYER");
	}

	//Settings
    if (forceOpaque_) {
        shader_.getFragmentShaderObject()->removeShaderDefine("USE_FRAGMENT_LIST");
    } 
    else {
        shader_.getFragmentShaderObject()->addShaderDefine("USE_FRAGMENT_LIST");
        if (!colorLayer_.get())
        {
            LogProcessorWarn("requesting alpha blending, but no color layer attached -> no output will be produced");
        }
    }

    //helper function that sets shader defines based on a boolean property
    auto SendBoolean = [this] (bool flag, const std::string& define)
	{
	    if (flag)
	    {
            this->shader_.getFragmentShaderObject()->addShaderDefine(define);
            this->shader_.getGeometryShaderObject()->addShaderDefine(define);
            this->shader_.getVertexShaderObject()->addShaderDefine(define);
	    } else
	    {
            this->shader_.getFragmentShaderObject()->removeShaderDefine(define);
            this->shader_.getGeometryShaderObject()->removeShaderDefine(define);
            this->shader_.getVertexShaderObject()->removeShaderDefine(define);
	    }
    };
    SendBoolean(alphaSettings_.enableUniform_.get(), "ALPHA_UNIFORM");
    SendBoolean(alphaSettings_.enableAngleBased_.get(), "ALPHA_ANGLE_BASED");
    SendBoolean(alphaSettings_.enableNormalVariation_.get(), "ALPHA_NORMAL_VARIATION");
    SendBoolean(alphaSettings_.enableDensity_.get(), "ALPHA_DENSITY");
    SendBoolean(alphaSettings_.enableShape_.get(), "ALPHA_SHAPE");
    SendBoolean(faceSettings_[0].showEdges_.get() || faceSettings_[1].showEdges_.get(), "DRAW_EDGES");
    SendBoolean(edgeSettings_.depthDependent_.get(), "DRAW_EDGES_DEPTH_DEPENDENT");
    SendBoolean(edgeSettings_.smoothEdges_.get(), "DRAW_EDGES_SMOOTHING");
    SendBoolean(meshHasAdjacency_, "MESH_HAS_ADJACENCY");
    SendBoolean(drawSilhouette_, "DRAW_SILHOUETTE");
    SendBoolean(faceSettings_[0].hatching_.mode_.get() != HatchingMode::Off
        || faceSettings_[1].hatching_.mode_.get() != HatchingMode::Off, "SEND_TEX_COORD");
    SendBoolean(faceSettings_[0].colorSource_.get() == ColorSource::TransferFunction
        || faceSettings_[1].colorSource_.get() == ColorSource::TransferFunction, "SEND_SCALAR");
    SendBoolean(faceSettings_[0].colorSource_.get() == ColorSource::VertexColor
        || faceSettings_[1].colorSource_.get() == ColorSource::VertexColor, "SEND_COLOR");
	shader_.build();

	LogProcessorInfo("shader compiled");
	needsRecompilation_ = false;
}

void FancyMeshRenderer::process() {
    //I have to call update here, otherwise, when you load a saved workspace,
    //the visibility of the properties is not updated on startup.
    update();

	if (imageInport_.isConnected()) {
		utilgl::activateTargetAndCopySource(outport_, imageInport_);
	}
	else {
		utilgl::activateAndClearTarget(outport_);
	}

    if (!faceSettings_[0].show_ && !faceSettings_[1].show_)
    {
        utilgl::deactivateCurrentTarget();
        return; //everything is culled
    }
    bool opaque = forceOpaque_.get();
    bool fragmentLists = !opaque; // or maybe add an option to use regular alpha blending
    if (fragmentLists && !colorLayer_.get())
    {
        //fragment lists can only render to color layer, but no color layer is available
        return;
    }

    //Loop: fragment list may need another try if not enough space for the pixels was available
    bool retry;
    do {
        retry = false;

        if (fragmentLists)
        {
            //prepare fragment list rendering
            flr_.prePass(outport_.getDimensions());
        }

        compileShader();
        shader_.activate();

        //various OpenGL states: depth, blending, culling
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, opaque);
        utilgl::DepthMaskState depthMask(opaque ? GL_TRUE : GL_FALSE);
        utilgl::CullFaceState culling(
            !faceSettings_[0].show_ && faceSettings_[1].show_ ? GL_FRONT :
            faceSettings_[0].show_ && !faceSettings_[1].show_ ? GL_BACK :
            GL_NONE);
        utilgl::BlendModeState blendModeStateGL(
            opaque ? GL_ONE : GL_SRC_ALPHA,
            opaque ? GL_ZERO : GL_ONE_MINUS_SRC_ALPHA);

        //general settings for camera, lighting, picking, mesh data
        utilgl::setUniforms(shader_, camera_, lightingProperty_);
        utilgl::setShaderUniforms(shader_, *(drawer_->getMesh()), "geometry");
        shader_.setUniform("pickingEnabled", meshutil::hasPickIDBuffer(drawer_->getMesh()));
        shader_.setUniform("halfScreenSize", ivec2(outport_.getDimensions()) / ivec2(2));

        //update face render settings
        TextureUnit transFuncUnit[2];
        for (int j = 0; j < 2; ++j)
        {
            int i = j;
            if (j==1 && faceSettings_[1].sameAsFrontFace_.get())
            { //use settings from font face also for back face
                i = 0;
            }

            std::stringstream ss;
            ss << "renderSettings[" << j << "].";
            std::string prefix = ss.str();
            shader_.setUniform(prefix + "externalColor", faceSettings_[i].externalColor_.get());
            shader_.setUniform(prefix + "colorSource", static_cast<int>(faceSettings_[i].colorSource_.get()));
            shader_.setUniform(prefix + "separateUniformAlpha", faceSettings_[i].separateUniformAlpha_.get());
            shader_.setUniform(prefix + "uniformAlpha", faceSettings_[i].uniformAlpha_.get());
            shader_.setUniform(prefix + "normalSource", static_cast<int>(faceSettings_[i].normalSource_.get()));
            shader_.setUniform(prefix + "shadingMode", static_cast<int>(faceSettings_[i].shadingMode_.get()));
            shader_.setUniform(prefix + "showEdges", faceSettings_[i].showEdges_.get());
            shader_.setUniform(prefix + "edgeColor", faceSettings_[i].edgeColor_.get());
            shader_.setUniform(prefix + "edgeOpacity", faceSettings_[i].edgeOpacity_.get());
            shader_.setUniform(prefix + "hatchingMode", static_cast<int>(faceSettings_[i].hatching_.mode_.get()));
            shader_.setUniform(prefix + "hatchingSteepness", faceSettings_[i].hatching_.steepness_.get());
            shader_.setUniform(prefix + "hatchingFreqU", faceSettings_[i].hatching_.baseFrequencyU_.getMaxValue() - faceSettings_[i].hatching_.baseFrequencyU_.get());
            shader_.setUniform(prefix + "hatchingFreqV", faceSettings_[i].hatching_.baseFrequencyV_.getMaxValue() - faceSettings_[i].hatching_.baseFrequencyV_.get());
            shader_.setUniform(prefix + "hatchingColor", faceSettings_[i].hatching_.color_.get());
            shader_.setUniform(prefix + "hatchingBlending", static_cast<int>(faceSettings_[i].hatching_.blendingMode_.get()));

            const auto& tf = faceSettings_[i].transferFunction_.get();
            const Layer* tfLayer = tf.getData();
            const LayerGL* transferFunctionGL = tfLayer->getRepresentation<LayerGL>();
            transferFunctionGL->bindTexture(transFuncUnit[j].getEnum());
            ss = std::stringstream();
            ss << "transferFunction" << j;
            shader_.setUniform(ss.str(), transFuncUnit[j].getUnitNumber());
        }

        //update alpha settings
        shader_.setUniform("alphaSettings.uniformScale", alphaSettings_.uniformScaling_.get());
        shader_.setUniform("alphaSettings.angleExp", alphaSettings_.angleBasedExponent_.get());
        shader_.setUniform("alphaSettings.normalExp", alphaSettings_.normalVariationExponent_.get());
        shader_.setUniform("alphaSettings.baseDensity", alphaSettings_.baseDensity_.get());
        shader_.setUniform("alphaSettings.densityExp", alphaSettings_.densityExponent_.get());
        shader_.setUniform("alphaSettings.shapeExp", alphaSettings_.shapeExponent_.get());

        //update geometry shader settings
        shader_.setUniform("geomSettings.edgeWidth", edgeSettings_.edgeThickness_.get());

        if (fragmentLists)
        {
            //set uniforms fragment list rendering
            flr_.setShaderUniforms(shader_);
        }

        //Finally, draw it
        drawer_->draw();

        shader_.deactivate();

        if (fragmentLists)
        {
            //final processing of fragment list rendering
            if (propUseIllustrationBuffer_.get())
            {
                FragmentListRenderer::IllustrationBufferSettings settings;
                settings.edgeColor_ = illustrationBufferSettings_.edgeColor_.get();
                settings.edgeStrength_ = illustrationBufferSettings_.edgeStrength_.get();
                settings.haloStrength_ = illustrationBufferSettings_.haloStrength_.get();
                settings.smoothingSteps_ = illustrationBufferSettings_.smoothingSteps_.get();
                settings.edgeSmoothing_ = illustrationBufferSettings_.edgeSmoothing_.get();
                settings.haloSmoothing_ = illustrationBufferSettings_.haloSmoothing_.get();
                flr_.setIllustrationBufferSettings(settings);
            }
            bool success = flr_.postPass(propUseIllustrationBuffer_.get(), debugFragmentLists_);
            debugFragmentLists_ = false;
            if (!success) {
                retry = true;
            }
        }

    } while (retry);

    //Workaround for a problem with the fragment lists:
    //The camera interaction requires the depth buffer for some reason to work,
    // otherwise, the rotation does not work.
    //My first idea was to set the depth in the fragment list's 'dispABufferLinkedList.frag'
    // using gl_FragDepth, but this don't work (yet).
    if (fragmentLists)
    {
        depthShader_.activate();
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        utilgl::DepthMaskState depthMask(GL_TRUE);
        utilgl::CullFaceState culling(
            !faceSettings_[0].show_ && faceSettings_[1].show_ ? GL_FRONT :
            faceSettings_[0].show_ && !faceSettings_[1].show_ ? GL_BACK :
            GL_NONE);
        utilgl::BlendModeState blendModeStateGL(GL_ZERO, GL_ZERO);
        utilgl::setUniforms(depthShader_, camera_);
        utilgl::setShaderUniforms(depthShader_, *(drawer_->getMesh()), "geometry");
        drawer_->draw();
        depthShader_.deactivate();
    }

	utilgl::deactivateCurrentTarget();
}

void FancyMeshRenderer::centerViewOnGeometry()
{
	if (!inport_.hasData()) return;

	auto minmax = calcWorldBoundingBox();
	camera_.setLook(camera_.getLookFrom(), 0.5f * (minmax.first + minmax.second),
		camera_.getLookUp());
}

std::pair<vec3, vec3> FancyMeshRenderer::calcWorldBoundingBox() const
{
	vec3 worldMin(std::numeric_limits<float>::max());
	vec3 worldMax(std::numeric_limits<float>::lowest());
	const auto& mesh = inport_.getData();
	const auto& buffers = mesh->getBuffers();
	auto it = std::find_if(buffers.begin(), buffers.end(), [](const auto& buff) {
		return buff.first.type == BufferType::PositionAttrib;
	});
	if (it != buffers.end()) {
		auto minmax = util::bufferMinMax(it->second.get());

		mat4 trans = mesh->getCoordinateTransformer().getDataToWorldMatrix();
		worldMin = glm::min(worldMin, vec3(trans * vec4(vec3(minmax.first), 1.f)));
		worldMax = glm::max(worldMax, vec3(trans * vec4(vec3(minmax.second), 1.f)));
	}
	return{ worldMin, worldMax };
}

void FancyMeshRenderer::setNearFarPlane()
{
	if (!inport_.hasData()) return;

	auto geom = inport_.getData();

	auto posBuffer =
		dynamic_cast<const Vec3BufferRAM*>(geom->getBuffer(0)->getRepresentation<BufferRAM>());

	if (posBuffer == nullptr) return;

	auto pos = posBuffer->getDataContainer();

	if (pos.empty()) return;

	float nearDist = std::numeric_limits<float>::infinity();
	float farDist = 0;
	vec3 nearPos;
	vec3 farPos;
	const vec3 camPos{ geom->getCoordinateTransformer().getWorldToModelMatrix() *
		vec4(camera_.getLookFrom(), 1.0) };
	for (auto& po : pos) {
		auto d = glm::distance2(po, camPos);
		if (d < nearDist) {
			nearDist = d;
			nearPos = po;
		}
		if (d > farDist) {
			farDist = d;
			farPos = po;
		}
	}

	mat4 m = camera_.viewMatrix() * geom->getCoordinateTransformer().getModelToWorldMatrix();

	camera_.setNearPlaneDist(std::max(0.0f, 0.5f * std::abs((m * vec4(nearPos, 1.0f)).z)));
	camera_.setFarPlaneDist(std::max(0.0f, 2.0f * std::abs((m * vec4(farPos, 1.0f)).z)));
}

void FancyMeshRenderer::updateDrawers()
{
    //sometimes, this is null:
    if (getNetwork() == nullptr) return;
    //aquire mesh
	auto changed = inport_.getChangedOutports();
	auto factory = getNetwork()->getApplication()->getMeshDrawerFactory();
    const Mesh* mesh = inport_.getData().get();
    if (mesh != originalMesh_)
    {
        //Delete old mesh
        enhancedMesh_.reset();
        halfEdges_.reset();
    }
    originalMesh_ = mesh;

    //check if we need to preprocess it
    if (drawSilhouette_.get())
    {
        if (mesh->getNumberOfIndicies() != 1)
        {
            LogProcessorWarn("Only meshes with exactly one index buffer are supported for adjacency information");
        }
        //create adjacency information
        if (halfEdges_ == nullptr) {
            halfEdges_ = std::make_unique<HalfEdges>(mesh->getIndices(0));

            //enhancedMesh_ = std::unique_ptr<Mesh>(mesh->clone());
            //while (enhancedMesh_->getNumberOfIndicies() > 0) enhancedMesh_->removeIndices(0);

            //duplication of mesh->clone() that does not include the index buffer
            //enhancedMesh_ = std::make_unique<Mesh>(mesh->getDefaultMeshInfo().dt, mesh->getDefaultMeshInfo().ct);
            enhancedMesh_ = std::make_unique<Mesh>(DrawType::Triangles, ConnectivityType::Adjacency); //we directly force the new mesh type
            for (const auto& elem : mesh->getBuffers()) {
                enhancedMesh_->addBuffer(elem.first, std::shared_ptr<BufferBase>(elem.second->clone()));
            }

            //add new index buffer with adjacency information
            enhancedMesh_->addIndicies({ DrawType::Triangles, ConnectivityType::Adjacency },
                halfEdges_->createIndexBufferWithAdjacency());
            LogProcessorInfo("Adjacency information created");
        }
        //send to drawer
        meshHasAdjacency_ = true;
        LogProcessorInfo("draw mesh with adjacency information");
        drawer_ = factory->create(enhancedMesh_.get());
    }
    else {
        //normal mode
        meshHasAdjacency_ = false;
        LogProcessorInfo("draw mesh without adjacency information");
        drawer_ = factory->create(originalMesh_);
    }
    //trigger shader recompilation
    //Geometry shader needs to know if it has adjacency or not
    needsRecompilation_ = true;
}

} // namespace

