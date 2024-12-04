/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/core/util/glm.h>            // for size2_t
#include <inviwo/core/util/rendercontext.h>  // for RenderContext
#include <inviwo/core/util/logcentral.h>
#include <modules/opengl/texture/texture2d.h>  // for Texture2D

#include <vector>  // for vector<>::value_type

#include <glm/common.hpp>                    // for max
#include <include/base/cef_scoped_refptr.h>  // for scoped_refptr
#include <include/cef_base.h>                // for CefRect, CefRefPtr, PET_VIEW, operator==
#include <include/cef_browser.h>             // for CefBrowser, CefBrowserHost
#include <include/cef_render_handler.h>      // for CefRenderHandler::RectList, CefRenderHandl...

#pragma optimize("", off)

namespace inviwo {

RenderHandlerGL::RenderHandlerGL() = default;

void RenderHandlerGL::updateCanvasSize(CefRefPtr<CefBrowser> browser, size2_t newSize) {
    rendercontext::activateDefault();
    // Prevent crash when newSize = 0
    const auto id = browser->GetIdentifier();
    auto it = browserData_.find(id);
    if (it == browserData_.end()) {
        throw Exception(IVW_CONTEXT, "Unexpected browser id {}", id);
    }

    it->second.viewRect = newSize;

    // we want to trigger a render here, to ensure that the resized outport in the processor is
    // updated with relevant data.
    auto& browserData = browserData_[browser->GetIdentifier()];
    try {
        browserData.onRender(browserData.texture2D);
    } catch (const std::exception& e) {
        LogErrorCustom("RenderHandlerGL", e.what());
    }
}

void RenderHandlerGL::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    const auto& viewRect = browserData_[browser->GetIdentifier()].viewRect;
    rect = CefRect{0, 0, static_cast<int>(viewRect.x), static_cast<int>(viewRect.y)};
}

void RenderHandlerGL::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) {
    if (!browser) return;

    if (!show) {
        // Clear the popup rectangle.
        ClearPopupRects(browser);
        browser->GetHost()->Invalidate(PET_VIEW);
    }
}

void RenderHandlerGL::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) {
    if (rect.width <= 0 || rect.height <= 0) return;
    auto& browserData = browserData_[browser->GetIdentifier()];
    browserData.originalPopupRect = rect;
    browserData.popupRect = GetPopupRectInWebView(browser, browserData.originalPopupRect);
}

CefRect RenderHandlerGL::GetPopupRectInWebView(CefRefPtr<CefBrowser> browser,
                                               const CefRect& original_rect) {
    CefRect rc(original_rect);
    // if x or y are negative, move them to 0.
    if (rc.x < 0) rc.x = 0;
    if (rc.y < 0) rc.y = 0;
    // if popup goes outside the view, try to reposition origin
    auto& texture2D = browserData_[browser->GetIdentifier()].texture2D;
    auto width = static_cast<int>(texture2D.getWidth());
    auto height = static_cast<int>(texture2D.getHeight());
    if (rc.x + rc.width > width) rc.x = width - rc.width;
    if (rc.y + rc.height > height) rc.y = height - rc.height;
    // if x or y became negative, move them to 0 again.
    if (rc.x < 0) rc.x = 0;
    if (rc.y < 0) rc.y = 0;
    return rc;
}

void RenderHandlerGL::ClearPopupRects(CefRefPtr<CefBrowser> browser) {
    auto& browserData = browserData_[browser->GetIdentifier()];
    browserData.popupRect.Set(0, 0, 0, 0);
    browserData.originalPopupRect.Set(0, 0, 0, 0);
}

void RenderHandlerGL::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                              const RectList& dirtyRects, const void* buffer, int width,
                              int height) {
    rendercontext::activateDefault();

    auto& browserData = browserData_[browser->GetIdentifier()];
    auto& texture2D = browserData.texture2D;
    auto& popupRect = browserData.popupRect;
    if (type == PET_VIEW && width == static_cast<int>(texture2D.getWidth()) &&
        height == static_cast<int>(texture2D.getHeight())) {
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
            texture2D.upload(buffer);
        } else {
            // Update dirty areas
            texture2D.bind();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // RGBA 8-bit are always aligned
            glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
            for (const auto& rect : dirtyRects) {
                // const CefRect& rect = *i;
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x);
                glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y);
                glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height,
                                texture2D.getFormat(), texture2D.getDataType(), buffer);
            }
        }
    } else if (type == PET_POPUP && popupRect.width > 0 && popupRect.height > 0) {
        //  buffer only contains data for drawing the popup widget (including dropdown elements)
        int skip_pixels = 0, x = popupRect.x;
        int skip_rows = 0, y = popupRect.y;
        int w = width;
        int h = height;

        // Adjust the popup to fit inside the view.
        if (x < 0) {
            skip_pixels = -x;
            x = 0;
        }
        if (y < 0) {
            skip_rows = -y;
            y = 0;
        }
        auto texWidth = static_cast<int>(texture2D.getWidth());
        auto texHeight = static_cast<int>(texture2D.getHeight());

        if (x + w > texWidth) w -= x + w - texWidth;
        if (y + h > texHeight) h -= y + h - texHeight;
        texture2D.bind();
        // Update the popup rectangle.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // RGBA 8-bit are always aligned
        glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, skip_pixels);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, skip_rows);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, texture2D.getFormat(),
                        texture2D.getDataType(), buffer);
    }
    // Reset states
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    if (type == PET_VIEW && !popupRect.IsEmpty()) {
        browser->GetHost()->Invalidate(PET_POPUP);
    }

    // Notify that we are done copying
    try {
        browserData.onRender(browserData.texture2D);
    } catch (const std::exception& e) {
        LogErrorCustom("RenderHandlerGL", e.what());
    }
}

void RenderHandlerGL::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                                         const RectList& dirtyRects,
                                         const CefAcceleratedPaintInfo& info) {

#if defined(WIN32)

    if (dirtyRects.empty()) {
        return;
    }
    // When the window is minimized the texture is 1x1.
    // This prevents a hard crash when minimizing the window or when you resize the window
    // so that only the top bar is visible.
    // @TODO (ylvse 2024-08-20): minimizing window should be handled with the appropriate
    // function in the CefBrowser called WasHidden
    if (dirtyRects[0].height <= 1 || dirtyRects[0].width <= 1) {
        return;
    }

    rendercontext::activateDefault();

    // Create new texture that we can copy the shared texture into. Unfortunately
    // textures are immutable so we have to create a new one

    const int newWidth = dirtyRects[0].width;
    const int newHeight = dirtyRects[0].height;
    const size2_t dims{newWidth, newHeight};

    Texture2D newTexture(dims, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, GL_NEAREST);

    // Create the memory object handle
    GLuint memObj;
    glCreateMemoryObjectsEXT(1, &memObj);

    // The size of the texture we get from CEF. The CEF format is CEF_COLOR_TYPE_BGRA_8888
    // It has 4 bytes per pixel. The mem object requires this to be multiplied with 2
    int size = newWidth * newHeight * 8;

    // Cef uses the GL_HANDLE_TYPE_D3D11_IMAGE_EXT handle for their shared texture
    // Import the shared texture to the memory object
    glImportMemoryWin32HandleEXT(memObj, size, GL_HANDLE_TYPE_D3D11_IMAGE_EXT,
                                 info.shared_texture_handle);

    // Allocate immutable storage for the texture for the data from the memory object
    // Use GL_RGBA8 since it is 4 bytes
    glTextureStorageMem2DEXT(newTexture.getID(), 1, GL_RGBA8, newWidth, newHeight, memObj, 0);

    glDeleteMemoryObjectsEXT(1, &memObj);

    // Set the updated texture
    auto& browserData = browserData_[browser->GetIdentifier()];
    browserData.texture2D = std::move(newTexture);

    // Notify that we are done copying
    if (browserData.onRender) {
        try {
            browserData.onRender(browserData.texture2D);
        } catch (const std::exception& e) {
            LogErrorCustom("RenderHandlerGL", e.what());
        }
    }

#endif
}

Texture2D& RenderHandlerGL::getTexture2D(CefRefPtr<CefBrowser> browser) {
    return browserData_[browser->GetIdentifier()].texture2D;
}

};  // namespace inviwo
