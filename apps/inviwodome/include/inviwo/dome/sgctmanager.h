/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/util/glm.h>

#include <sgct/callbackdata.h>

#include <functional>
#include <vector>
#include <string>
#include <optional>

struct GLFWwindow;

namespace inviwo {

class GLFWUserData;
class GLFWWindowEventManager;
class Event;
class CanvasProcessor;
class CameraProperty;
class Shader;

struct SgctManager : public ProcessorNetworkObserver {
    SgctManager(InviwoApplication& app);
    ~SgctManager();

    CanvasProcessor* getCanvas() { return canvases.empty() ? nullptr : canvases.front(); }
    CameraProperty* getCamera() { return cameras.empty() ? nullptr : cameras.front(); }

    void setupUpInteraction(GLFWwindow* win);
    void teardownInteraction();

    void evaluate(const ::sgct::RenderData& renderData);

    void createShader();
    void copy();

    virtual void onProcessorNetworkDidAddProcessor(Processor*) override;
    virtual void onProcessorNetworkWillRemoveProcessor(Processor*) override;

    std::vector<CanvasProcessor*> canvases;
    std::vector<CameraProperty*> cameras;
    InviwoApplication& app;

    CanvasProcessor* eventTarget = nullptr;
    std::vector<std::unique_ptr<GLFWUserData>> userdata;
    std::vector<std::unique_ptr<GLFWWindowEventManager>> interactionManagers;
    std::unique_ptr<Shader> copyShader;
    std::function<void(bool)> onStatChange;


};

}  // namespace inviwo