/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_TRANSFERFUNCTIONEDITORVIEW_H
#define IVW_TRANSFERFUNCTIONEDITORVIEW_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/histogram.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsView>
#include <warn/pop>

class QWheelEvent;

namespace inviwo {

namespace util {
struct TFPropertyConcept;
}

class VolumeRAM;

class IVW_MODULE_QTWIDGETS_API TFEditorView : public QGraphicsView, public TFPropertyObserver {
public:
    TFEditorView(util::TFPropertyConcept* tfProperty, QGraphicsScene* scene = nullptr,
                 QWidget* parent = nullptr);
    ~TFEditorView();

protected:
    const HistogramContainer* getNormalizedHistograms();

    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    void updateHistogram();
    void updateZoom();

    // TransferFunctionPropertyObserver overloads
    virtual void onMaskChange(const dvec2& mask) override;
    virtual void onZoomHChange(const dvec2& zoomH) override;
    virtual void onZoomVChange(const dvec2& zoomV) override;
    virtual void onHistogramModeChange(HistogramMode mode) override;

    virtual void wheelEvent(QWheelEvent* event) override;

private:
    util::TFPropertyConcept* tfPropertyPtr_;
    VolumeInport* volumeInport_;
    HistogramMode histogramMode_;

    std::vector<QPolygonF> histograms_;

    bool stopHistCalculation_ = false;
    std::future<void> histCalculation_;

    dvec2 maskHorizontal_;

    const BaseCallBack* callbackOnInvalid = nullptr;
    const BaseCallBack* callbackOnChange = nullptr;
    const BaseCallBack* callbackOnConnect = nullptr;
    const BaseCallBack* callbackOnDisconnect = nullptr;
};

}  // namespace inviwo

#endif  // IVW_TRANSFERFUNCTIONEDITORVIEW_H
