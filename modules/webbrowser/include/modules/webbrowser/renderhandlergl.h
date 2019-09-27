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

#ifndef IVW_RENDERHANDLERGL_H
#define IVW_RENDERHANDLERGL_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/opengl/texture/texture2d.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_render_handler.h>
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
    typedef std::function<void()> OnWebPageCopiedCallback;

    RenderHandlerGL(OnWebPageCopiedCallback onWebPageCopiedCallback);
    void updateCanvasSize(size2_t newSize);
    ///
    // Called to retrieve the view rectangle which is relative to screen
    // coordinates. Return true if the rectangle was provided.
    ///
    /*--cef()--*/
    virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;

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
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect &rect) override;

    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                         const RectList &dirtyRects, const void *buffer, int width,
                         int height) override;
    /*
     * Get data containing the web page.
     * Note that top-left is considered origin so it needs to be vertically flipped to match Inviwo
     * convention. The coordinate system looks like this:
     * (0,0) -- (1,0)
     *   |        |
     * (0,1) -- (1,1)
     */
    Texture2D &getTexture2D() { return texture2D_; }

    /*
     * Get RGBA color of pixel given top-left as pixel origin.
     * The coordinate system looks like this:
     *     (0,0)     --     (width-1,0)
     *       |                   |
     * (0,height -1) -- (width-1,height -1)
     */
    uvec4 getPixel(int x, int y);

    void ClearPopupRects();

private:
    CefRect GetPopupRectInWebView(const CefRect &original_rect);

    Texture2D texture2D_;
    OnWebPageCopiedCallback
        onWebPageCopiedCallback;  /// Called after web page has been copied in OnPaint

    CefRect popupRect_;
    CefRect originalPopupRect_;

public:
    IMPLEMENT_REFCOUNTING(RenderHandlerGL);
};
#include <warn/pop>
};  // namespace inviwo

#endif  // IVW_RENDERHANDLERGL_H
