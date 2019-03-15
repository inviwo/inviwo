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

#ifndef IVW_KEYFRAMEWIDGETQT_H
#define IVW_KEYFRAMEWIDGETQT_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframeobserver.h>
#include <modules/animationqt/widgets/editorconstants.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <warn/pop>

namespace inviwo {

namespace animation {

/**
 * \class KeyframeWidgetQt
 * \brief Graphical representation of a keyframe
 */
class IVW_MODULE_ANIMATIONQT_API KeyframeWidgetQt : public QGraphicsItem, public KeyframeObserver {
public:
    KeyframeWidgetQt(Keyframe& keyframe, QGraphicsItem* parent);
    virtual ~KeyframeWidgetQt() = default;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;

    const Keyframe& getKeyframe() const { return keyframe_; }
    Keyframe& getKeyframe() { return keyframe_; }

    /**
     * Lock when editing keyframe from GUI
     */
    void lock();
    void unlock();
    bool islocked() const;

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + static_cast<int>(ItemTypes::Keyframe) };
    virtual int type() const override { return Type; }

protected:
    virtual void onKeyframeTimeChanged(Keyframe* key, Seconds oldTime) override;

    virtual QRectF boundingRect() const override;
    // Restrict vertical movement and snap keyframe to grid
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    Keyframe& keyframe_;

    bool isEditing_ = false;  // User interaction
};

// A RAII utility for locking and unlocking the network
struct IVW_MODULE_ANIMATIONQT_API KeyframeWidgetQtLock {
    KeyframeWidgetQtLock(KeyframeWidgetQt* keyframe);
    ~KeyframeWidgetQtLock();

    KeyframeWidgetQtLock(KeyframeWidgetQtLock const&) = delete;
    KeyframeWidgetQtLock& operator=(KeyframeWidgetQtLock const& that) = delete;
    KeyframeWidgetQtLock(KeyframeWidgetQtLock&& rhs) = delete;
    KeyframeWidgetQtLock& operator=(KeyframeWidgetQtLock&& that) = delete;

private:
    KeyframeWidgetQt* keyframe_;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_KEYFRAMEWIDGETQT_H
