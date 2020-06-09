/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2020 Inviwo Foundation
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

#include <modules/openglqt/openglqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/canvas.h>
#include <modules/opengl/inviwoopengl.h> // Make sure GLEW is included before QOpenGL stuff

#include <warn/push>
#include <warn/ignore/all>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <warn/pop>

namespace inviwo {

/*
 * Convenience class for creating an QOffscreenSurface with an QOpenGLContext.
 *
 * This class can be used for concurrent OpenGL operations in background threads.
 * The class must be created in the main thread.
 * initializeGL must be called before use, but can be called in a different thread.
 *
 * @note Most Canvas overriden functions are non-functional except HiddenCanvasQt::activate()
 */
class IVW_MODULE_OPENGLQT_API HiddenCanvasQt : public Canvas {
public:
    /*
     * Must be created in the main thread.
     * Must call initializeGL before using it for
     */
    HiddenCanvasQt(QSurfaceFormat format = QSurfaceFormat::defaultFormat());
    virtual ~HiddenCanvasQt() = default;
    /*
     * Initialize context and OpenGL functions. Only call this function once.
     */
    void initializeGL();
    /*
     * Does nothing
     */
    virtual void render([[maybe_unused]] std::shared_ptr<const Image>,
                        [[maybe_unused]] LayerType layerType = LayerType::Color,
                        [[maybe_unused]] size_t idx = 0) override {};

    virtual size2_t getImageDimensions() const { return size2_t{0}; }
    virtual void update() override;
    virtual void activate() override;

    // used to create hidden canvases used for context in background threads.
    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;
    static std::unique_ptr<Canvas> createHiddenQtCanvas();

    using ContextID = const void*;
    virtual ContextID activeContext() const override;
    virtual ContextID contextId() const override;

    virtual void releaseContext() override;

    QOpenGLContext* getContext() { return &context_; };

protected:
    /**
     * Derived classes should override to implement actual window state.
     */
    virtual void setFullScreenInternal([[maybe_unused]] bool fullscreen) override {};
    QOpenGLContext context_;
    QOffscreenSurface offScreenSurface_;
};

}  // namespace inviwo
