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
#include <modules/animation/factories/trackfactory.h>
#include <modules/animationqt/factories/trackwidgetqtfactory.h>
#include <modules/animationqt/widgets/editorconstants.h>
#include <modules/animationqt/widgets/keyframesequencewidgetqt.h>
#include <modules/animationqt/widgets/keyframewidgetqt.h>
#include <modules/animationqt/widgets/trackwidgetqt.h>
#include <modules/qtwidgets/textlabeloverlay.h>
#include <modules/qtwidgets/inviwoqtutils.h>

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
#include <QMenu>
#include <Qt>
#include <QtGlobal>
#include <fmt/core.h>

class QGraphicsSceneDragDropEvent;

namespace inviwo {

namespace animation {

AnimationEditorQt::AnimationEditorQt(
    AnimationController& controller, TrackWidgetQtFactory& widgetFactory,
    TrackFactory& trackFactory,
    std::function<void(std::string_view, std::chrono::milliseconds)> showText)
    : QGraphicsScene()
    , controller_(controller)
    , widgetFactory_{widgetFactory}
    , trackFactory_{trackFactory}
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
        deleteSelection();
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

template <typename T>
T* findParentOfType(QGraphicsItem* item) {
    for (QGraphicsItem* parent = item->parentItem(); parent; parent = parent->parentItem()) {
        if (auto casted = dynamic_cast<T*>(parent)) {
            return casted;
        }
    }
    return nullptr;
}

}  // namespace

void AnimationEditorQt::copy() {
    const auto selection = selectedItems();
    if (selection.empty()) return;

    // Classify selected items: collect keyframes, sequences, and tracks.
    // Prefer the highest-level selection: if a sequence is selected, don't also copy its keyframes.
    std::set<Track*> selectedTracks;
    std::vector<std::pair<Track*, Keyframe*>> keyframes;
    std::vector<std::pair<Track*, KeyframeSequence*>> sequences;
    std::set<KeyframeSequenceWidgetQt*> selectedSequences;

    for (auto* elem : selection) {
        if (auto* seqqt = qgraphicsitem_cast<KeyframeSequenceWidgetQt*>(elem)) {
            if (auto* track = findParentOfType<TrackWidgetQt>(elem)) {
                selectedTracks.insert(&track->getTrack());
                selectedSequences.insert(seqqt);
                sequences.emplace_back(&track->getTrack(), &seqqt->getKeyframeSequence());
            }
        }
    }

    for (auto* elem : selection) {
        if (auto* keyqt = qgraphicsitem_cast<KeyframeWidgetQt*>(elem)) {
            auto* track = findParentOfType<TrackWidgetQt>(elem);
            if (!track) continue;

            auto* seqqt = findParentOfType<KeyframeSequenceWidgetQt>(keyqt);
            if (!seqqt) continue;

            // Skip keyframes whose parent sequence is already selected
            if (selectedSequences.contains(seqqt)) continue;

            keyframes.emplace_back(&track->getTrack(), &keyqt->getKeyframe());
        }
    }

    auto& animation = controller_.getAnimation();
    auto mimeData = std::make_unique<QMimeData>();

    // Serialize keyframe sequences
    if (!sequences.empty()) {
        Serializer serializer("");

        serializer.serializeRange(
            "sequences", "item", sequences, [&](Serializer& nested, const auto& item) {
                const auto [track, sequence] = item;
                auto it =
                    std::ranges::find_if(animation, [&](const auto& t) { return &t == track; });
                const auto idx = std::distance(animation.begin(), it);
                nested.serialize("trackCId", item.first->getClassIdentifier(),
                                 SerializationTarget::Attribute);
                nested.serialize("trackIdx", idx, SerializationTarget::Attribute);
                nested.serialize("sequence", *item.second);
            });

        std::pmr::string str;
        serializer.write(str);
        const QByteArray byteArray(str.c_str(), static_cast<int>(str.length()));
        mimeData->setData(utilqt::toQString(mimeKeyframeSequences), byteArray);
    }

    // Serialize individual keyframes
    if (!keyframes.empty()) {
        Serializer serializer("");

        serializer.serializeRange(
            "keyframes", "item", keyframes, [&](Serializer& nested, const auto& item) {
                const auto [track, keyframe] = item;
                auto it =
                    std::ranges::find_if(animation, [&](const auto& t) { return &t == track; });
                const auto idx = std::distance(animation.begin(), it);
                nested.serialize("trackCId", item.first->getClassIdentifier(),
                                 SerializationTarget::Attribute);
                nested.serialize("trackIdx", idx, SerializationTarget::Attribute);
                nested.serialize("keyframe", *item.second);
            });

        std::pmr::string str;
        serializer.write(str);
        const QByteArray byteArray(str.c_str(), static_cast<int>(str.length()));
        mimeData->setData(utilqt::toQString(mimeKeyframes), byteArray);
    }

    // Only set clipboard if we have content
    if (!mimeData->formats().empty()) {
        QApplication::clipboard()->setMimeData(mimeData.release());
        showText_("Copied to clipboard", std::chrono::milliseconds{1000});
    }
}

void AnimationEditorQt::paste(std::optional<QPointF> scenePos, QGraphicsView* view) {
    const auto targetTime = [&]() {
        if (!scenePos) {
            return controller_.getCurrentTime();
        } else {
            const auto snapX = getSnapTime(scenePos->x(), view ? view->transform().m11() : 1);
            return scenePosToTime(snapX);
        }
    }();

    auto* clipboard = QApplication::clipboard();
    const auto* mimeData = clipboard->mimeData();
    if (!mimeData) return;

    auto* app = controller_.getInviwoApplication();
    auto& animation = controller_.getAnimation();

    const auto seqMime = utilqt::toQString(mimeKeyframeSequences);
    const auto kfMime = utilqt::toQString(mimeKeyframes);

    const auto findTrack = [&](size_t idx, std::string_view classId) -> Track* {
        if (scenePos) {
            const auto index = static_cast<size_t>(scenePos->y() / trackHeight);
            if (index < animation.size()) {
                auto& track = animation[index];
                if (track.getClassIdentifier() == classId) {
                    return &track;
                }
            }
        }

        if (idx < animation.size()) {
            auto& track = animation[idx];
            if (track.getClassIdentifier() == classId) {
                return &track;
            }
        }

        // Fallback: find the first track with a matching classId
        for (auto& track : animation) {
            if (track.getClassIdentifier() == classId) {
                return &track;
            }
        }
        return nullptr;
    };

    // Paste keyframe sequences
    if (mimeData->hasFormat(seqMime)) {
        const QByteArray data = mimeData->data(seqMime);
        const std::pmr::string xml{data.constData(), static_cast<size_t>(data.length())};

        try {
            auto [deserializer, info] =
                app->getWorkspaceManager()->createWorkspaceDeserializerAndInfo(xml, {});

            deserializer.deserializeRange(
                "sequences", "item", [&](Deserializer& nested, size_t index) {
                    std::string trackCId{};
                    size_t trackIdx{};
                    nested.deserialize("trackCId", trackCId, SerializationTarget::Attribute);
                    nested.deserialize("trackIdx", trackIdx, SerializationTarget::Attribute);

                    if (auto seq = trackFactory_.keyframeSequenceFactory.create(trackCId)) {
                        nested.deserialize("sequence", *seq);

                        if (auto* track = findTrack(trackIdx, trackCId)) {
                            const auto timeOffset = targetTime - seq->getFirstTime();
                            for (size_t i = seq->size(); i > 0; i--) {
                                (*seq)[i - 1].setTime((*seq)[i - 1].getTime() + timeOffset);
                            }
                            track->add(std::move(seq));
                        }
                    }
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
        const QByteArray data = mimeData->data(kfMime);
        const std::pmr::string xml{data.constData(), static_cast<size_t>(data.length())};

        try {
            auto [deserializer, info] =
                app->getWorkspaceManager()->createWorkspaceDeserializerAndInfo(xml, {});
            deserializer.deserializeRange(
                "keyframes", "item", [&](Deserializer& nested, size_t index) {
                    std::string trackCId{};
                    size_t trackIdx{};
                    nested.deserialize("trackCId", trackCId, SerializationTarget::Attribute);
                    nested.deserialize("trackIdx", trackIdx, SerializationTarget::Attribute);

                    if (auto kf = trackFactory_.keyframeFactory.create(trackCId)) {
                        nested.deserialize("keyframe", *kf);

                        if (auto* track = findTrack(trackIdx, trackCId)) {
                            kf->setTime(targetTime);
                            track->addToClosestSequence(std::move(kf));
                        }
                    }
                });

            showText_("Pasted keyframes", std::chrono::milliseconds{1000});
        } catch (const Exception& e) {
            showText_(fmt::format("Paste failed: {}", e.getMessage()),
                      std::chrono::milliseconds{3000});
        }
        return;
    }
}

void AnimationEditorQt::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
    const auto pos = e->scenePos();
    const auto selection = selectedItems();
    auto* view = dynamic_cast<QGraphicsView*>(e->widget());

    QMenu menu{};

    {
        auto* cutAction = menu.addAction(QIcon(":/svgicons/edit-cut.svg"), "Cu&t");
        cutAction->setEnabled(!selection.empty());
        connect(cutAction, &QAction::triggered, this, [this]() { cut(); });

        auto* copyAction = menu.addAction(QIcon(":/svgicons/edit-copy.svg"), "&Copy");
        copyAction->setEnabled(!selection.empty());
        connect(copyAction, &QAction::triggered, this, [this]() { copy(); });

        auto* pasteAction = menu.addAction(QIcon(":/svgicons/edit-paste.svg"), "&Paste");
        auto* clipboard = QApplication::clipboard();
        pasteAction->setEnabled(
            clipboard->mimeData() &&
            (clipboard->mimeData()->hasFormat(QString::fromUtf8(mimeKeyframes)) ||
             clipboard->mimeData()->hasFormat(QString::fromUtf8(mimeKeyframeSequences))));
        connect(pasteAction, &QAction::triggered, this, [this, pos, view]() { paste(pos, view); });

        auto* selectAllAction =
            menu.addAction(QIcon(":/svgicons/edit-selectall.svg"), tr("&Select All"));
        connect(selectAllAction, &QAction::triggered, this, [this]() { selectAll(); });
    }
    menu.addSeparator();
    {

        auto* deleteAction = menu.addAction("&Delete");
        auto* clearAction = menu.addAction("&Clear");

        deleteAction->setEnabled(!selection.empty());
        connect(deleteAction, &QAction::triggered, this,
                [this, selection]() { deleteItems(selection); });

        connect(clearAction, &QAction::triggered, this, [this]() { deleteItems(items()); });
    }

    if (menu.exec(e->screenPos())) {
        e->accept();
    }
}

void AnimationEditorQt::cut() {
    copy();
    // Delete the selected items (same logic as Key_Delete)
    const QList<QGraphicsItem*> itemList = selectedItems();
    for (auto& elem : itemList) {
        if (auto* keyqt = qgraphicsitem_cast<KeyframeWidgetQt*>(elem)) {
            controller_.getAnimation().remove(&(keyqt->getKeyframe()));
        } else if (auto* seqqt = qgraphicsitem_cast<KeyframeSequenceWidgetQt*>(elem)) {
            controller_.getAnimation().remove(&(seqqt->getKeyframeSequence()));
        }
    }
}

void AnimationEditorQt::deleteItems(const QList<QGraphicsItem*>& itemsList) {
    for (auto& elem : itemsList) {
        if (auto* keyqt = qgraphicsitem_cast<KeyframeWidgetQt*>(elem)) {
            controller_.getAnimation().remove(&(keyqt->getKeyframe()));
        } else if (auto* seqqt = qgraphicsitem_cast<KeyframeSequenceWidgetQt*>(elem)) {
            controller_.getAnimation().remove(&(seqqt->getKeyframeSequence()));
        }
    }
}

void AnimationEditorQt::deleteSelection() { deleteItems(selectedItems()); }

void AnimationEditorQt::selectAll() {
    std::ranges::for_each(items(), [](auto* item) { item->setSelected(true); });
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
        const auto time = scenePosToTime(snapX);
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
