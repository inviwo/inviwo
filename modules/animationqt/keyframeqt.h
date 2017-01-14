/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_KEYFRAMEQT_H
#define IVW_KEYFRAMEQT_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <modules/animation/datastructures/keyframeobserver.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <warn/pop>

namespace inviwo {

namespace animation {

class Keyframe;

/**
 * \class KeyframeQt
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_ANIMATIONQT_API KeyframeQt : public QGraphicsItem, public KeyframeObserver {
public:
    KeyframeQt(Keyframe& keyframe, QGraphicsItem* parent);
    virtual ~KeyframeQt() = default;

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
    enum { Type = UserType + 1 };
    int type() const { return Type; }

protected:
    virtual void onKeyframeTimeChanged(Keyframe* key, Seconds oldTime) override;

    virtual QRectF boundingRect() const;
    // Restrict vertical movement and snap keyframe to grid
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    Keyframe& keyframe_;

    bool isEditing_ = false;  // User interaction
};

// A RAII utility for locking and unlocking the network
struct IVW_MODULE_ANIMATIONQT_API KeyframeQtLock {
    KeyframeQtLock(KeyframeQt* keyframe);
    ~KeyframeQtLock();

    KeyframeQtLock(KeyframeQtLock const&) = delete;
    KeyframeQtLock& operator=(KeyframeQtLock const& that) = delete;
    KeyframeQtLock(KeyframeQtLock&& rhs) = delete;
    KeyframeQtLock& operator=(KeyframeQtLock&& that) = delete;

private:
    KeyframeQt* keyframe_;
};

}  // namespace

}  // namespace

#endif  // IVW_KEYFRAMEQT_H
