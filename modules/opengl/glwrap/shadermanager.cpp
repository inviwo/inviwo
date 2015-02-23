/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "shadermanager.h"
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/opengl/openglsettings.h>
#include <modules/opengl/openglmodule.h>
#include <pathsexternalmodules.h>
#include <string>

namespace inviwo {

ShaderManager::ShaderManager() : FileObserver() {
    InviwoApplication::getPtr()->registerFileObserver(this);
    openGLInfoRef_ = nullptr;
}

void ShaderManager::registerShader(Shader* shader) {
    shaders_.push_back(shader);
    const Shader::ShaderObjectMap* shaderObjects = shader->getShaderObjects();

    for (const auto& shaderObject : *shaderObjects) {
        startFileObservation(shaderObject.second->getAbsoluteFileName());
        std::vector<std::string> shaderIncludes = shaderObject.second->getIncludeFileNames();

        for (auto& shaderInclude : shaderIncludes) startFileObservation(shaderInclude);
    }
}

void ShaderManager::unregisterShader(Shader* shader) {
    shaders_.erase(std::remove(shaders_.begin(), shaders_.end(), shader), shaders_.end());
    const Shader::ShaderObjectMap* shaderObjects = shader->getShaderObjects();

    for (const auto& shaderObject : *shaderObjects) {
        if (shaderObject.second != nullptr) {
            stopFileObservation(shaderObject.second->getAbsoluteFileName());
            std::vector<std::string> shaderIncludes = shaderObject.second->getIncludeFileNames();

            for (auto& shaderInclude : shaderIncludes) stopFileObservation(shaderInclude);
        }
    }
}

void ShaderManager::fileChanged(std::string shaderFilename) {
    Property* prop = InviwoApplication::getPtr()->getSettingsByType<OpenGLSettings>()->getPropertyByIdentifier("shaderReloading");
    if (prop){
        BoolProperty* shaderReloadProp = dynamic_cast<BoolProperty*>(prop);
        if (shaderReloadProp && shaderReloadProp->get()) {
            if (isObserved(shaderFilename)) {
                bool successfulReload = false;

                for (auto& elem : shaders_) {
                    bool relink = false;
                    const Shader::ShaderObjectMap* shaderObjects = elem->getShaderObjects();

                    for (const auto& shaderObject : *shaderObjects) {
                        std::vector<std::string> shaderIncludes =
                            shaderObject.second->getIncludeFileNames();

                        if (shaderObject.second->getAbsoluteFileName() == shaderFilename ||
                            std::find(shaderIncludes.begin(), shaderIncludes.end(),
                                      shaderFilename) != shaderIncludes.end()) {
                            successfulReload = shaderObject.second->rebuild();
                            relink = true;
                        }
                    }

                    if (relink) elem->link();
                }

                if (successfulReload) {
                    LogInfo(shaderFilename + " successfuly reloaded");
                    InviwoApplication::getPtr()->playSound(InviwoApplication::IVW_OK);
                    //TODO: Don't invalidate all processors when shader change, invalidate only owners if shader has one.
                    std::vector<Processor*> processors = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessors();

                    for (auto& processor : processors) {
                        std::string tags = processor->getTags().getString();
                        if (tags.find_first_of(Tags::GL.getString()) != std::string::npos)
                            processor->invalidate(INVALID_RESOURCES);
                    }
                }
                else InviwoApplication::getPtr()->playSound(InviwoApplication::IVW_ERROR);
            }
        }
    }
}

std::string ShaderManager::getGlobalGLSLHeader() {
    OpenGLCapabilities* glCaps = getOpenGLCapabilitiesObject();
    return glCaps->getCurrentGlobalGLSLHeader();
}

std::string ShaderManager::getGlobalGLSLVertexDefines() {
    OpenGLCapabilities* glCaps = getOpenGLCapabilitiesObject();
    return glCaps->getCurrentGlobalGLSLVertexDefines();
}

std::string ShaderManager::getGlobalGLSLFragmentDefines() {
    OpenGLCapabilities* glCaps = getOpenGLCapabilitiesObject();
    return glCaps->getCurrentGlobalGLSLFragmentDefines();
}

int ShaderManager::getGlobalGLSLVersion() {
    OpenGLCapabilities* glCaps = getOpenGLCapabilitiesObject();
    return glCaps->getCurrentShaderVersion().getVersion();
}

void ShaderManager::bindCommonAttributes(unsigned int programID) {
    int glslVersion = this->getGlobalGLSLVersion();
    glBindAttribLocation(programID, 0, "in_Vertex");
    glBindAttribLocation(programID, 1, "in_Normal");
    glBindAttribLocation(programID, 2, "in_Color");
    glBindAttribLocation(programID, 3, "in_TexCoord");

    if (glslVersion >= 130){
        glBindFragDataLocation(programID, 0, "FragData0");
        glBindFragDataLocation(programID, 1, "PickingData");
    }
}

std::vector<std::string> ShaderManager::getShaderSearchPaths() { 
    return shaderSearchPaths_; 
}

void ShaderManager::addShaderSearchPath(std::string shaderSearchPath) {
    if (!addShaderSearchPathImpl(shaderSearchPath)){
        LogWarn("Failed to add shader search path: " << shaderSearchPath);
    }
}

void ShaderManager::addShaderSearchPath(InviwoApplication::PathType pathType, std::string relativeShaderSearchPath) {
    bool added = addShaderSearchPathImpl(InviwoApplication::getPtr()->getPath(pathType) + relativeShaderSearchPath);
#ifdef IVW_EXTERNAL_MODULES_PATH_COUNT
    if(!added && pathType == InviwoApplication::PATH_MODULES){
        for (int i = 0; !added && i < IVW_EXTERNAL_MODULES_PATH_COUNT; ++i){
            added |= addShaderSearchPathImpl(externalModulePaths_[i] + "/" + relativeShaderSearchPath);
        }
    }
#endif
    if (!added){
        LogWarn("Failed to add shader search path: " << relativeShaderSearchPath);
        LogInfo("Tried with:");
        LogInfo("\t" << InviwoApplication::getPtr()->getPath(pathType) + relativeShaderSearchPath);
#ifdef IVW_EXTERNAL_MODULES_PATH_COUNT
        if (pathType == InviwoApplication::PATH_MODULES){
            for (auto& elem : externalModulePaths_) {
                LogInfo("\t" << elem << "/" << relativeShaderSearchPath);
            }
        }
#endif
    }
}

void ShaderManager::addShaderResource(std::string key, std::string src){
    size_t start_pos = 0;
    std::string from="NEWLINE";
    std::string to="\n";
    while((start_pos = src.find(from, start_pos)) != std::string::npos) {
        src.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    shaderResources_[key] = src;
}

std::string ShaderManager::getShaderResource(std::string key){
    if (shaderResources_.find(key) == shaderResources_.end()) {
        return "";
    } else {
        return shaderResources_[key];
    }
}

OpenGLCapabilities* ShaderManager::getOpenGLCapabilitiesObject(){
    if (!openGLInfoRef_) {
        OpenGLModule* openGLModule = getTypeFromVector<OpenGLModule>(InviwoApplication::getPtr()->getModules());

        if (openGLModule)
            openGLInfoRef_ = getTypeFromVector<OpenGLCapabilities>(openGLModule->getCapabilities());
    }
    return openGLInfoRef_;
}

void ShaderManager::rebuildAllShaders(){
    if(shaders_.empty())
        return;

    for (auto& shader : shaders_) {
        shader->rebuild();
    }
    LogInfo("Rebuild of all shaders completed");
}

bool ShaderManager::addShaderSearchPathImpl(const std::string &shaderSearchPath){
    if (filesystem::directoryExists(shaderSearchPath)) {
        shaderSearchPaths_.push_back(shaderSearchPath);
        return true;
    }
    return false;
}

} // namespace
