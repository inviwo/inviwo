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
	, separateFaceSettings_("separateFaceSettings", "Separate Face Settings", false)
	, copyFrontToBack_("copyFrontToBack", "Copy Front to Back")
    , forceOpaque_("forceOpaque", "Force Opaque", false)
	, faceSettings_{"front_", "back_"}
	, shader_("fancymeshrenderer.vert", "fancymeshrenderer.frag", false)
	, needsRecompilation_(true)
{
	addPort(inport_);
	addPort(imageInport_);
	addPort(outport_);

	imageInport_.setOptional(true);

	addProperty(camera_);
	centerViewOnGeometry_.onChange(this, &FancyMeshRenderer::centerViewOnGeometry);
	addProperty(centerViewOnGeometry_);
	setNearFarPlane_.onChange(this, &FancyMeshRenderer::setNearFarPlane);
	addProperty(setNearFarPlane_);
	resetViewParams_.onChange([this]() { camera_.resetCamera(); });
	addProperty(resetViewParams_);
	outport_.addResizeEventListener(&camera_);
	inport_.onChange(this, &FancyMeshRenderer::updateDrawers);

	addProperty(lightingProperty_);
	addProperty(trackball_);

	copyFrontToBack_.onChange(this, &FancyMeshRenderer::copyFrontToBackSettings);
	faceSettings_[0].container_.setDisplayName("Front Face");
	faceSettings_[1].container_.setDisplayName("Back Face");
	faceSettings_[1].container_.setCollapsed(true);
	addProperty(separateFaceSettings_);
	addProperty(copyFrontToBack_);
    addProperty(forceOpaque_);
	addProperty(faceSettings_[0].container_);
	addProperty(faceSettings_[1].container_);

	addProperty(layers_);
	layers_.addProperty(colorLayer_);
	layers_.addProperty(normalsLayer_);
	layers_.addProperty(viewNormalsLayer_);

	camera_.setCollapsed(true);
	lightingProperty_.setCollapsed(true);
	trackball_.setCollapsed(true);

	shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}
FancyMeshRenderer::FaceRenderSettings::FaceRenderSettings(const std::string& prefix)
	: container_(prefix + "container", "Foo")
	, cull_(prefix + "cull", "Cull", false)
	, transferFunction_(prefix + "tf", "Transfer Function")
	, externalColor_(prefix + "extraColor", "Color Overwrite")
	, colorSource_(prefix + "colorSource", "Color Source")
	, alphaMode_(prefix + "alphaMode", "Alpha Mode")
	, alphaScale_(prefix + "alphaScale", "Alpha Scale", 1, 0, 10)
	, normalSource_(prefix + "normalSource", "Normal Source")
	, shadingMode_(prefix + "shadingMode", "Shading Mode")
{
	colorSource_.addOption("vertexColor", "VertexColor", ColorSource::VertexColor);
	colorSource_.addOption("tf", "Transfer Function", ColorSource::TransferFunction);
	colorSource_.addOption("external", "Color Overwrite", ColorSource::ExternalColor);
	colorSource_.set(ColorSource::ExternalColor);
	colorSource_.setCurrentStateAsDefault();
	externalColor_.setSemantics(PropertySemantics::Color);

	alphaMode_.addOption("uniform", "Uniform", AlphaMode::Uniform);
	alphaMode_.addOption("angle", "Angle-Based", AlphaMode::AngleBased);
	alphaMode_.addOption("normal", "Normal-Based", AlphaMode::NormalBased);
	alphaMode_.set(AlphaMode::Uniform);
	alphaMode_.setCurrentStateAsDefault();

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

	container_.addProperty(cull_);
	container_.addProperty(colorSource_);
	container_.addProperty(transferFunction_);
	container_.addProperty(externalColor_);
	container_.addProperty(alphaMode_);
	container_.addProperty(alphaScale_);
	container_.addProperty(normalSource_);
	container_.addProperty(shadingMode_);
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
	shader_.getFragmentShaderObject()->addShaderDefine("OVERRIDE_COLOR_BUFFER");
	//TODO: more settings

	shader_.build();

	LogProcessorInfo("shader compiled");
	needsRecompilation_ = false;
}

void FancyMeshRenderer::process() {
	if (imageInport_.isConnected()) {
		utilgl::activateTargetAndCopySource(outport_, imageInport_);
	}
	else {
		utilgl::activateAndClearTarget(outport_);
	}

    if (faceSettings_[0].cull_ && faceSettings_[0].cull_)
    {
        utilgl::deactivateCurrentTarget();
        return; //everything is culled
    }

	compileShader();
	shader_.activate();

    bool opaque = forceOpaque_.get();

	utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
	utilgl::DepthMaskState depthMask(opaque ? GL_TRUE : GL_FALSE);
	utilgl::CullFaceState culling(
		faceSettings_[0].cull_ && !faceSettings_[1].cull_ ? GL_FRONT :
		!faceSettings_[0].cull_ && faceSettings_[1].cull_ ? GL_BACK :
		GL_NONE);
    utilgl::BlendModeState blendModeStateGL(
        opaque ? GL_ONE : GL_SRC_ALPHA,
        opaque ? GL_ZERO : GL_ONE_MINUS_SRC_ALPHA);

	utilgl::setUniforms(shader_, camera_, lightingProperty_);
	utilgl::setShaderUniforms(shader_, *(drawer_->getMesh()), "geometry");
	shader_.setUniform("overrideColor", faceSettings_[0].externalColor_.get());
	shader_.setUniform("pickingEnabled", meshutil::hasPickIDBuffer(drawer_->getMesh()));

    //update face render settings
    shader_.setUniform("frontSettings.alphaScale", faceSettings_[0].alphaScale_.get());

	//Finally, draw it
	drawer_->draw();

	shader_.deactivate();
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
	auto changed = inport_.getChangedOutports();
	auto factory = getNetwork()->getApplication()->getMeshDrawerFactory();
	drawer_ = factory->create(inport_.getData().get());
}

void FancyMeshRenderer::copyFrontToBackSettings()
{
	//TODO:
}
} // namespace

