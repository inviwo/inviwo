/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgraphicsitem.h>

#include <inviwo/core/network/networkannotations.h>
#include <inviwo/core/metadata/processormetadata.h>

#include <QObject>
#include <QStaticText>
#include <QGraphicsItem>

#include <memory>
#include <string_view>
#include <functional>

class QEvent;
class QGraphicsTextItem;
class QTextDocument;

namespace inviwo {

class Processor;
class ProcessorNetwork;

class IVW_QTEDITOR_API NetworkAnnotationGraphicsItem : public QObject,
                                                       public QGraphicsItem,
                                                       public ProcessorMetaDataObserver {
    Q_OBJECT
public:
    explicit NetworkAnnotationGraphicsItem(ProcessorNetwork* network,
                                           const NetworkAnnotation& annotation);
    NetworkAnnotationGraphicsItem(const NetworkAnnotationGraphicsItem&) = delete;
    NetworkAnnotationGraphicsItem(NetworkAnnotationGraphicsItem&&) = delete;
    NetworkAnnotationGraphicsItem& operator=(const NetworkAnnotationGraphicsItem&) = delete;
    NetworkAnnotationGraphicsItem& operator=(NetworkAnnotationGraphicsItem&&) = delete;
    virtual ~NetworkAnnotationGraphicsItem();

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = static_cast<int>(UserType) + static_cast<int>(NetworkAnnotationType) };
    virtual int type() const override { return Type; }

    void updateAnnotation(const NetworkAnnotation& annotation);

    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget) override;
    virtual QRectF boundingRect() const override;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void onProcessorMetaDataPositionChange() override;

private:
    void updateGeometry();
    void selectedHasChanged();
    void setRect(const QRectF& r);
    const QRectF& rect() const;
    NetworkEditor* getNetworkEditor() const;

    size_t index_;
    ProcessorNetwork* network_;

    std::vector<Processor*> processors_;

    QStaticText nameText_;
    QGraphicsTextItem* description_;
    QTextDocument* textDocument_;
    QRectF rect_;
    QMarginsF margins_;

    std::function<QString()> titleFunc_;
    std::function<QPointF(const QRectF&)> titlePos_;
    std::function<QPointF(const QRectF&)> descriptionPos_;
    std::function<double()> textWidth_;
    std::function<QColor()> color_;

    std::shared_ptr<std::function<void(std::string_view, std::string_view)>> nameChange_;
};

}  // namespace inviwo
