/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#ifndef IVW_HIDDENCANVASQT_H
#define IVW_HIDDENCANVASQT_H

#include <modules/openglqt/openglqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QResizeEvent>
#include <QPaintEvent>
#include <warn/pop>

namespace inviwo {

// Inspiration from http://www.krazer.com/?p=109
// https://github.com/caseymcc/TestOpenCL

template <typename Base>
class HiddenCanvasQt : public Base {
public:
    explicit HiddenCanvasQt(uvec2 dim = uvec2(256, 256)) : Base(dim, "Background") {
        this->setVisible(false);
        this->doneCurrent();
    }
    virtual ~HiddenCanvasQt() = default;

protected:
    virtual void initializeGL() override{};
    virtual void resizeGL(int /*width*/, int /*height*/) override{};
    virtual void paintGL() override{};

    virtual void resizeEvent(QResizeEvent *) override{};
    virtual void paintEvent(QPaintEvent *) override{};
};

}  // namespace inviwo

#endif  // IVW_HIDDENCANVASQT_H
