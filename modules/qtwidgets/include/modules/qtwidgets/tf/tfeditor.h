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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/datastructures/datamapper.h>   // for DataMapper
#include <inviwo/core/util/glmvec.h>                 // for dvec2, vec4
#include <modules/qtwidgets/tf/tfeditorprimitive.h>  // for TFEditorPrimitive, TFEditorPrimitiv...
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <modules/qtwidgets/tf/tfmovemode.h>

#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <vector>      // for vector
#include <span>

#include <QGraphicsScene>  // for QGraphicsScene
#include <QObject>         // for Q_OBJECT, signals
#include <QPointF>         // for QPointF

class QGraphicsSceneMouseEvent;
class QKeyEvent;
class QPointF;
class QWidget;
class QGraphicsSceneContextMenuEvent;

namespace inviwo {

class TFControlPointConnection;
class TFEditorControlPoint;
class TFEditorIsovalue;
class TFPrimitive;
class TFPrimitiveSet;
class TFPropertyConcept;
class TFEditorMaskMin;
class TFEditorMaskMax;

template <typename T>
struct PtrHash {
    using hash_type = std::hash<const T*>;
    using is_transparent = void;

    constexpr std::size_t operator()(const T* ptr) const { return hash_type{}(ptr); }
    constexpr std::size_t operator()(T* ptr) const { return hash_type{}(ptr); }
    constexpr std::size_t operator()(const T& item) const { return hash_type{}(&item); }
    constexpr std::size_t operator()(T& item) const { return hash_type{}(&item); }
};
template <typename T>
struct PtrEqual {
    using is_transparent = void;

    constexpr bool operator()(const T* ptr1, const T* ptr2) const { return ptr1 == ptr2; }
    constexpr bool operator()(const T& item1, const T* ptr2) const { return &item1 == ptr2; }
    constexpr bool operator()(const T* ptr1, const T& item2) const { return ptr1 == &item2; }
    constexpr bool operator()(const T& item1, const T& item2) const { return &item1 == item2; }
};

class IVW_MODULE_QTWIDGETS_API TFEditor : public QGraphicsScene, public TFPrimitiveSetObserver {
    Q_OBJECT
public:
    explicit TFEditor(TFPropertyConcept* tfProperty, QWidget* parent = nullptr);
    virtual ~TFEditor();

    void setMoveMode(TFMoveMode i);
    TFMoveMode getMoveMode() const;

    const DataMapper& getDataMapper() const;

    std::vector<TFPrimitive*> getSelectedPrimitives() const;

    void updateConnections();

signals:
    void showColorDialog();
    void updateBegin();
    void updateEnd();
    void moveModeChange(TFMoveMode);

protected:
    virtual void onTFPrimitiveAdded(const TFPrimitiveSet& set, TFPrimitive& p) override;
    virtual void onTFPrimitiveRemoved(const TFPrimitiveSet& set, TFPrimitive& p) override;
    virtual void onTFPrimitiveChanged(const TFPrimitiveSet& set, const TFPrimitive& p) override;
    virtual void onTFTypeChanged(const TFPrimitiveSet& set, TFPrimitiveSetType type) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override;

    void removeControlPoint(TFEditorPrimitive* p);

    TFEditorPrimitive* getTFPrimitiveItemAt(const QPointF& pos) const;

private:
    void addPoint(double pos, const vec4& color, TFPrimitiveSet* set);
    void addPoint(double pos, double alpha, TFPrimitiveSet* set);
    void addPoint(const QPointF& scenePos, TFPrimitiveSet* set);
    void addPoint(const QPointF& scenePos);
    void addPeak(const QPointF& scenePos, TFPrimitiveSet* set);
    double sceneToPos(const QPointF& pos) const;
    double sceneToAlpha(const QPointF& pos) const;

    TFPrimitiveSet* findSet(TFPrimitive*) const;

    std::vector<TFPrimitive*> getAllPrimitives() const;
    std::vector<TFPrimitive*> getAllOrSelectedPrimitives() const;

    std::vector<TFEditorPrimitive*> getSelectedPrimitiveItems() const;
    static void setSelected(std::span<TFEditorPrimitive*> primitives, bool selected);

    QTransform calcTransform(QPointF scenePos, QPointF lastScenePos) const;
    static QPointF calcTransformRef(std::span<TFEditorPrimitive*> primitives,
                                    TFEditorPrimitive* start);
    static void move(std::span<TFEditorPrimitive*> primitives, const QTransform& transform,
                     const QRectF& rect);

    void duplicate(std::span<TFEditorPrimitive*> primitives);

    bool handleGroupSelection(QKeyEvent* event);
    bool handleModifySelection(QKeyEvent* event);
    bool handleMoveSelection(QKeyEvent* event);
    void handleMirroring();
    /**
     * calculate the horizontal and vertical offset in scene coordinates based on the current
     * viewport size and zoom. The offset then corresponds to defaultOffset pixels on screen.
     */
    dvec2 viewDependentOffset() const;

    TFPropertyConcept* concept_;

    struct Items {
        std::vector<std::unique_ptr<TFEditorPrimitive>> points;
        std::vector<std::unique_ptr<TFControlPointConnection>> connections;
        bool connected = false;
    };
    std::unordered_map<TFPrimitiveSet*, Items, PtrHash<TFPrimitiveSet>, PtrEqual<TFPrimitiveSet>>
        primitives_;
    TFPrimitiveSet* activeSet_;

    TFPrimitiveSet* activeSet();
    Items* activeItem();

    std::unordered_map<TFPrimitive*, TFPrimitive*, PtrHash<TFPrimitive>, PtrEqual<TFPrimitive>>
        mirrors_;
    
    static double mirror(double dos, const DataMapper& dm);


    struct Mouse {
        TFEditorPrimitive* dragItem = nullptr;
        QPointF rigid = QPointF{0.0, 0.0};
    };

    Mouse mouse_;

    std::vector<std::vector<TFEditorPrimitive*>> groups_;
    TFMoveMode moveMode_;

    std::unique_ptr<TFEditorMaskMin> maskMin_;
    std::unique_ptr<TFEditorMaskMax> maskMax_;

    bool selectNewPrimitives_;
};

}  // namespace inviwo
