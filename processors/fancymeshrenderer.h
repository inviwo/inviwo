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

#ifndef IVW_FANCYMESHRENDERER_H
#define IVW_FANCYMESHRENDERER_H

#include <fancymeshrenderer/fancymeshrenderermoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <modules/opengl/shader/shader.h>

#include <fancymeshrenderer/processors/FragmentListRenderer.h>

namespace inviwo {

/** \docpage{org.inviwo.FancyMeshRenderer, Fancy Mesh Renderer}
 * ![](org.inviwo.FancyMeshRenderer.png?classIdentifier=org.inviwo.FancyMeshRenderer)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 * 
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */


/**
 * \class FancyMeshRenderer
 * \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
 * DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_FANCYMESHRENDERER_API FancyMeshRenderer : public Processor { 
public:
    FancyMeshRenderer();
    virtual ~FancyMeshRenderer() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

	virtual void initializeResources() override;
	virtual void process() override;

protected:

	void centerViewOnGeometry();
	std::pair<vec3, vec3> calcWorldBoundingBox() const;

	void setNearFarPlane();
	void updateDrawers();
	void copyFrontToBackSettings();

	void compileShader();

	MeshInport inport_;
	ImageInport imageInport_;
    ImageOutport outport_;

	CameraProperty camera_;
	ButtonProperty centerViewOnGeometry_;
	ButtonProperty setNearFarPlane_;
	ButtonProperty resetViewParams_;
	CameraTrackball trackball_;
	SimpleLightingProperty lightingProperty_;

	CompositeProperty layers_;
	BoolProperty colorLayer_;
	BoolProperty normalsLayer_;
	BoolProperty viewNormalsLayer_;

	BoolProperty separateFaceSettings_;
	ButtonProperty copyFrontToBack_;
    BoolProperty forceOpaque_;

	enum ColorSource
	{
		VertexColor,
		TransferFunction,
		ExternalColor
	};
	enum AlphaMode
	{
		Uniform,
		AngleBased,
		NormalBased
	};
	enum NormalSource
	{
		InputVertex,
		InputTriangle,
		GenerateVertex,
		GenerateTriangle
	};
	enum ShadingMode
	{
		Off, //no light, no reflection, just diffuse
		Phong,
		Pbr
	};
	/**
	 * \brief The render settings per face.
	 * faceSettings_[0]=front face, faceSettings_[1]=back face
	 */
	struct FaceRenderSettings
	{
		CompositeProperty container_;
		BoolProperty cull_;
		
		TransferFunctionProperty transferFunction_;
		FloatVec4Property externalColor_;
		TemplateOptionProperty<ColorSource> colorSource_;

		TemplateOptionProperty<AlphaMode> alphaMode_;
		FloatProperty alphaScale_;

		TemplateOptionProperty<NormalSource> normalSource_;
		TemplateOptionProperty<ShadingMode> shadingMode_;

		FaceRenderSettings(const std::string& prefix);
	} faceSettings_[2];


	Shader shader_;
	bool needsRecompilation_;
	std::unique_ptr<MeshDrawer> drawer_;
    FragmentListRenderer flr_;
};

} // namespace

#endif // IVW_FANCYMESHRENDERER_H

