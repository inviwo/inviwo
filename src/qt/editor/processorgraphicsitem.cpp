/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <inviwo/core/util/tooltiphelper.h>

#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/processorlinkgraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/processorprogressgraphicsitem.h>
#include <inviwo/qt/editor/processorstatusgraphicsitem.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/widgets/propertylistwidget.h>
#include <inviwo/qt/widgets/processors/processorwidgetqt.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>

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

static const int width = 150;
static const int height = 50;
static const int roundedCorners = 9;
static const int labelHeight = 8;

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

    setZValue(PROCESSORGRAPHICSITEM_DEPTH);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setRect(-width / 2, -height / 2, width, height);
    QGraphicsDropShadowEffect* processorShadowEffect = new QGraphicsDropShadowEffect();
    processorShadowEffect->setOffset(3.0);
    processorShadowEffect->setBlurRadius(3.0);
    setGraphicsEffect(processorShadowEffect);
    nameLabel_ = new LabelGraphicsItem(this);
    nameLabel_->setCrop(9, 8);
    nameLabel_->setPos(-width / 2.0 + labelHeight, -height / 2.0 + 0.6 * labelHeight);
    nameLabel_->setDefaultTextColor(Qt::white);
    QFont nameFont("Segoe", labelHeight, QFont::Black, false);
    nameFont.setPixelSize(pointSizeToPixelSize(labelHeight));
    nameLabel_->setFont(nameFont);
    LabelGraphicsItemObserver::addObservation(nameLabel_);
    classLabel_ = new LabelGraphicsItem(this);
    classLabel_->setCrop(9, 8);
    classLabel_->setPos(-width / 2.0 + labelHeight, -height / 2.0 + labelHeight * 2.0);
    classLabel_->setDefaultTextColor(Qt::lightGray);
    QFont classFont("Segoe", labelHeight, QFont::Normal, true);
    classFont.setPixelSize(pointSizeToPixelSize(labelHeight));
    classLabel_->setFont(classFont);

    nameLabel_->setText(QString::fromStdString(processor_->getIdentifier()));
    classLabel_->setText(QString::fromStdString(processor_->getDisplayName() + " " 
        + processor_->getTags().getString()));
    processor_->ProcessorObservable::addObserver(this);

    processorMeta_ = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);

    linkItem_ = new ProcessorLinkGraphicsItem(this);


    std::vector<Inport*> inports = processor_->getInports();
    std::vector<Outport*> outports = processor_->getOutports();

    inportX = rect().left() + 12.5f;
    inportY = rect().top() + 4.5f;
    outportX = rect().left() + 12.5f;
    outportY = rect().bottom() - 4.5f;

    for (auto& inport : inports) {
        addInport(inport);
    }

    for (auto& outport : outports) {
        addOutport(outport);
    }

    statusItem_ = new ProcessorStatusGraphicsItem(this, processor_);
    if (auto progressBarOwner = dynamic_cast<ProgressBarOwner*>(processor_)) {
        progressItem_ =
            new ProcessorProgressGraphicsItem(this, &(progressBarOwner->getProgressBar()));

        progressBarOwner->getProgressBar().ActivityIndicator::addObserver(statusItem_);
    }
        
    
    
    if (auto activityInd = dynamic_cast<ActivityIndicatorOwner*>(processor_)){
        activityInd->getActivityIndicator().addObserver(statusItem_);
    }

    #if IVW_PROFILING
    countLabel_ = new LabelGraphicsItem(this);
    countLabel_->setCrop(9,8);
    countLabel_->setPos(rect().left() + labelHeight, height / 2 - labelHeight*2.5);
    countLabel_->setDefaultTextColor(Qt::lightGray);
    countLabel_->setFont(classFont);
    countLabel_->setTextWidth(width - 2*labelHeight);
    #endif
}


void ProcessorGraphicsItem::addInport(Inport *port){
    inportItems_[port] = new ProcessorInportGraphicsItem(this, QPointF(inportX,inportY), port);
    inportX += (25 / 2.0);
}

void ProcessorGraphicsItem::addOutport(Outport *port){
    outportItems_[port] =
        new ProcessorOutportGraphicsItem(this, QPointF(outportX, outportY), port);
    outportX += (25 / 2.0);
}


ProcessorInportGraphicsItem* ProcessorGraphicsItem::getInportGraphicsItem(Inport* port) {
    return inportItems_[port];
}
ProcessorOutportGraphicsItem* ProcessorGraphicsItem::getOutportGraphicsItem(Outport* port) {
    return outportItems_[port];
}

ProcessorGraphicsItem::~ProcessorGraphicsItem() {}

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
    p->save();
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing, true);
    QColor topColor(140, 140, 140);
    QColor middleColor(59, 61, 61);
    QColor bottomColor(40, 40, 40);
    // paint processor
    QLinearGradient grad(rect().topLeft(), rect().bottomLeft());

    if (isSelected()) {
        grad.setColorAt(0.0f, topColor);
        grad.setColorAt(0.2f, middleColor);
        grad.setColorAt(0.5f, Qt::darkRed);
        grad.setColorAt(1.0f, bottomColor);
    } else {
        grad.setColorAt(0.0f, topColor);
        grad.setColorAt(0.2f, middleColor);
        grad.setColorAt(1.0f, bottomColor);
    }

    QRectF bRect = rect();
    QPainterPath roundRectPath = makeRoundedBox(rect(), roundedCorners);

    p->setBrush(grad);
    p->drawPath(roundRectPath);
    QLinearGradient highlightGrad(rect().topLeft(), rect().bottomLeft());

    if (isSelected()) {
        highlightGrad.setColorAt(0.0f, bottomColor);
        highlightGrad.setColorAt(0.1f, bottomColor);
        highlightGrad.setColorAt(0.5f, Qt::darkRed);
        highlightGrad.setColorAt(1.0f, bottomColor);
    } else {
        highlightGrad.setColorAt(0.0f, bottomColor);
        highlightGrad.setColorAt(1.0f, bottomColor);
    }

    QPainterPath highlightPath;
    float highlightLength = bRect.width() / 8.0;
    highlightPath.moveTo(bRect.left(), bRect.top() + roundedCorners);
    highlightPath.lineTo(bRect.left(), bRect.bottom() - roundedCorners);
    highlightPath.arcTo(bRect.left(), bRect.bottom() - (2 * roundedCorners), (2 * roundedCorners),
                        (2 * roundedCorners), 180.0, 90.0);
    highlightPath.lineTo(bRect.left() + (bRect.width() / 2.0) + highlightLength, bRect.bottom());
    highlightPath.lineTo(bRect.left() + (bRect.width() / 2.0) - highlightLength, bRect.top());
    highlightPath.lineTo(bRect.left() + roundedCorners, bRect.top());
    highlightPath.arcTo(bRect.left(), bRect.top(), (2 * roundedCorners), (2 * roundedCorners), 90.0,
                        90.0);

    p->setBrush(highlightGrad);
    p->drawPath(highlightPath);
    p->setPen(QPen(QColor(164, 164, 164), 1.0));
    p->setBrush(Qt::NoBrush);
    p->drawPath(roundRectPath);

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

    std::string updatedNewName = getProcessor()->setIdentifier(newName);

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
            if (isSelected()) {
                setZValue(SELECTED_PROCESSORGRAPHICSITEM_DEPTH);
                if (!highlight_) NetworkEditor::getPtr()->addPropertyWidgets(getProcessor());
            } else {
                setZValue(PROCESSORGRAPHICSITEM_DEPTH);
                NetworkEditor::getPtr()->removePropertyWidgets(getProcessor());
            }
            if (!highlight_ && processorMeta_) processorMeta_->setSelected(isSelected());
            break;
        case QGraphicsItem::ItemVisibleHasChanged:
            if (processorMeta_) processorMeta_->setVisibile(isVisible());
            break;
        default:
            break;
    }
    #include <warn/pop>
    return QGraphicsItem::itemChange(change, value);
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

#if IVW_PROFILING
void ProcessorGraphicsItem::onProcessorAboutToProcess(Processor*) {
    processCount_++;
    countLabel_->setHtml(QString::fromStdString("<p align=\"right\">"+toString(processCount_)+"</p>"));
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
    countLabel_->setHtml(QString("<p align=\"right\">0</p>"));
    maxEvalTime_ = 0.0;
    evalTime_ = 0.0;
    totEvalTime_ = 0.0;
}
#endif

ProcessorStatusGraphicsItem* ProcessorGraphicsItem::getStatusItem() const {
    return statusItem_;
}

void ProcessorGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    ToolTipHelper t(processor_->getDisplayName());
    t.row("Identifier", processor_->getIdentifier());
    t.row("Class", processor_->getClassIdentifier());
    t.row("Category", processor_->getCategory());
    t.row("Code", Processor::getCodeStateString(processor_->getCodeState()));
    t.row("Tags", processor_->getTags().getString());

#if IVW_PROFILING
    t.row("Ready", processor_->isReady()?"Yes":"No");
    t.row("Eval Count", processCount_);
    t.row("Eval Time", evalTime_);
    t.row("Mean Time", totEvalTime_/ std::max(static_cast<double>(processCount_), 1.0));
    t.row("Max Time", maxEvalTime_);
#endif

    showToolTipHelper(e, utilqt::toLocalQString(t));
}

void ProcessorGraphicsItem::setHighlight(bool val) {
    highlight_ = val;
    setSelected(val);
}

}  // namespace
