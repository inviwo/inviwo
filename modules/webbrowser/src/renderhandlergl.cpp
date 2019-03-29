/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/webbrowser/renderhandlergl.h>

namespace inviwo {

RenderHandlerGL::RenderHandlerGL(std::function<void()> onWebPageCopiedCallback)
    : CefRenderHandler()
    , texture2D_(size2_t{1, 1}, GL_BGRA, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST)
    , onWebPageCopiedCallback{onWebPageCopiedCallback} {}

void RenderHandlerGL::updateCanvasSize(size2_t newSize) { texture2D_.resize(newSize); }

bool RenderHandlerGL::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    rect = CefRect{0, 0, static_cast<int>(texture2D_.getWidth()),
                   static_cast<int>(texture2D_.getHeight())};
    return true;
}

void RenderHandlerGL::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType,
                              const RectList& dirtyRects, const void* buffer, int width,
                              int height) {
    auto dims = texture2D_.getDimensions();

    if (dims.x == static_cast<size_t>(width) && dims.y == static_cast<size_t>(height)) {
        // CPU implementation using LayerRAM

        // Flipping image and swizzling using CPU code was too slow.
        // Instead send to GPU and do it there.

        // auto indata = static_cast<const unsigned char*>(buffer);
        // auto outdata = static_cast<unsigned char*>(layer->getData());
        // auto indataV = static_cast<const glm::tvec4<unsigned char>*>(buffer);
        // auto outdataV = static_cast<glm::tvec4<unsigned char>*>(layer->getData());
        // auto rowSize = width * 4;
        // auto size = width * height * 4;
        // for (auto row = 0; row < height; ++row) {
        //    for (auto col = 0; col < width; ++col) {
        //        auto index = row * width + col;
        //        auto outIndex = width * (height - row - 1) + col;
        //        const auto& pixel = indataV[index];
        //        // Swizzle, bgra -> rgba
        //        outdataV[outIndex] = {pixel[2], pixel[1], pixel[0], pixel[3]};
        //    }
        //}

        if ((dirtyRects.size() == 1 && dirtyRects[0] == CefRect(0, 0, width, height))) {
            // Upload all data
            texture2D_.upload(buffer);
        } else {
            // Update dirty areas
            texture2D_.bind();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
            for (const auto& rect : dirtyRects) {
                // const CefRect& rect = *i;
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x);
                glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y);
                glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height,
                                texture2D_.getFormat(), texture2D_.getDataType(), buffer);
            }
        }
        // Reset states
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

        // Notify that we are done copying
        onWebPageCopiedCallback();
    }
}

};  // namespace inviwo
