/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/processors/activityindicator.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/clock.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/stdextensions.h>

#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/processorlinkgraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/processorprogressgraphicsitem.h>
#include <inviwo/qt/editor/processorstatusgraphicsitem.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <modules/qtwidgets/propertylistwidget.h>
#include <modules/qtwidgets/processors/processorwidgetqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/qstringhelper.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QVector2D>
#include <QTextCursor>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>
#include <warn/pop>

namespace inviwo {

const QSizeF ProcessorGraphicsItem::size_ = {150.f, 50.f};

int pointSizeToPixelSize(const int pointSize) {
    // compute pixel size for fonts by assuming 96 dpi as basis
    return ((pointSize * 4) / 3);
}

ProcessorGraphicsItem::ProcessorGraphicsItem(Processor* processor)
    : ProcessorObserver()
    , LabelGraphicsItemObserver()
    , processor_(processor)
    , processorMeta_(nullptr)
    , progressItem_(nullptr)
    , statusItem_(nullptr)
    , linkItem_(nullptr)
    , highlight_(false)
#if IVW_PROFILING
    , processCount_(0)
    , countLabel_(nullptr)
    , maxEvalTime_(0.0)
    , evalTime_(0.0)
    , totEvalTime_(0.0)
#endif
{
    static constexpr int labelHeight = 8;
    auto width = static_cast<int>(size_.width());

    setZValue(PROCESSORGRAPHICSITEM_DEPTH);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);
    setRect(-size_.width() / 2, -size_.height() / 2, size_.width(), size_.height());

    {
        displayNameLabel_ =
            new LabelGraphicsItem(this, width - 2 * labelHeight - 10, Qt::AlignBottom);
        displayNameLabel_->setDefaultTextColor(Qt::white);
        QFont nameFont("Segoe", labelHeight, QFont::Black, false);
        nameFont.setPixelSize(pointSizeToPixelSize(labelHeight));
        displayNameLabel_->setFont(nameFont);
        displayNameLabel_->setText(utilqt::toQString(processor_->getDisplayName()));
        LabelGraphicsItemObserver::addObservation(displayNameLabel_);
    }
    {
        identifierLabel_ = new LabelGraphicsItem(this, width - 2 * labelHeight, Qt::AlignTop);
        identifierLabel_->setDefaultTextColor(Qt::lightGray);
        QFont classFont("Segoe", labelHeight, QFont::Normal, false);
        classFont.setPixelSize(pointSizeToPixelSize(labelHeight));
        identifierLabel_->setFont(classFont);
        identifierLabel_->setText(utilqt::toQString(processor_->getIdentifier()));
        LabelGraphicsItemObserver::addObservation(identifierLabel_);
    }
    {
        tagLabel_ = new LabelGraphicsItem(this, width / 2, Qt::AlignTop);
        tagLabel_->setDefaultTextColor(Qt::lightGray);
        QFont classFont("Segoe", labelHeight, QFont::Bold, false);
        classFont.setPixelSize(pointSizeToPixelSize(labelHeight));
        tagLabel_->setFont(classFont);
        tagLabel_->setText(
            utilqt::toQString(util::getPlatformTags(processor_->getTags()).getString()));
    }
    auto tagSize = tagLabel_->usedTextWidth();
    identifierLabel_->setCrop(width - 2 * labelHeight - (tagSize > 0 ? tagSize + 4 : 0));
    positionLablels();

    processor_->ProcessorObservable::addObserver(this);

    processorMeta_ = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
    processorMeta_->addObserver(this);

    linkItem_ = new ProcessorLinkGraphicsItem(this);

    for (auto& inport : processor_->getInports()) {
        addInport(inport);
    }
    for (auto& outport : processor_->getOutports()) {
        addOutport(outport);
    }

    statusItem_ = new ProcessorStatusGraphicsItem(this, processor_);
    statusItem_->setPos(rect().topRight() + QPointF(-9.0f, 9.0f));

    if (auto progressBarOwner = dynamic_cast<ProgressBarOwner*>(processor_)) {
        progressItem_ =
            new ProcessorProgressGraphicsItem(this, &(progressBarOwner->getProgressBar()));
        progressItem_->setPos(QPointF(0.0f, 9.0f));

        progressBarOwner->getProgressBar().ActivityIndicator::addObserver(statusItem_);
    }

    if (auto activityInd = dynamic_cast<ActivityIndicatorOwner*>(processor_)) {
        activityInd->getActivityIndicator().addObserver(statusItem_);
    }

#if IVW_PROFILING
    {
        countLabel_ = new LabelGraphicsItem(this, 100, Qt::AlignRight | Qt::AlignBottom);
        countLabel_->setPos(rect().bottomRight() + QPointF(-5.0, 0.0));
        countLabel_->setDefaultTextColor(Qt::lightGray);
        QFont font("Segoe", labelHeight, QFont::Normal, false);
        font.setPixelSize(pointSizeToPixelSize(labelHeight));
        countLabel_->setFont(font);
    }
#endif

    setVisible(processorMeta_->isVisible());
    setSelected(processorMeta_->isSelected());
    setPos(QPointF(processorMeta_->getPosition().x, processorMeta_->getPosition().y));
}

void ProcessorGraphicsItem::positionLablels() {
    static constexpr int labelMargin = 7;

    displayNameLabel_->setPos(QPointF(rect().left() + labelMargin, -2));
    identifierLabel_->setPos(QPointF(rect().left() + labelMargin, -3));

    auto offset = identifierLabel_->usedTextWidth();
    tagLabel_->setPos(QPointF(rect().left() + labelMargin + offset + 4, -3));
}

QPointF ProcessorGraphicsItem::portOffset(PortType type, size_t index) {
    const QPointF offset = {12.5f, (type == PortType::In ? 1.0f : -1.0f) * 4.5f};
    const QPointF delta = {12.5f, 0.0f};
    const QPointF rowDelta = {0.0f, (type == PortType::In ? -1.0f : 1.0f) * 12.5f};
    const size_t portsPerRow = 10;

    auto poffset = QPointF{-ProcessorGraphicsItem::size_.width() / 2,
                           (type == PortType::In ? -ProcessorGraphicsItem::size_.height() / 2
                                                 : ProcessorGraphicsItem::size_.height() / 2)};

    return poffset + offset + rowDelta * static_cast<qreal>(index / portsPerRow) +
           delta * static_cast<qreal>(index % portsPerRow);
}

QPointF ProcessorGraphicsItem::portPosition(PortType type, size_t index) {
    return rect().center() + portOffset(type, index);
}

void ProcessorGraphicsItem::addInport(Inport* port) {
    auto pos = portPosition(PortType::In, inportItems_.size());
    inportItems_[port] = new ProcessorInportGraphicsItem(this, pos, port);
}

void ProcessorGraphicsItem::addOutport(Outport* port) {
    auto pos = portPosition(PortType::Out, outportItems_.size());
    outportItems_[port] = new ProcessorOutportGraphicsItem(this, pos, port);
}

void ProcessorGraphicsItem::removeInport(Inport* port) {
    delete inportItems_[port];
    inportItems_.erase(port);

    size_t count = 0;
    for (auto& item : inportItems_) {
        item.second->setPos(portPosition(PortType::In, count));
        count++;
    }
    update();
}

void ProcessorGraphicsItem::removeOutport(Outport* port) {
    delete outportItems_[port];
    outportItems_.erase(port);
    size_t count = 0;
    for (auto& item : outportItems_) {
        item.second->setPos(portPosition(PortType::Out, count));
        count++;
    }
    update();
}

void ProcessorGraphicsItem::onProcessorMetaDataPositionChange() {
    auto ipos = processorMeta_->getPosition();
    auto qpos = QPointF(ipos.x, ipos.y);
    if (qpos != pos()) {
        setPos(qpos);
    }
}

void ProcessorGraphicsItem::onProcessorMetaDataVisibilityChange() {
    if (processorMeta_->isVisible() != isVisible()) {
        setVisible(processorMeta_->isVisible());
    }
}

void ProcessorGraphicsItem::onProcessorMetaDataSelectionChange() {
    if (processorMeta_->isSelected() != isSelected()) {
        setSelected(processorMeta_->isSelected());
    }
}

ProcessorInportGraphicsItem* ProcessorGraphicsItem::getInportGraphicsItem(Inport* port) const {
    return util::map_find_or_null(inportItems_, port);
}
ProcessorOutportGraphicsItem* ProcessorGraphicsItem::getOutportGraphicsItem(Outport* port) const {
    return util::map_find_or_null(outportItems_, port);
}

ProcessorGraphicsItem::~ProcessorGraphicsItem() = default;

Processor* ProcessorGraphicsItem::getProcessor() const { return processor_; }

void ProcessorGraphicsItem::editDisplayName() {
    setFocus();
    displayNameLabel_->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    QTextCursor cur = displayNameLabel_->textCursor();
    cur.movePosition(QTextCursor::End);
    displayNameLabel_->setTextCursor(cur);
    displayNameLabel_->setTextInteractionFlags(Qt::TextEditorInteraction);
    displayNameLabel_->setFocus();
    displayNameLabel_->setSelected(true);
}

void ProcessorGraphicsItem::editIdentifier() {
    setFocus();
    identifierLabel_->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    QTextCursor cur = identifierLabel_->textCursor();
    cur.movePosition(QTextCursor::End);
    identifierLabel_->setTextCursor(cur);
    identifierLabel_->setTextInteractionFlags(Qt::TextEditorInteraction);
    identifierLabel_->setFocus();
    identifierLabel_->setSelected(true);
}

void ProcessorGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                                  QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);
    const float roundedCorners = 9.0f;

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    QColor selectionColor("#7a191b");
    QColor deprecatedColor("#562e14");
    QColor backgroundColor("#3b3d3d");
    QColor borderColor("#282828");

    if (isSelected()) {
        p->setBrush(selectionColor);
    } else if (processor_->getProcessorInfo().codeState == CodeState::Deprecated) {
        p->setBrush(deprecatedColor);
    } else {
        p->setBrush(backgroundColor);
    }
    p->setPen(QPen(QBrush(borderColor), 2.0));

    p->drawRoundedRect(rect(), roundedCorners, roundedCorners);

    p->restore();
}

bool ProcessorGraphicsItem::isEditingProcessorName() {
    return (displayNameLabel_->textInteractionFlags() == Qt::TextEditorInteraction);
}

void ProcessorGraphicsItem::snapToGrid() {
    QPointF newpos = NetworkEditor::snapToGrid(pos());
    if (newpos != pos()) {
        setPos(newpos);
    }
}

QVariant ProcessorGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
#include <warn/push>
#include <warn/ignore/switch-enum>
    switch (change) {
        case QGraphicsItem::ItemPositionHasChanged: {
            QPointF newPos = value.toPointF();
            newPos.setX(round(newPos.x()));
            newPos.setY(round(newPos.y()));
            if (processorMeta_) {
                processorMeta_->setPosition(ivec2(newPos.x(), newPos.y()));
            }
            if (auto editor = qobject_cast<NetworkEditor*>(scene())) {
                editor->updateSceneSize();
            }
            return newPos;
        }
        case QGraphicsItem::ItemSelectedHasChanged:
            updateWidgets();
            if (!highlight_ && processorMeta_) processorMeta_->setSelected(isSelected());
            break;
        case QGraphicsItem::ItemVisibleHasChanged: {
            if (processorMeta_) {
                processorMeta_->setVisible(isVisible());
            }
            if (auto editor = qobject_cast<NetworkEditor*>(scene())) {
                editor->updateSceneSize();
            }
            break;
        }
        case QGraphicsItem::ItemSceneHasChanged:
            updateWidgets();
            break;
        default:
            break;
    }
#include <warn/pop>
    return QGraphicsItem::itemChange(change, value);
}

void ProcessorGraphicsItem::updateWidgets() {
    if (isSelected()) {
        setZValue(SELECTED_PROCESSORGRAPHICSITEM_DEPTH);
        if (!highlight_) {
            if (auto editor = getNetworkEditor()) {
                editor->addPropertyWidgets(processor_);
                editor->showProcessorHelp(processor_->getClassIdentifier());
            }
        }
    } else {
        setZValue(PROCESSORGRAPHICSITEM_DEPTH);
        if (auto editor = getNetworkEditor()) {
            editor->removePropertyWidgets(processor_);
        }
    }
}

void ProcessorGraphicsItem::onLabelGraphicsItemChanged(LabelGraphicsItem* item) {
    if (item == displayNameLabel_ && displayNameLabel_->isFocusOut()) {
        auto newName = utilqt::fromQString(displayNameLabel_->text());
        if (!newName.empty()) {
            processor_->setDisplayName(newName);
            displayNameLabel_->setNoFocusOut();
        }
    } else if (item == identifierLabel_ && identifierLabel_->isFocusOut()) {
        auto newId = utilqt::fromQString(identifierLabel_->text());
        if (!newId.empty()) {
            try {
                processor_->setIdentifier(newId);
                identifierLabel_->setNoFocusOut();
            } catch (Exception& e) {
                identifierLabel_->setText(utilqt::toQString(processor_->getIdentifier()));
                LogWarn(e.getMessage());
            }
            positionLablels();
        }
    }
}

void ProcessorGraphicsItem::onLabelGraphicsItemEdited(LabelGraphicsItem*) { positionLablels(); }

std::string ProcessorGraphicsItem::getIdentifier() const { return processor_->getIdentifier(); }

ProcessorLinkGraphicsItem* ProcessorGraphicsItem::getLinkGraphicsItem() const { return linkItem_; }

void ProcessorGraphicsItem::onProcessorIdentifierChanged(Processor* processor, const std::string&) {
    auto newIdentifier = utilqt::toQString(processor->getIdentifier());
    if (newIdentifier != displayNameLabel_->text()) {
        identifierLabel_->setText(newIdentifier);
    }
}

void ProcessorGraphicsItem::onProcessorDisplayNameChanged(Processor*, const std::string&) {
    auto newDisplayName = utilqt::toQString(processor_->getDisplayName());
    if (newDisplayName != displayNameLabel_->text()) {
        displayNameLabel_->setText(newDisplayName);
    }
}

void ProcessorGraphicsItem::onProcessorReadyChanged(Processor*) { statusItem_->update(); }

void ProcessorGraphicsItem::onProcessorPortAdded(Processor*, Port* port) {
    if (auto inport = dynamic_cast<Inport*>(port)) {
        addInport(inport);
    } else if (auto outport = dynamic_cast<Outport*>(port)) {
        addOutport(outport);
    }
}

void ProcessorGraphicsItem::onProcessorPortRemoved(Processor*, Port* port) {
    if (auto inport = dynamic_cast<Inport*>(port)) {
        removeInport(inport);
    } else if (auto outport = dynamic_cast<Outport*>(port)) {
        removeOutport(outport);
    }
}

#if IVW_PROFILING
void ProcessorGraphicsItem::onProcessorAboutToProcess(Processor*) {
    processCount_++;
    auto str =
        QStringHelper<decltype(processCount_)>::toLocaleString(QLocale::system(), processCount_);
    countLabel_->setText(str);
    clock_.reset();
    clock_.start();
}

void ProcessorGraphicsItem::onProcessorFinishedProcess(Processor*) {
    clock_.stop();
    evalTime_ = clock_.getElapsedMilliseconds();
    maxEvalTime_ = maxEvalTime_ < evalTime_ ? evalTime_ : maxEvalTime_;
    totEvalTime_ += evalTime_;
}

void ProcessorGraphicsItem::resetTimeMeasurements() {
    processCount_ = 0;
    countLabel_->setText("0");
    maxEvalTime_ = 0.0;
    evalTime_ = 0.0;
    totEvalTime_ = 0.0;
}
#endif

ProcessorStatusGraphicsItem* ProcessorGraphicsItem::getStatusItem() const { return statusItem_; }

void ProcessorGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc;
    auto b = doc.append("html").append("body");
    b.append("b", processor_->getDisplayName(), {{"style", "color:white;"}});
    utildoc::TableBuilder tb(b, P::end());

    tb(H("Identifier"), processor_->getIdentifier());
    tb(H("Class"), processor_->getClassIdentifier());
    tb(H("Category"), processor_->getCategory());
    tb(H("Code"), processor_->getCodeState());
    tb(H("Tags"), processor_->getTags().getString());
    tb(H("Ready"), processor_->isReady() ? "Yes" : "No");
    tb(H("Source"), processor_->isSource() ? "Yes" : "No");
    tb(H("Sink"), processor_->isSink() ? "Yes" : "No");

#if IVW_PROFILING
    tb(H("Eval Count"), processCount_);
    tb(H("Eval Time"), msToString(evalTime_, true, true));
    tb(H("Mean Time"),
       msToString(totEvalTime_ / std::max(static_cast<double>(processCount_), 1.0), true, true));
    tb(H("Max Time"), msToString(maxEvalTime_, true, true));
#endif

    showToolTipHelper(e, utilqt::toLocalQString(doc));
}

void ProcessorGraphicsItem::setHighlight(bool val) { highlight_ = val; }

}  // namespace inviwo
