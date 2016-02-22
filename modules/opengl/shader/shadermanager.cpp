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
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/opengl/openglsettings.h>
#include <modules/opengl/openglmodule.h>
#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/openglcapabilities.h>

#include <inviwomodulespaths.h>

#include <string>

namespace inviwo {

ShaderManager::ShaderManager() : uniformWarnings_(nullptr) {
    openGLInfoRef_ = nullptr;
}

void ShaderManager::setOpenGLSettings(OpenGLSettings* settings) {
    uniformWarnings_ = &(settings->uniformWarnings_);

    for (auto shader : shaders_) {
        shader->setUniformWarningLevel(uniformWarnings_->get());
    }
    uniformWarnings_->onChange([this]() {
        for (auto shader : shaders_) {
            shader->setUniformWarningLevel(uniformWarnings_->get());
        }
    });

    shaderObjectErrors_ = &(settings->shaderObjectErrors_);

    for (auto shader : shaders_) {
        for (auto& obj : shader->getShaderObjects()) {
            obj.second->setError(shaderObjectErrors_->get());
        }
    }
    shaderObjectErrors_->onChange([this]() {
        for (auto shader : shaders_) {
            for (auto& obj : shader->getShaderObjects()) {
                obj.second->setError(shaderObjectErrors_->get());
            }
        }
    });
}

ShaderObject::Error ShaderManager::getShaderObjectError() const {
    return shaderObjectErrors_->get();
}

void ShaderManager::registerShader(Shader* shader) {
    shaders_.push_back(shader);
    if (uniformWarnings_) shader->setUniformWarningLevel(uniformWarnings_->get());
}

void ShaderManager::unregisterShader(Shader* shader) {
    util::erase_remove(shaders_, shader);
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

    if (glslVersion >= 130) {
        glBindFragDataLocation(programID, 0, "FragData0");
        glBindFragDataLocation(programID, 1, "PickingData");
    }
}

const std::vector<std::string>& ShaderManager::getShaderSearchPaths() { return shaderSearchPaths_; }

void ShaderManager::addShaderSearchPath(std::string shaderSearchPath) {
    if (!addShaderSearchPathImpl(shaderSearchPath)) {
        LogWarn("Failed to add shader search path: " << shaderSearchPath);
    }
}

void ShaderManager::resourceChanged(ShaderResource* resource) {
    BoolProperty& shaderReloadProp =
        InviwoApplication::getPtr()->getSettingsByType<OpenGLSettings>()->shaderReloadingProperty_;

    if (shaderReloadProp) {
        try {
            std::unordered_set<Shader*> toRelink;
            for (auto shader : shaders_) {
                const auto& shaderObjects = shader->getShaderObjects();
                for (const auto& shaderObject : shaderObjects) {
                    const auto& shaderResources = shaderObject.second->getResources();

                    if (util::contains(shaderResources, resource)) {
                        shaderObject.second->rebuild();
                        toRelink.insert(shader);
                    }
                }
            }
            // Relink after for loop to avoid invalidation the shaders_ iterators.
            // this can happen since link will trigger on reload callbacks.
            NetworkLock lock;
            for (auto shader : toRelink) shader->link(true);  // Link and notifyRebuild.

            LogInfo(resource->key() + " successfully reloaded");

        } catch (OpenGLException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
        }
    }
}

void ShaderManager::addShaderResource(std::string key, std::string src) {
    replaceInString(src, "NEWLINE", "\n");
    auto resource =  util::make_unique<StringShaderResource>(key, src);
    auto res = resource.get();
    resource->onChange([&, res](){
        resourceChanged(res);
    });
    shaderResources_[key] = std::move(resource);
}

void ShaderManager::addShaderResource(std::string key, std::unique_ptr<ShaderResource> resource) {
    auto res = resource.get();
    resource->onChange([&, res](){
        resourceChanged(res);
    });
    shaderResources_[key] = std::move(resource);
}

ShaderResource* ShaderManager::getShaderResource(std::string key) {
    auto it1 = shaderResources_.find(key);
    if (it1 != shaderResources_.end()) return it1->second.get();

    if (filesystem::fileExists(key)) {
        auto ret = shaderResources_.emplace(
            std::make_pair(key, util::make_unique<FileShaderResource>(key, key)));
        return ret.first->second.get();
    }

    auto it2 = util::find_if(shaderSearchPaths_, [&](const std::string& path) {
        return filesystem::fileExists(path + "/" + key);
    });
    if (it2 != shaderSearchPaths_.end()) {
        std::string file = *it2 + "/" + key;
        auto ret = shaderResources_.emplace(
            std::make_pair(key, util::make_unique<FileShaderResource>(key, file)));
        return ret.first->second.get();
    }

    std::string tmp{key};
    replaceInString(tmp, "/", "_");
    replaceInString(tmp, ".", "_");
    auto it0 = shaderResources_.find(tmp);
    if (it0 != shaderResources_.end()) return it0->second.get();

    return nullptr;
}

OpenGLCapabilities* ShaderManager::getOpenGLCapabilitiesObject() {
    if (!openGLInfoRef_) {
        if (auto openGLModule = InviwoApplication::getPtr()->getModuleByType<OpenGLModule>())
            openGLInfoRef_ = getTypeFromVector<OpenGLCapabilities>(openGLModule->getCapabilities());
    }
    return openGLInfoRef_;
}

void ShaderManager::rebuildAllShaders() {
    if (shaders_.empty()) return;

    for (auto& shader : shaders_) {
        shader->rebuild();
    }
    LogInfo("Rebuild of all shaders completed");
}

bool ShaderManager::addShaderSearchPathImpl(const std::string& shaderSearchPath) {
    if (filesystem::directoryExists(shaderSearchPath)) {
        shaderSearchPaths_.push_back(shaderSearchPath);
        return true;
    }
    return false;
}

const std::vector<Shader*>& ShaderManager::getShaders() const { return shaders_; }

}  // namespace
