/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/datastructures/histogramtools.h>
#include <modules/qtwidgets/tf/tfpropertyconcept.h>

#include <QGraphicsView>

class QWheelEvent;

namespace inviwo {

class VolumeRAM;

class IVW_MODULE_QTWIDGETS_API TFEditorView : public QGraphicsView, public TFPropertyObserver {
public:
    explicit TFEditorView(TFPropertyConcept* tfProperty, QGraphicsScene* scene = nullptr,
                          QWidget* parent = nullptr);
    ~TFEditorView();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

    void updateZoom();

    // TransferFunctionPropertyObserver overloads
    virtual void onZoomHChange(const dvec2& zoomH) override;
    virtual void onZoomVChange(const dvec2& zoomV) override;
    virtual void onHistogramModeChange(HistogramMode mode) override;
    virtual void onHistogramSelectionChange(HistogramSelection selection) override;

    virtual void wheelEvent(QWheelEvent* event) override;

private:
    TFPropertyConcept* property_;

    struct HistogramState {
        TFPropertyConcept::HistogramChange change = TFPropertyConcept::HistogramChange::NoData;
        HistogramMode mode = HistogramMode::Off;
        HistogramSelection selection = histogramSelectionAll;
        DataMapper dataMap;
        std::vector<Histogram1D> histograms = {};
        std::vector<QPolygonF> polygons = {};
        static void paintHistogram(QPainter* painter, const QPolygonF& histogram, size_t channel,
                                   size_t nChannels, const QRectF& sceneRect);
        static void paintLabel(QPainter* painter, size_t channel, size_t count, size_t nChannels,
                               const QRect& rect);
        void paintState(QPainter* painter, const QRect& rect) const;
        void paintHistograms(QPainter* painter, const QRectF& sceneRect, const QRect& rect) const;
        static QPolygonF createHistogramPolygon(const Histogram1D& histogram, HistogramMode mode,
                                                const DataMapper& dataMap);
        static std::vector<QPolygonF> createHistogramPolygons(
            const std::vector<Histogram1D>& histograms, HistogramMode mode,
            const DataMapper& dataMap);
    };
    HistogramState histogramState_;
    DispatcherHandle<TFPropertyConcept::HistogramCallback> histogramChangeHandle_;

    std::shared_ptr<std::function<void()>> callbackOnChange = nullptr;
    std::shared_ptr<std::function<void()>> callbackOnConnect = nullptr;
    std::shared_ptr<std::function<void()>> callbackOnDisconnect = nullptr;
};

}  // namespace inviwo
