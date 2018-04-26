/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_TRANSFERFUNCTIONEDITOR_H
#define IVW_TRANSFERFUNCTIONEDITOR_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/properties/transferfunctioneditorprimitive.h>
#include <inviwo/core/datastructures/datamapper.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QPointF>
#include <warn/pop>

#include <vector>

class QGraphicsView;
class QGraphicsSceneContextMenuEvent;

namespace inviwo {

class TransferFunction;
class TransferFunctionProperty;
class TransferFunctionEditorControlPoint;
class TransferFunctionControlPointConnection;
class TFPrimitive;

class IVW_MODULE_QTWIDGETS_API TransferFunctionEditor : public QGraphicsScene,
                                                        public TFEditorPrimitiveObserver {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    TransferFunctionEditor(TransferFunctionProperty* tfProperty, QWidget* parent = nullptr);
    virtual ~TransferFunctionEditor();

    virtual void onControlPointAdded(TFPrimitive* p);
    virtual void onControlPointRemoved(TFPrimitive* p);
    virtual void onControlPointChanged(const TFPrimitive* p);

    void updateConnections();

    void setMoveMode(int i);
    int getMoveMode() const;

    /**
     * \brief Set the display size of the control points.
     *
     * @see TransferFunctionEditorControlPoint::setSize
     * @param val Display size
     */
    void setControlPointSize(double val);
    /**
     * \brief Get the display size of the control points.
     * @see TransferFunctionEditorControlPoint::setSize
     */
    double getControlPointSize() const;

    void setRelativeSceneOffset(const dvec2& offset);
    const dvec2& getRelativeSceneOffset() const;

    const DataMapper& getDataMapper() const;

    TransferFunctionProperty* getTransferFunctionProperty();

    std::vector<TFPrimitive*> getSelectedPrimitives() const;

signals:
    void showColorDialog();
    void importTF();
    void exportTF();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override;

    /**
     * \brief adds a new control point at the given position
     *
     * Adds a new control point the the points_ array, adds a new line item to the lines_ array,
     * sorts the points_ array and updates the line items to go to and from the correct points.
     * Runs CalcTransferValues to update the TransferFunction data Image
     */
    void addControlPoint(const QPointF& pos);
    void addControlPoint(double pos, const vec4& color);

    /**
     * \brief adds a new TF Peak at the given position
     *
     * Adds a control point as well as two more points with 0 alpha, one point to the left and the
     * second to the right using the current primitive offset.
     */
    void addControlPointPeak(const QPointF& pos);

    void removeControlPoint(TransferFunctionEditorPrimitive* p);

    TransferFunctionEditorPrimitive* getTFPrimitiveItemAt(const QPointF& pos) const;

    virtual void onTFPrimitiveDoubleClicked(const TransferFunctionEditorPrimitive* p) override;

private:
    std::vector<TransferFunctionEditorPrimitive*> getSelectedPrimitiveItems() const;

    double controlPointSize_ = 15.0;           //!< size of TF primitives
    dvec2 relativeSceneOffset_ = dvec2(10.0);  //!< offset for duplicating TF primitives

    TransferFunctionProperty* tfProperty_;
    TransferFunction* transferFunction_;  //!< pointer to TF inside TF property

    using PointVec = std::vector<TransferFunctionEditorControlPoint*>;
    using ConnectionVec = std::vector<TransferFunctionControlPointConnection*>;
    PointVec points_;
    ConnectionVec connections_;
    bool mouseDrag_;
    bool mouseMovedSincePress_ = false;
    bool mouseDoubleClick_ = false;
    DataMapper dataMap_;

    std::vector<std::vector<TransferFunctionEditorPrimitive*> > groups_;
    int moveMode_;

    bool selectNewPrimitives_;
};

inline double TransferFunctionEditor::getControlPointSize() const { return controlPointSize_; }
inline const dvec2& TransferFunctionEditor::getRelativeSceneOffset() const {
    return relativeSceneOffset_;
}

}  // namespace inviwo

#endif  // IVW_TRANSFERFUNCTIONEDITOR_H