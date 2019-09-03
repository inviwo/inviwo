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

#include <modules/postprocessing/postprocessingmodule.h>
#include <modules/postprocessing/processors/ssao.h>
#include <modules/postprocessing/processors/fog.h>
#include <modules/postprocessing/processors/fxaa.h>
#include <modules/postprocessing/processors/tonemapping.h>
#include <modules/postprocessing/processors/hdrbloom.h>
#include <modules/postprocessing/processors/imagebrightnesscontrast.h>
#include <modules/postprocessing/processors/imageedgedarken.h>
#include <modules/postprocessing/processors/imagehuesaturationluminance.h>
#include <modules/postprocessing/processors/imageopacity.h>
#include <modules/postprocessing/processors/imagefilter.h>
#include <modules/postprocessing/processors/depthdarkening.h>
#include <modules/opengl/shader/shadermanager.h>

namespace inviwo {

PostProcessingModule::PostProcessingModule(InviwoApplication* app)
    : InviwoModule(app, "PostProcessing") {

    // Add a directory to the search path of the Shadermanager
    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    registerProcessor<SSAO>();
    registerProcessor<FXAA>();
    registerProcessor<Fog>();
    registerProcessor<Tonemapping>();
    registerProcessor<HdrBloom>();
    registerProcessor<ImageBrightnessContrast>();
    registerProcessor<ImageEdgeDarken>();
    registerProcessor<DepthDarkening>();
    registerProcessor<ImageHueSaturationLuminance>();
    registerProcessor<ImageFilter>();
    registerProcessor<ImageOpacity>();

    // Properties
    // registerProperty<PostProcessingProperty>();

    // Readers and writes
    // registerDataReader(std::make_unique<PostProcessingReader>());
    // registerDataWriter(std::make_unique<PostProcessingWriter>());

    // Data converters
    // registerRepresentationConverter(std::make_unique<PostProcessingDisk2RAMConverter>());

    // Ports
    // registerPort<PostProcessingOutport>();
    // registerPort<PostProcessingInport>();

    // PropertyWidgets
    // registerPropertyWidget<PostProcessingPropertyWidget, PostProcessingProperty>("Default");

    // Dialogs
    // registerDialog<PostProcessingDialog>(PostProcessingOutport);

    // Other varius things
    // registerCapabilities(std::make_unique<PostProcessingCapabilities>());
    // registerSettings(std::make_unique<PostProcessingSettings>());
    // registerMetaData(std::make_unique<PostProcessingMetaData>());
    // registerPortInspector("PostProcessingOutport", "path/workspace.inv");
    // registerProcessorWidget(std::string processorClassName, std::unique_ptr<ProcessorWidget>
    // processorWidget);
    // registerDrawer(util::make_unique_ptr<PostProcessingDrawer>());
}

}  // namespace inviwo
