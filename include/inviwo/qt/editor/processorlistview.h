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

#include <optional>

#include <QModelIndex>
#include <QTreeView>
#include <QDrag>

class QAbstractItemModel;
class QMouseEvent;
class QPoint;
class QFocusEvent;

namespace inviwo {

class ProcessorListWidget;
class Processor;

class IVW_QTEDITOR_API ProcessorListView : public QTreeView {

public:
    ProcessorListView(QAbstractItemModel* model, ProcessorListWidget* parent);
    virtual ~ProcessorListView() = default;

protected:
    static QModelIndex findFirstLeaf(QAbstractItemModel* model, QModelIndex parent = QModelIndex());

    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;

    virtual void focusInEvent(QFocusEvent* e) override;
    virtual void focusOutEvent(QFocusEvent* e) override;

private:
    void showContextMenu(const QPoint& p);

    ProcessorListWidget* processorTreeWidget_;
    std::optional<QPoint> dragStartPosition_;
};

class IVW_QTEDITOR_API ProcessorDragObject : public QDrag {
public:
    ProcessorDragObject(QWidget* source, std::shared_ptr<Processor> processor);
};

}  // namespace inviwo
