/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_CANVASPROCESSORWIDGETGLFW_H
#define IVW_CANVASPROCESSORWIDGETGLFW_H

#include <modules/glfw/glfwmoduledefine.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>

namespace inviwo {

class CanvasGLFW;
class CanvasProcessor;

class IVW_MODULE_GLFW_API CanvasProcessorWidgetGLFW : public CanvasProcessorWidget {
public:
    CanvasProcessorWidgetGLFW(Processor* p);
    virtual ~CanvasProcessorWidgetGLFW();
    
    virtual void setVisible(bool visible) override;
    virtual void show() override;
    virtual void hide() override;
    virtual void setDimensions(ivec2) override;
    virtual void setPosition(ivec2) override;

    virtual Canvas* getCanvas() const override;

private:
    virtual void updateVisible(bool visible) override;
    virtual void updateDimensions(ivec2) override;
    virtual void updatePosition(ivec2) override;

    using canvas_ptr = std::unique_ptr<CanvasGLFW, std::function<void(CanvasGLFW*)>>;
    canvas_ptr canvas_;
};

} // namespace

#endif // IVW_CANVASPROCESSORWIDGETGLFW_H
