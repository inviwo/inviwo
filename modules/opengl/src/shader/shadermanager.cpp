/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/opengl/shader/shadermanager.h>

#include <inviwo/core/common/inviwoapplication.h>   // for InviwoApplication
#include <inviwo/core/properties/optionproperty.h>  // for OptionProperty
#include <inviwo/core/util/dispatcher.h>            // for Dispatcher
#include <inviwo/core/util/filesystem.h>            // for fileExists, directoryExists
#include <inviwo/core/util/logcentral.h>            // for LogCentral
#include <inviwo/core/util/stdextensions.h>         // for erase_remove, find_if
#include <inviwo/core/util/stringconversion.h>      // for replaceInString
#include <modules/opengl/openglmodule.h>            // for OpenGLModule
#include <modules/opengl/openglsettings.h>          // for OpenGLSettings
#include <modules/opengl/shader/shader.h>           // for Shader, Shader::OnError
#include <modules/opengl/shader/shaderresource.h>   // for FileShaderResource, StringShaderResource
#include <modules/opengl/shader/fileshaderresource.h>
#include <modules/opengl/shader/stringshaderresource.h>

#include <algorithm>    // for find
#include <ostream>      // for operator<<
#include <string>       // for string, operator<, basic_string, char...
#include <type_traits>  // for remove_extent_t

#include <fmt/base.h>  // for format
#include <fmt/std.h>

namespace inviwo {

ShaderManager* ShaderManager::instance_ = nullptr;

ShaderManager::ShaderManager()
    : uniformWarnings_(nullptr), shaderObjectErrors_{nullptr} {}

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
}

Shader::OnError ShaderManager::getOnShaderError() const { return shaderObjectErrors_->get(); }

void ShaderManager::registerShader(Shader* shader) {
    shaders_.push_back(shader);
    if (uniformWarnings_) shader->setUniformWarningLevel(uniformWarnings_->get());
    shaderAddCallbacks_.invoke(shader->getID());
}

void ShaderManager::unregisterShader(Shader* shader) {
    shaderRemoveCallbacks_.invoke(shader->getID());
    std::erase(shaders_, shader);
}

bool ShaderManager::isRegistered(Shader* shader) const {
    return std::find(shaders_.begin(), shaders_.end(), shader) != shaders_.end();
}

const std::vector<std::filesystem::path>& ShaderManager::getShaderSearchPaths() {
    return shaderSearchPaths_;
}

void ShaderManager::addShaderSearchPath(const std::filesystem::path& shaderSearchPath) {
    if (!addShaderSearchPathImpl(shaderSearchPath)) {
        log::warn("Failed to add shader search path: {}", shaderSearchPath);
    }
}

void ShaderManager::addShaderResource(std::string key, std::string src) {
    replaceInString(src, "NEWLINE", "\n");
    auto resource = std::make_shared<StringShaderResource>(key, src);
    ownedResources_.push_back(resource);
    shaderResources_[key] = std::weak_ptr<ShaderResource>(resource);
}

void ShaderManager::addShaderResource(std::unique_ptr<ShaderResource> resource) {
    std::shared_ptr<ShaderResource> res(std::move(resource));
    ownedResources_.push_back(res);
    shaderResources_[res->key()] = std::weak_ptr<ShaderResource>(res);
}

void ShaderManager::addShaderResource(std::shared_ptr<ShaderResource> resource) {
    ownedResources_.push_back(resource);
    shaderResources_[resource->key()] = std::weak_ptr<ShaderResource>(resource);
}

std::shared_ptr<ShaderResource> ShaderManager::getShaderResource(std::string_view key) {
    auto it1 = shaderResources_.find(key);
    if (it1 != shaderResources_.end()) {
        if (!it1->second.expired()) {
            return it1->second.lock();
        } else {
            shaderResources_.erase(it1);
        }
    }

    std::string key2{key};
    replaceInString(key2, "/", "_");
    replaceInString(key2, ".", "_");
    auto it0 = shaderResources_.find(key2);
    if (it0 != shaderResources_.end()) {
        if (!it0->second.expired()) {
            return it0->second.lock();
        } else {
            shaderResources_.erase(it0);
        }
    }

    if (std::filesystem::is_regular_file(key)) {
        auto resource = std::make_shared<FileShaderResource>(key, key);
        shaderResources_.emplace(key, resource);
        return resource;
    }

    auto it2 = util::find_if(shaderSearchPaths_, [&](const std::filesystem::path& path) {
        return std::filesystem::is_regular_file(path / key);
    });
    if (it2 != shaderSearchPaths_.end()) {
        auto resource = std::make_shared<FileShaderResource>(key, *it2 / key);
        shaderResources_.emplace(key, resource);
        return resource;
    }

    return nullptr;
}

void ShaderManager::rebuildAllShaders() {
    if (shaders_.empty()) return;

    for (auto& shader : shaders_) {
        shader->build();
    }
    log::info("Rebuild of all shaders completed");
}

bool ShaderManager::addShaderSearchPathImpl(const std::filesystem::path& shaderSearchPath) {
    if (std::filesystem::is_directory(shaderSearchPath)) {
        shaderSearchPaths_.push_back(shaderSearchPath);
        return true;
    }
    return false;
}

const std::vector<Shader*>& ShaderManager::getShaders() const { return shaders_; }

}  // namespace inviwo
