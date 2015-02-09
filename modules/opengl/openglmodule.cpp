/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <modules/opengl/buffer/bufferglconverter.h>
#include <modules/opengl/buffer/elementbuffergl.h>
#include <modules/opengl/buffer/elementbufferglconverter.h>
#include <modules/opengl/canvasprocessorgl.h>
#include <modules/opengl/geometry/geometryrenderprocessorgl.h>
#include <modules/opengl/glwrap/shadermanager.h>
#include <modules/opengl/image/layerglconverter.h>
#include <modules/opengl/openglmodule.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/rendering/meshrenderer.h>
#include <modules/opengl/volume/volumeglconverter.h>
#ifdef OPENGL_INCLUDE_SHADER_RESOURCES
#include <modules/opengl/shader_resources.h>
#endif

namespace inviwo {

OpenGLModule::OpenGLModule() :
    InviwoModule()
    , btnOpenGLInfo_("printOpenGLInfo", "Print OpenGL Info")
    , selectedOpenGLProfile_("selectedOpenGLProfile", "OpenGL Profile")
    , hasOutputedGLSLVersionOnce_(false) {
    selectedOpenGLProfile_.addOption("core", "Core");
    selectedOpenGLProfile_.addOption("compatibility", "Compatibility");
    selectedOpenGLProfile_.setSelectedIdentifier(OpenGLCapabilities::getPreferredProfile());
    selectedOpenGLProfile_.setCurrentStateAsDefault();
    SystemSettings* settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    if(settings){
        settings->addProperty(&selectedOpenGLProfile_);
        settings->loadFromDisk();
    }
    setIdentifier("OpenGL");
    ShaderManager::init();

#ifdef OPENGL_INCLUDE_SHADER_RESOURCES
    OPENGL_addGeneratedShaderResources();
#else
    ShaderManager::getPtr()->addShaderSearchPath(InviwoApplication::PATH_MODULES, "opengl/glsl");
#endif

    registerRenderer(new MeshRenderer());
    registerRepresentationConverter(new LayerRAM2GLConverter());
    registerRepresentationConverter(new LayerGL2RAMConverter());
    registerRepresentationConverter(new LayerDisk2GLConverter());

    registerRepresentationConverter(new VolumeRAM2GLConverter());
    registerRepresentationConverter(new VolumeGL2RAMConverter());
    registerRepresentationConverter(new VolumeDisk2GLConverter());
    
    registerRepresentationConverter(new BufferRAM2GLConverter());
    registerRepresentationConverter(new BufferGL2RAMConverter());
    registerRepresentationConverter(new ElementBufferRAM2GLConverter());
    registerRepresentationConverter(new ElementBufferGL2RAMConverter());

    registerProcessor(CanvasProcessorGL);
    registerProcessor(GeometryRenderProcessorGL);
    registerCapabilities(new OpenGLCapabilities());
    contextMode_ = OpenGLCapabilities::getPreferredProfile();
}



OpenGLModule::~OpenGLModule() {
    //ShaderManager::deleteInstance();
}

void OpenGLModule::updateProfile(){
    OpenGLCapabilities* openglInfo = getTypeFromVector<OpenGLCapabilities>(getCapabilities());

    if (openglInfo) {
        if(openglInfo->setPreferredProfile(selectedOpenGLProfile_.getSelectedValue(), !hasOutputedGLSLVersionOnce_)){
            ShaderManager::getPtr()->rebuildAllShaders();
            if(contextMode_ != selectedOpenGLProfile_.getSelectedValue())
                LogInfoCustom("OpenGLInfo", "Restart application to enable " << selectedOpenGLProfile_.getSelectedValue() << " mode.");
        }
        hasOutputedGLSLVersionOnce_ = true;
    }
}

void OpenGLModule::setupModuleSettings() {
    //New OpengGLSettings class can be created if required.
    SystemSettings* settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();

    if (settings) {
        OpenGLCapabilities* openglInfo = getTypeFromVector<OpenGLCapabilities>(getCapabilities());

        if (openglInfo) {
            btnOpenGLInfo_.onChange(openglInfo, &OpenGLCapabilities::printDetailedInfo);
            settings->addProperty(&btnOpenGLInfo_);

            if(openglInfo->getCurrentShaderVersion().getVersion() >= 150){
                selectedOpenGLProfile_.onChange(this, &OpenGLModule::updateProfile);
                updateProfile();
            }
            else
                selectedOpenGLProfile_.setVisible(false);
        }
    }
}

} // namespace
