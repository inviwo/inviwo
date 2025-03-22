/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/webbrowser/webbrowsermoduledefine.h>  // for IVW_MODULE_WEBBROWSER_API

#include <inviwo/core/util/glmvec.h>           // for size2_t
#include <modules/opengl/inviwoopengl.h>       // for GL_BGRA, GL_NEAREST, GL_RGBA, GL_...
#include <modules/opengl/texture/texture2d.h>  // for Texture2D

#include <functional>  // for function
#include <map>         // for map

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_base.h>            // for CefRefPtr, CefRect, IMPLEMENT_REF...
#include <include/cef_render_handler.h>  // for CefRenderHandler, CefRenderHandle...

class CefBrowser;

#include <warn/pop>

namespace inviwo {

/** \class RenderHandlerGL
 * Copies web page into a Texture2D each time it has been painted by the browser and calls
 * onWebPageCopiedCallback afterwards.
 */
#include <warn/push>
#include <warn/ignore/dll-interface-base>  // Fine if dependent libs use the same CEF lib binaries
#include <warn/ignore/extra-semi>  // Due to IMPLEMENT_REFCOUNTING, remove when upgrading CEF
class IVW_MODULE_WEBBROWSER_API RenderHandlerGL : public CefRenderHandler {
public:
    RenderHandlerGL();
    void updateCanvasSize(CefRefPtr<CefBrowser> browser, size2_t newSize);

    ///
    // Called to retrieve the view rectangle which is relative to screen
    // coordinates. Return true if the rectangle was provided.
    ///
    /*--cef()--*/
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    ///
    // Called when the browser wants to show or hide the popup widget. The popup
    // should be shown if |show| is true and hidden if |show| is false.
    ///
    /*--cef()--*/
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;

    ///
    // Called when the browser wants to move or resize the popup widget. |rect|
    // contains the new location and size in view coordinates.
    ///
    /*--cef()--*/
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                         const RectList& dirtyRects, const void* buffer, int width,
                         int height) override;

    virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                                    const RectList& dirtyRects,
                                    const CefAcceleratedPaintInfo& info) override;

    /*
     * Get data containing the web page.
     * Note that top-left is considered origin so it needs to be vertically flipped to match Inviwo
     * convention. The coordinate system looks like this:
     * (0,0) -- (1,0)
     *   |        |
     * (0,1) -- (1,1)
     */
    Texture2D& getTexture2D(CefRefPtr<CefBrowser> browser);

    void ClearPopupRects(CefRefPtr<CefBrowser> browser);

    void onRender(CefRefPtr<CefBrowser> browser, std::function<void(Texture2D&)> onRender) {
        browserData_[browser->GetIdentifier()].onRender = onRender;
    }

private:
    struct BrowserData {
        BrowserData() { texture2D.initialize(nullptr); }
        size2_t viewRect{1, 1};
        Texture2D texture2D{size2_t{1, 1}, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, GL_NEAREST};
        CefRect popupRect;
        CefRect originalPopupRect;
        std::function<void(Texture2D&)> onRender;
    };
    CefRect GetPopupRectInWebView(CefRefPtr<CefBrowser> browser, const CefRect& original_rect);

    std::map<int, BrowserData> browserData_;  /// Per browser data

    IMPLEMENT_REFCOUNTING(RenderHandlerGL);
};
#include <warn/pop>
};  // namespace inviwo
