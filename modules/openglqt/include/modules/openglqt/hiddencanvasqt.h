/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/openglqt/openglqtmoduledefine.h>  // for IVW_MODULE_OPENGLQT_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for LayerType, LayerType::Color
#include <inviwo/core/util/canvas.h>                      // for Canvas

#include <QSurfaceFormat>  // for QSurfaceFormat

#include <cstddef>      // for size_t
#include <memory>       // for unique_ptr, shared_ptr
#include <string_view>  // for string_view

class QOffscreenSurface;
class QOpenGLContext;  // Include causes: warning qopenglfunctions.h is not compatible with GLEW,
                       // GLEW defines will be undefined

namespace inviwo {

class Image;

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
     * Must call initializeGL before using it
     * The default format is usually set globally beforehand, usually directly in the main function.
     *     // Must be set before constructing QApplication
     *     QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
     *     QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
     *     QSurfaceFormat defaultFormat;
     *     defaultFormat.setMajorVersion(10);
     *     defaultFormat.setProfile(QSurfaceFormat::CoreProfile);
     *     QSurfaceFormat::setDefaultFormat(defaultFormat);
     */
    HiddenCanvasQt(std::string_view name, QSurfaceFormat format = QSurfaceFormat::defaultFormat());
    HiddenCanvasQt(const HiddenCanvasQt&) = delete;
    HiddenCanvasQt(HiddenCanvasQt&&) = delete;
    HiddenCanvasQt& operator=(const HiddenCanvasQt&) = delete;
    HiddenCanvasQt& operator=(HiddenCanvasQt&&) = delete;

    virtual ~HiddenCanvasQt();
    /*
     * Initialize context and OpenGL functions. Only call this function once.
     */
    void initializeGL();
    /*
     * Does nothing
     */
    virtual void render([[maybe_unused]] std::shared_ptr<const Image>,
                        [[maybe_unused]] LayerType layerType = LayerType::Color,
                        [[maybe_unused]] size_t idx = 0) override{};

    virtual void update() override;
    virtual void activate() override;

    // used to create hidden canvases used for context in background threads.
    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;
    static std::unique_ptr<Canvas> createHiddenQtCanvas();

    using ContextID = const void*;
    virtual ContextID activeContext() const override;
    virtual ContextID contextId() const override;

    virtual void releaseContext() override;

    QOpenGLContext* getContext() { return context_; };

protected:
    QOpenGLContext* context_;
    QOffscreenSurface* offScreenSurface_;
};

}  // namespace inviwo
