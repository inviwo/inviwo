/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/animationqt/animationeditorqt.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/indirectiterator.h>
#include <inviwo/core/util/zip.h>
#include <modules/animation/animationcontroller.h>
#include <modules/animation/animationcontrollerobserver.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/keyframe.h>  // IWYU pragma: keep
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animationqt/factories/trackwidgetqtfactory.h>
#include <modules/animationqt/widgets/editorconstants.h>
#include <modules/animationqt/widgets/keyframesequencewidgetqt.h>
#include <modules/animationqt/widgets/keyframewidgetqt.h>
#include <modules/animationqt/widgets/trackwidgetqt.h>
#include <modules/qtwidgets/textlabeloverlay.h>

#include <chrono>
#include <set>
#include <sstream>
#include <string>
#include <utility>

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QFlags>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QKeySequence>
#include <QList>
#include <QMimeData>
#include <QPen>
#include <QPointF>
#include <QTransform>
#include <QWidget>
#include <Qt>
#include <QtGlobal>
#include <fmt/core.h>

class QGraphicsSceneDragDropEvent;

namespace inviwo {

namespace animation {

AnimationEditorQt::AnimationEditorQt(
    AnimationController& controller, TrackWidgetQtFactory& widgetFactory,
    std::function<void(std::string_view, std::chrono::milliseconds)> showText)
    : QGraphicsScene()
    , controller_(controller)
    , widgetFactory_{widgetFactory}
    , showText_{showText} {
    auto& animation = controller_.getAnimation();
    animation.addObserver(this);
    controller_.AnimationControllerObservable::addObserver(this);

    // Add Property tracks
    for (auto& track : animation) {
        onTrackAdded(&track);
    }

    // Add drag&drop indicator
    QPen timePen;
    timePen.setColor(QColor(255, 128, 0));
    timePen.setWidthF(1.0);
    timePen.setCosmetic(true);
    timePen.setStyle(Qt::DashLine);
    dropIndicatorLine = addLine(10, 0, 10, 1000, timePen);
    if (dropIndicatorLine) {
        dropIndicatorLine->setZValue(1);
        dropIndicatorLine->setVisible(false);
    }

    updateSceneRect();
}

AnimationEditorQt::~AnimationEditorQt() = default;

void AnimationEditorQt::onAnimationChanged(AnimationController*, Animation* oldAnim,
                                           Animation* newAnim) {
    oldAnim->removeObserver(this);
    for (auto& track : *oldAnim) {
        onTrackRemoved(&track);
    }
    for (auto& track : *newAnim) {
        onTrackAdded(&track);
    }
    newAnim->addObserver(this);
}

std::unique_ptr<TrackWidgetQt> AnimationEditorQt::createTrackWidget(Track& track) const {
    auto widgetId = widgetFactory_.getWidgetId(track.getClassIdentifier());
    return widgetFactory_.create(widgetId, track);
}

void AnimationEditorQt::onTrackAdded(Track* track) {
    if (!track) return;
    if (auto trackWidget = createTrackWidget(*track)) {
        trackWidget->setPos(0, trackHeight * tracks_.size() + trackHeight * 0.5);
        this->addItem(trackWidget.get());
        tracks_[track] = std::move(trackWidget);
        updateSceneRect();
    } else {
        throw Exception(SourceContext{}, "Not able to create widget for track: {} of type: {}",
                        track->getName(), track->getClassIdentifier());
    }
}

void AnimationEditorQt::onTrackRemoved(Track* track) {
    tracks_.erase(track);

    for (auto&& item : util::enumerate(tracks_)) {
        item.second().second->setY(trackHeight * item.first() + trackHeight * 0.5);
    }
    updateSceneRect();
}

void AnimationEditorQt::keyPressEvent(QKeyEvent* keyEvent) {
    int k = keyEvent->key();
    if (k == Qt::Key_Delete) {  // Delete selected
        QList<QGraphicsItem*> itemList = selectedItems();
        for (auto& elem : itemList) {
            if (auto keyqt = qgraphicsitem_cast<KeyframeWidgetQt*>(elem)) {
                auto& animation = controller_.getAnimation();
                animation.remove(&(keyqt->getKeyframe()));
            } else if (auto seqqt = qgraphicsitem_cast<KeyframeSequenceWidgetQt*>(elem)) {
                auto& animation = controller_.getAnimation();
                animation.remove(&(seqqt->getKeyframeSequence()));
            }
        }
        keyEvent->accept();
    } else if (keyEvent->matches(QKeySequence::Copy)) {
        copy();
        keyEvent->accept();
    } else if (keyEvent->matches(QKeySequence::Cut)) {
        cut();
        keyEvent->accept();
    } else if (keyEvent->matches(QKeySequence::Paste)) {
        paste();
        keyEvent->accept();
    }
    QGraphicsScene::keyPressEvent(keyEvent);
}

namespace {

Track* findOwningTrack(QGraphicsItem* item, Animation& animation) {
    // Navigate up from keyframe or sequence widget to its track widget
    QGraphicsItem* parent = item->parentItem();
    while (parent) {
        if (auto trackWidget = dynamic_cast<TrackWidgetQt*>(parent)) {
            return &trackWidget->getTrack();
        }
        parent = parent->parentItem();
    }
    return nullptr;
}

}  // namespace

void AnimationEditorQt::copy() {
    const QList<QGraphicsItem*> itemList = selectedItems();
    if (itemList.empty()) return;

    // Classify selected items: collect keyframes, sequences, and tracks.
    // Prefer the highest-level selection: if a sequence is selected, don't also copy its keyframes.
    std::vector<std::pair<Track*, Keyframe*>> keyframes;
    std::vector<std::pair<Track*, KeyframeSequence*>> sequences;
    std::set<Track*> trackSet;

    auto& animation = controller_.getAnimation();

    for (auto* elem : itemList) {
        if (auto seqqt = qgraphicsitem_cast<KeyframeSequenceWidgetQt*>(elem)) {
            auto* track = findOwningTrack(elem, animation);
            if (track) {
                sequences.emplace_back(track, &seqqt->getKeyframeSequence());
                trackSet.insert(track);
            }
        }
    }

    for (auto* elem : itemList) {
        if (auto keyqt = qgraphicsitem_cast<KeyframeWidgetQt*>(elem)) {
            auto* track = findOwningTrack(elem, animation);
            if (!track) continue;
            // Skip keyframes whose parent sequence is already selected
            auto& kf = keyqt->getKeyframe();
            bool parentSeqSelected = false;
            for (auto& [t, seq] : sequences) {
                for (size_t i = 0; i < seq->size(); ++i) {
                    if (&(*seq)[i] == &kf) {
                        parentSeqSelected = true;
                        break;
                    }
                }
                if (parentSeqSelected) break;
            }
            if (!parentSeqSelected) {
                keyframes.emplace_back(track, &kf);
            }
        }
    }

    // Determine what to serialize: prefer tracks > sequences > keyframes
    // If we have whole tracks selected (all sequences of a track are selected), serialize tracks.
    // Otherwise, serialize the individual items.

    auto mimeData = std::make_unique<QMimeData>();

    if (!sequences.empty() || !keyframes.empty()) {
        // Serialize tracks that contain all selected content.
        // Group by track: each track serializes its own selected sequences/keyframes.

        // Serialize keyframe sequences
        if (!sequences.empty()) {
            Serializer serializer("");
            // Serialize track type info alongside each sequence so paste knows what track to
            // create
            std::vector<std::string> trackClassIds;
            for (auto& [track, seq] : sequences) {
                trackClassIds.push_back(std::string(track->getClassIdentifier()));
            }
            serializer.serialize("trackClassIds", trackClassIds, "id");

            // Serialize each sequence
            std::vector<const KeyframeSequence*> seqs;
            for (auto& [track, seq] : sequences) {
                seqs.push_back(seq);
            }
            serializer.serialize("sequences", seqs, "sequence");

            std::stringstream ss;
            serializer.writeFile(ss);
            auto str = ss.str();
            QByteArray byteArray(str.c_str(), static_cast<int>(str.length()));
            mimeData->setData(
                QString::fromUtf8(mimeKeyframeSequences.data(), mimeKeyframeSequences.size()),
                byteArray);
        }

        // Serialize individual keyframes
        if (!keyframes.empty()) {
            Serializer serializer("");
            std::vector<std::string> trackClassIds;
            for (auto& [track, kf] : keyframes) {
                trackClassIds.push_back(std::string(track->getClassIdentifier()));
            }
            serializer.serialize("trackClassIds", trackClassIds, "id");

            std::vector<const Keyframe*> kfs;
            for (auto& [track, kf] : keyframes) {
                kfs.push_back(kf);
            }
            serializer.serialize("keyframes", kfs, "keyframe");

            std::stringstream ss;
            serializer.writeFile(ss);
            auto str = ss.str();
            QByteArray byteArray(str.c_str(), static_cast<int>(str.length()));
            mimeData->setData(QString::fromUtf8(mimeKeyframes.data(), mimeKeyframes.size()),
                              byteArray);
        }
    }

    // Only set clipboard if we have content
    if (mimeData->formats().size() > 0) {
        QApplication::clipboard()->setMimeData(mimeData.release());
        showText_("Copied to clipboard", std::chrono::milliseconds{1000});
    }
}

void AnimationEditorQt::paste() {
    auto* clipboard = QApplication::clipboard();
    const auto* mimeData = clipboard->mimeData();
    if (!mimeData) return;

    auto* app = controller_.getInviwoApplication();
    auto& animation = controller_.getAnimation();

    const auto seqMime =
        QString::fromUtf8(mimeKeyframeSequences.data(), mimeKeyframeSequences.size());
    const auto kfMime = QString::fromUtf8(mimeKeyframes.data(), mimeKeyframes.size());

    // Paste keyframe sequences
    if (mimeData->hasFormat(seqMime)) {
        QByteArray data = mimeData->data(seqMime);
        std::stringstream ss;
        for (auto d : data) ss << d;

        try {
            auto deserializer = app->getWorkspaceManager()->createWorkspaceDeserializer(ss, "");

            std::vector<std::string> trackClassIds;
            deserializer.deserialize("trackClassIds", trackClassIds, "id");

            // We need to deserialize each sequence and add it to a matching track.
            // The sequences need to be deserialized within the context of a compatible track.
            // We use clone() via serialize/deserialize roundtrip by finding a matching track.

            // For each sequence, find a compatible track (same classIdentifier) and add to it
            size_t seqIndex = 0;
            deserializer.deserializeRange("sequences", "sequence", [&](Deserializer& d, size_t) {
                if (seqIndex >= trackClassIds.size()) return;

                const auto& trackClassId = trackClassIds[seqIndex];
                // Find a matching track in the animation
                Track* targetTrack = nullptr;
                for (auto& track : animation) {
                    if (track.getClassIdentifier() == trackClassId) {
                        targetTrack = &track;
                        break;
                    }
                }

                if (targetTrack && targetTrack->size() > 0) {
                    // Clone an existing sequence to get the right concrete type,
                    // then deserialize the copied data into it.
                    auto seq = std::unique_ptr<KeyframeSequence>((*targetTrack)[0].clone());
                    seq->deserialize(d);
                    targetTrack->add(std::move(seq));
                }
                ++seqIndex;
            });
            showText_("Pasted keyframe sequences", std::chrono::milliseconds{1000});
        } catch (const Exception& e) {
            showText_(fmt::format("Paste failed: {}", e.getMessage()),
                      std::chrono::milliseconds{3000});
        }
        return;
    }

    // Paste keyframes
    if (mimeData->hasFormat(kfMime)) {
        QByteArray data = mimeData->data(kfMime);
        std::stringstream ss;
        for (auto d : data) ss << d;

        try {
            auto deserializer = app->getWorkspaceManager()->createWorkspaceDeserializer(ss, "");

            std::vector<std::string> trackClassIds;
            deserializer.deserialize("trackClassIds", trackClassIds, "id");

            size_t kfIndex = 0;
            deserializer.deserializeRange("keyframes", "keyframe", [&](Deserializer& d, size_t) {
                if (kfIndex >= trackClassIds.size()) return;

                const auto& trackClassId = trackClassIds[kfIndex];
                // Find a matching track
                Track* targetTrack = nullptr;
                for (auto& track : animation) {
                    if (track.getClassIdentifier() == trackClassId) {
                        targetTrack = &track;
                        break;
                    }
                }

                if (targetTrack && targetTrack->size() > 0) {
                    // Clone a keyframe from the track to get the right concrete type,
                    // then deserialize the copied data into it
                    auto& firstSeq = (*targetTrack)[0];
                    auto kf = std::unique_ptr<Keyframe>(firstSeq[0].clone());
                    kf->deserialize(d);
                    firstSeq.add(std::move(kf));
                }
                ++kfIndex;
            });
            showText_("Pasted keyframes", std::chrono::milliseconds{1000});
        } catch (const Exception& e) {
            showText_(fmt::format("Paste failed: {}", e.getMessage()),
                      std::chrono::milliseconds{3000});
        }
        return;
    }
}

void AnimationEditorQt::cut() {
    copy();
    // Delete the selected items (same logic as Key_Delete)
    QList<QGraphicsItem*> itemList = selectedItems();
    for (auto& elem : itemList) {
        if (auto keyqt = qgraphicsitem_cast<KeyframeWidgetQt*>(elem)) {
            controller_.getAnimation().remove(&(keyqt->getKeyframe()));
        } else if (auto seqqt = qgraphicsitem_cast<KeyframeSequenceWidgetQt*>(elem)) {
            controller_.getAnimation().remove(&(seqqt->getKeyframeSequence()));
        }
    }
}

void AnimationEditorQt::dragEnterEvent(QGraphicsSceneDragDropEvent* event) {
    // Only accept PropertyWidgets from a processor
    auto source = dynamic_cast<PropertyWidget*>(event->source());
    event->setAccepted(source != nullptr && source->getProperty() != nullptr &&
                       source->getProperty()->getOwner()->getProcessor() != nullptr);
}

void AnimationEditorQt::dragLeaveEvent(QGraphicsSceneDragDropEvent*) {
    if (dropIndicatorLine) dropIndicatorLine->setVisible(false);
    showText_("", std::chrono::milliseconds{0});
}

void AnimationEditorQt::dragMoveEvent(QGraphicsSceneDragDropEvent* event) {
    // Must override for drop events to occur. Do not call QGraphicsScene::dragMoveEvent

    // Indicate position
    if (dropIndicatorLine) {
        QGraphicsView* pView = views().empty() ? nullptr : views().first();
        const qreal snapX =
            getSnapTime(event->scenePos().x(), pView ? pView->transform().m11() : 1);
        dropIndicatorLine->setLine(snapX, 0, snapX, pView ? pView->height() : height());
        dropIndicatorLine->setVisible(true);
    }

    // Indicate insertion mode: keyframe or keyframe sequence.
    showText_((event->modifiers() & Qt::ControlModifier)
                  ? "Insert new keyframe sequence (Alt for non-snapping time)"
                  : "Insert new keyframe (Ctrl for sequence, Alt for non-snapping time)",
              std::chrono::milliseconds{1000});

    event->accept();
}

void AnimationEditorQt::dropEvent(QGraphicsSceneDragDropEvent* event) {

    // Switch off drag&drop indicator
    if (dropIndicatorLine) dropIndicatorLine->setVisible(false);

    // Drop it into the scene
    auto source = dynamic_cast<PropertyWidget*>(event->source());
    if (source) {
        auto property = source->getProperty();

        // Get time
        QGraphicsView* pView = views().empty() ? nullptr : views().first();
        const qreal snapX =
            getSnapTime(event->scenePos().x(), pView ? pView->transform().m11() : 1);
        const qreal time = snapX / static_cast<double>(widthPerSecond);

        if (event->modifiers() & Qt::ControlModifier) {
            controller_.getAnimation().addKeyframeSequence(property, Seconds(time));
        } else {
            controller_.getAnimation().addKeyframe(property, Seconds(time));
        }

        event->acceptProposedAction();
    }
}

void AnimationEditorQt::updateSceneRect() {
    setSceneRect(
        0.0, 0.0,
        static_cast<double>(controller_.getAnimation().getLastTime().count() * widthPerSecond) +
            5 * widthPerSecond,
        static_cast<double>(controller_.getAnimation().size() * trackHeight));
}

void AnimationEditorQt::onFirstMoved() { updateSceneRect(); }

void AnimationEditorQt::onLastMoved() { updateSceneRect(); }

}  // namespace animation

}  // namespace inviwo
