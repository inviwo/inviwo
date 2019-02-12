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

#ifndef IVW_KEYFRAMESEQUENCEWIDGETQT_H
#define IVW_KEYFRAMESEQUENCEWIDGETQT_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/keyframesequenceobserver.h>
#include <modules/animationqt/widgets/editorconstants.h>
#include <modules/animationqt/widgets/keyframewidgetqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <warn/pop>

namespace inviwo {

namespace animation {

class IVW_MODULE_ANIMATIONQT_API KeyframeSequenceWidgetQt : public QGraphicsItem,
                                                            public KeyframeSequenceObserver {
public:
    KeyframeSequenceWidgetQt(KeyframeSequence& keyframeSequence, QGraphicsItem* parent);
    virtual ~KeyframeSequenceWidgetQt();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* options,
                       QWidget* widget) override;

    KeyframeSequence& getKeyframeSequence();
    const KeyframeSequence& getKeyframeSequence() const;

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + static_cast<int>(ItemTypes::KeyframeSequence) };
    virtual int type() const override { return Type; }

protected:
    virtual void onKeyframeAdded(Keyframe* key, KeyframeSequence* seq) override;
    virtual void onKeyframeRemoved(Keyframe* key, KeyframeSequence* seq) override;
    virtual void onKeyframeSequenceMoved(KeyframeSequence* seq) override;

    virtual QRectF boundingRect() const override;

    /**
     * \brief Get the KeyframeQt corresponding to the given keyframe
     *
     * @param keyframe The keyframe to search for
     * @return KeyframeWidgetQt containing the keyframe or null if not found.
     */
    KeyframeWidgetQt* getKeyframeQt(const Keyframe* keyframe) const;

    // Move all keyframes, restrict vertical movement and snap to grid
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    KeyframeSequence& keyframeSequence_;

    QRectF rect_;
    std::unordered_map<const Keyframe*, std::unique_ptr<KeyframeWidgetQt>> keyframes_;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_KEYFRAMESEQUENCEWIDGETQT_H
