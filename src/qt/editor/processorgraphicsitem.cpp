/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
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
    static constexpr int labelHeight_ = 8;
    static constexpr int labelMargin_ = 8;
    

    setZValue(PROCESSORGRAPHICSITEM_DEPTH);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);
    setRect(-size_.width() / 2, -size_.height() / 2, size_.width(), size_.height());

    {
        nameLabel_ =
            new LabelGraphicsItem(this, size_.width() - 2 * labelHeight_ - 10, Qt::AlignBottom);
        nameLabel_->setPos(QPointF(rect().left() + labelMargin_, -3));
        nameLabel_->setDefaultTextColor(Qt::white);
        QFont nameFont("Segoe", labelHeight_, QFont::Black, false);
        nameFont.setPixelSize(pointSizeToPixelSize(labelHeight_));
        nameLabel_->setFont(nameFont);
        nameLabel_->setText(QString::fromStdString(processor_->getIdentifier()));
        LabelGraphicsItemObserver::addObservation(nameLabel_);
    }
    {
        classLabel_ = new LabelGraphicsItem(this, size_.width() - 2 * labelHeight_, Qt::AlignTop);
        classLabel_->setPos(QPointF(rect().left() + labelMargin_, -3));
        classLabel_->setDefaultTextColor(Qt::lightGray);
        QFont classFont("Segoe", labelHeight_, QFont::Normal, true);
        classFont.setPixelSize(pointSizeToPixelSize(labelHeight_));
        classLabel_->setFont(classFont);
        classLabel_->setText(QString::fromStdString(processor_->getDisplayName() + " " +
                                                    processor_->getTags().getString()));
    }
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
        
    if (auto activityInd = dynamic_cast<ActivityIndicatorOwner*>(processor_)){
        activityInd->getActivityIndicator().addObserver(statusItem_);
    }
    
    #if IVW_PROFILING
    {
        countLabel_ = new LabelGraphicsItem(this, 100, Qt::AlignRight | Qt::AlignBottom);
        countLabel_->setPos(rect().bottomRight() + QPointF(-5.0, 0.0));
        countLabel_->setDefaultTextColor(Qt::lightGray);
        QFont font("Segoe", labelHeight_, QFont::Normal, false);
        font.setPixelSize(pointSizeToPixelSize(labelHeight_));
        countLabel_->setFont(font);
    }
    #endif
    
    setVisible(processorMeta_->isVisible());
    setSelected(processorMeta_->isSelected());
    setPos(QPointF(processorMeta_->getPosition().x, processorMeta_->getPosition().y));
}

QPointF ProcessorGraphicsItem::portPosition(PortType type, size_t index) {
    const QPointF offset = {12.5f, (type == PortType::In ? 1.0f : -1.0f) * 4.5f};
    const QPointF delta = {12.5f, 0.0f};
    const QPointF rowDelta = {0.0f, (type == PortType::In ? -1.0f : 1.0f) * 12.5f};
    const size_t portsPerRow = 10;

    return (type == PortType::In ? rect().topLeft() : rect().bottomLeft()) + offset +
           rowDelta * static_cast<qreal>(index / portsPerRow) +
           delta * static_cast<qreal>(index % portsPerRow);
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

ProcessorInportGraphicsItem* ProcessorGraphicsItem::getInportGraphicsItem(Inport* port) {
    return inportItems_[port];
}
ProcessorOutportGraphicsItem* ProcessorGraphicsItem::getOutportGraphicsItem(Outport* port) {
    return outportItems_[port];
}

ProcessorGraphicsItem::~ProcessorGraphicsItem() = default;

inviwo::Processor* ProcessorGraphicsItem::getProcessor() const {
    return processor_;
}

void ProcessorGraphicsItem::editProcessorName() {
    setFocus();
    nameLabel_->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    QTextCursor cur = nameLabel_->textCursor();
    cur.select(QTextCursor::Document);
    nameLabel_->setTextCursor(cur);
    nameLabel_->setTextInteractionFlags(Qt::TextEditorInteraction);
    nameLabel_->setFocus();
    nameLabel_->setSelected(true);
}

void ProcessorGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                                  QWidget* widget) {
    IVW_UNUSED_PARAM(options);
    IVW_UNUSED_PARAM(widget);
    const float roundedCorners = 9.0f;
    
    p->save();
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing, true);
    QColor topColor(140, 140, 140);
    QColor middleColor(59, 61, 61);
    QColor bottomColor(40, 40, 40);
    
    p->setBrush(middleColor);
    p->setPen(QPen(QBrush(isSelected() ? Qt::darkRed : bottomColor), 2.0));
    
    p->drawRoundedRect(rect(), roundedCorners, roundedCorners);

    p->restore();
}

bool ProcessorGraphicsItem::isEditingProcessorName() {
    return (nameLabel_->textInteractionFlags() == Qt::TextEditorInteraction);
}

void ProcessorGraphicsItem::setIdentifier(QString text) {
    std::string oldName = getProcessor()->getIdentifier();
    std::string newName = text.toLocal8Bit().constData();

    if (oldName == newName) return;

    if (newName.size() == 0) {
        nameLabel_->setText(oldName.c_str());
        return;
    }
    std::string updatedNewName;
    try {
        updatedNewName = getProcessor()->setIdentifier(newName);
    } catch (Exception& e) {
        updatedNewName = getProcessor()->getIdentifier();
        nameLabel_->setText(updatedNewName.c_str());
        LogWarn(e.getMessage());
        return;
    }
    if (updatedNewName != newName) {
        nameLabel_->setText(updatedNewName.c_str());
    }

    if (auto* widget = dynamic_cast<ProcessorWidgetQt*>(getProcessor()->getProcessorWidget())) {
        widget->setWindowTitle(updatedNewName.c_str());
    }
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
        case QGraphicsItem::ItemPositionHasChanged:
            if (processorMeta_) processorMeta_->setPosition(ivec2(x(), y()));
            break;
        case QGraphicsItem::ItemSelectedHasChanged:
            updateWidgets();
            if (!highlight_ && processorMeta_) processorMeta_->setSelected(isSelected());
            break;
        case QGraphicsItem::ItemVisibleHasChanged:
            if (processorMeta_) processorMeta_->setVisible(isVisible());
            break;
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
                editor->addPropertyWidgets(getProcessor());
                editor->showProecssorHelp(getProcessor()->getClassIdentifier());
            }
        }
    } else {
        setZValue(PROCESSORGRAPHICSITEM_DEPTH);
        if (auto editor = getNetworkEditor()) {
            editor->removePropertyWidgets(getProcessor());
        }
    }
}

void ProcessorGraphicsItem::onLabelGraphicsItemChange() {
    if (nameLabel_->isFocusOut()) {
        setIdentifier(nameLabel_->text());
        nameLabel_->setNoFocusOut();
    }
}

std::string ProcessorGraphicsItem::getIdentifier() const { return processor_->getIdentifier(); }

ProcessorLinkGraphicsItem* ProcessorGraphicsItem::getLinkGraphicsItem() const { return linkItem_; }

void ProcessorGraphicsItem::onProcessorIdentifierChange(Processor* processor) {
    std::string newIdentifier = processor->getIdentifier();
    
    if (newIdentifier != nameLabel_->text().toUtf8().constData()) {
        nameLabel_->setText(QString::fromStdString(newIdentifier));
    }

    ProcessorWidgetQt* processorWidgetQt =
        dynamic_cast<ProcessorWidgetQt*>(getProcessor()->getProcessorWidget());

    if (processorWidgetQt) processorWidgetQt->setWindowTitle(QString::fromStdString(newIdentifier));
}

void ProcessorGraphicsItem::onProcessorPortAdded(Processor *, Port *port){
    Inport *inport = dynamic_cast<Inport*>(port);
    Outport *outport = dynamic_cast<Outport*>(port);
    if(inport) addInport(inport);
    else if(outport) addOutport(outport);
}

void ProcessorGraphicsItem::onProcessorPortRemoved(Processor*, Port* port) {
    Inport *inport = dynamic_cast<Inport*>(port);
    Outport *outport = dynamic_cast<Outport*>(port);
    if (inport) removeInport(inport);
    else if (outport) removeOutport(outport);
}

#if IVW_PROFILING
void ProcessorGraphicsItem::onProcessorAboutToProcess(Processor*) {
    processCount_++;
    countLabel_->setText(QString::number(processCount_));
    clock_.start();
}
void ProcessorGraphicsItem::onProcessorFinishedProcess(Processor*) {
    clock_.tick();
    evalTime_ = clock_.getElapsedMiliseconds();
    maxEvalTime_ = maxEvalTime_ < evalTime_ ? evalTime_ : maxEvalTime_;
    totEvalTime_ += evalTime_;
}
void ProcessorGraphicsItem::resetTimeMeasurements(){
    processCount_ = 0;
    countLabel_->setText("0");
    maxEvalTime_ = 0.0;
    evalTime_ = 0.0;
    totEvalTime_ = 0.0;
}
#endif

ProcessorStatusGraphicsItem* ProcessorGraphicsItem::getStatusItem() const {
    return statusItem_;
}

void ProcessorGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("b", processor_->getDisplayName(), { {"style", "color:white;"} });
    utildoc::TableBuilder tb(b, P::end());

    tb(H("Identifier"), processor_->getIdentifier());
    tb(H("Class"), processor_->getClassIdentifier());
    tb(H("Category"), processor_->getCategory());
    tb(H("Code"), Processor::getCodeStateString(processor_->getCodeState()));
    tb(H("Tags"), processor_->getTags().getString());

#if IVW_PROFILING
    tb(H("Ready"), processor_->isReady()?"Yes":"No");
    tb(H("Eval Count"), processCount_);
    tb(H("Eval Time"), evalTime_);
    tb(H("Mean Time"), totEvalTime_/ std::max(static_cast<double>(processCount_), 1.0));
    tb(H("Max Time"), maxEvalTime_);
#endif

    showToolTipHelper(e, utilqt::toLocalQString(doc));
}

void ProcessorGraphicsItem::setHighlight(bool val) {
    highlight_ = val;
    setSelected(val);
}

}  // namespace
