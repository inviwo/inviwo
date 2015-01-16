/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/imageeditorwidgetqt.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo {

/////////////////////////////////////////////////
// Simple Graphics Scene
SimpleGraphicsScene::SimpleGraphicsScene(QWidget* parent) : QGraphicsScene(parent) { }


/////////////////////////////////////////////////
// Simple Graphics Rectangle Item with label
// used by Simple Graphics View

SimpleWithRectangleLabel::SimpleWithRectangleLabel(QPointF rectSize, QGraphicsScene* scene, int index)
    : QGraphicsRectItem(), label_(0), uniqueIndex_(index) {
    setRect(0, 0, rectSize.x(), rectSize.y());
    label_ = new LabelGraphicsItem(this);
    label_->setPos(0, 0);
    label_->setDefaultTextColor(Qt::black);
    label_->setFont(QFont("Segoe", 10, QFont::Black, false));
    label_->setCrop(9, 8);
}

SimpleWithRectangleLabel::~SimpleWithRectangleLabel() {}

void SimpleWithRectangleLabel::updateLabelPosition() {
    //set offset
}

void SimpleWithRectangleLabel::setLabel(std::string label) {
    label_->setText(QString::fromStdString(label));
}

std::string SimpleWithRectangleLabel::getLabel() { return label_->text().toLocal8Bit().constData(); }

void SimpleWithRectangleLabel::editLabel() {
    setFocus();
    label_->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    label_->setTextInteractionFlags(Qt::TextEditorInteraction);
    label_->setFocus();
}

int SimpleWithRectangleLabel::getIndex() {return uniqueIndex_;}
/////////////////////////////////////////////////
// Simple Graphics view

SimpleGraphicsView::SimpleGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , scene_(0)
    , rubberBandActive_(false)
    , readOnly_(false)
    , hideLabels_(false)
    , hideLabelDescriptions_(false)
    , fillRectangle_(false)
    , currentRectItem_(0) {
    setRenderHint(QPainter::Antialiasing, true);
    setMouseTracking(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    shadowEffect_ = new QGraphicsDropShadowEffect(this);
    shadowEffect_->setOffset(3.0);
    shadowEffect_->setBlurRadius(3.0);
}

void SimpleGraphicsView::setDialogScene(QGraphicsScene* scene) {
    //scene->setBackgroundBrush(QBrush(Qt::lightGray));
    setScene(scene);
    scene_ = scene;
}

void SimpleGraphicsView::addRectangle(QPointF mStartPoint, QPointF deltaPoint,ivec3 color, int index) {
    //QAbstractGraphicsShapeItem *i = scene_->addRect( mStartPoint.x(), mStartPoint.y(),  deltaPoint.x(), deltaPoint.y() );
    SimpleWithRectangleLabel* i = new SimpleWithRectangleLabel(deltaPoint, scene_, index);
    i->setPos(mStartPoint.x(), mStartPoint.y());
    scene_->addItem(i);

    if (!hideLabelDescriptions_)
        i->setLabel("Box");

    i->updateLabelPosition();
    i->setFlag(QGraphicsItem::ItemIsMovable);

    if (fillRectangle_)
        i->setBrush(QBrush(QColor(color.x, color.y, color.z), Qt::SolidPattern));
    else
        i->setPen(QPen(QColor(color.x, color.y, color.z), 2));

    i->setZValue(255);
    currentRectItem_ = i;

    if (hideLabels_)
        currentRectItem_->hide();

    setCurrentLabelPositionToTextField();
}

void SimpleGraphicsView::mouseDoubleClickEvent(QMouseEvent* e) {
    QList<QGraphicsItem*> graphicsItems =items(e->pos());

    //graphicsItems.size()==1 because of background pixmap item
    if (e->button()==Qt::LeftButton && graphicsItems.size()>1) {
        //Delete rectangle
        for (int i=0; i<graphicsItems.size(); i++) {
            SimpleWithRectangleLabel* rectItem = qgraphicsitem_cast<SimpleWithRectangleLabel*>(graphicsItems[i]);

            if (rectItem)
                rectItem->editLabel();
        }
    }

    QGraphicsView::mouseDoubleClickEvent(e);
}

SimpleGraphicsView::~SimpleGraphicsView() {
    delete shadowEffect_;
}

void SimpleGraphicsView::mousePressEvent(QMouseEvent* e) {
    QPoint currentPoint = e->pos();
    QList<QGraphicsItem*> graphicsItems =items(e->pos());

    //graphicsItems.size()==1 because of background pixmap item
    if (e->button()==Qt::LeftButton && graphicsItems.size()<2) {
        //Left click on canvas region, where there is no rectangle item
        startPoint_ = currentPoint;
        rubberBandActive_ = true;
    }
    else {
        rubberBandActive_ = false;

        if (e->modifiers() == Qt::ControlModifier) {
            //Delete rectangle
            for (int i=0; i<graphicsItems.size(); i++) {
                QGraphicsRectItem* rectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItems[i]);

                if (rectItem) {
                    if (!readOnly_) {
                        if (currentRectItem_ == rectItem)
                            currentRectItem_ = 0;

                        scene_->removeItem(rectItem);
                    }
                }
            }
        }
        else {
            //selection
            for (int i=0; i<graphicsItems.size(); i++) {
                QGraphicsRectItem* rectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItems[i]);

                if (rectItem) {
                    if (currentRectItem_) {
                        //switch off shadow
                        shadowEffect_->setOffset(0.0);
                        shadowEffect_->setBlurRadius(0.0);
                        currentRectItem_->setGraphicsEffect(shadowEffect_);
                        currentRectItem_ = 0;
                    }

                    currentRectItem_ = rectItem;
                    //switch on shadow
                    shadowEffect_->setOffset(3.0);
                    shadowEffect_->setBlurRadius(3.0);
                    currentRectItem_->setGraphicsEffect(shadowEffect_);
                    setCurrentLabelPositionToTextField();
                }
            }
        }
    }

    //e->accept();
    QGraphicsView::mousePressEvent(e);
}

std::vector<ImgRect> SimpleGraphicsView::getRectList() {
    std::vector<ImgRect> rectList;
    QList<QGraphicsItem*> graphicsItems = items();

    for (int i=0; i<graphicsItems.size(); i++) {
        SimpleWithRectangleLabel* rectItem = qgraphicsitem_cast<SimpleWithRectangleLabel*>(graphicsItems[i]);

        if (rectItem) {
            ImgRect r;
            vec2 pos(rectItem->scenePos().x(), rectItem->scenePos().y());
            vec2 pos1(rectItem->pos().x(), rectItem->pos().y());
            r.rect_ = QRectF(rectItem->mapRectToScene(rectItem->rect()));
            r.label_ = rectItem->getLabel();
            r.uniqueIndex_ = rectItem->getIndex();
            rectList.push_back(r) ;
        }
    }

    return rectList;
}

void SimpleGraphicsView::mouseReleaseEvent(QMouseEvent* e)
{
    QPointF mendPoint = mapToScene(e->pos());
    QPointF mStartPoint = mapToScene(startPoint_);
    QPointF deltaPoint = mendPoint - mStartPoint;

    if (rubberBandActive_ && (deltaPoint.x()>5 && deltaPoint.y()>5)) {
        if (e->modifiers() == Qt::ShiftModifier) {
            //add a square
            (deltaPoint.x()>deltaPoint.y())?deltaPoint.setY(deltaPoint.x()):deltaPoint.setX(deltaPoint.y());
        }

        if (!readOnly_)
            addRectangle(mStartPoint, deltaPoint);
    }

    rubberBandActive_ = false;
    //e->accept();
    QGraphicsView::mouseReleaseEvent(e);
}

void SimpleGraphicsView::mouseMoveEvent(QMouseEvent* e) {
    QList<QGraphicsItem*> graphicsItems =items(e->pos());

    if (e->button()==Qt::LeftButton) {
        //graphicsItems.size()==1 because of background pixmap item
        for (int i=0; i<graphicsItems.size(); i++) {
            QGraphicsRectItem* rectItem = qgraphicsitem_cast<QGraphicsRectItem*>(graphicsItems[i]);

            if (rectItem == currentRectItem_) {
                //LogWarn("Rect Item Move");
                setCurrentLabelPositionToTextField();
            }
        }

        //e->accept();
    }

    QGraphicsView::mouseMoveEvent(e);
}

void SimpleGraphicsView::setReadOnly(bool readOnly) {
    //does not allow creation of new labels
    readOnly_ = readOnly;
}

void SimpleGraphicsView::hideLabelDescription(bool hide) {
    hideLabelDescriptions_ = hide;
}

void SimpleGraphicsView::hideLabels(bool hide) {
    hideLabels_ = hide;
    QList<QGraphicsItem*> graphicsItems = items();

    if (hideLabels_) {
        for (int i=0; i<graphicsItems.size(); i++) {
            SimpleWithRectangleLabel* rectItem = qgraphicsitem_cast<SimpleWithRectangleLabel*>(graphicsItems[i]);

            if (rectItem) rectItem->hide();
        }
    }
    else {
        for (int i=0; i<graphicsItems.size(); i++) {
            SimpleWithRectangleLabel* rectItem = qgraphicsitem_cast<SimpleWithRectangleLabel*>(graphicsItems[i]);

            if (rectItem) rectItem->show();
        }
    }
}

void SimpleGraphicsView::filledRectangles(bool fill) {
    fillRectangle_ = fill;
}

void SimpleGraphicsView::setScaleFactor(float scaling) {
    QMatrix matrix;
    matrix.scale(scaling, scaling);
    setMatrix(matrix);
}

void SimpleGraphicsView::setCurrentLabelPositionFromTextField(ivec2 pos) {
    if (currentRectItem_) {
        SimpleWithRectangleLabel* rectItem = qgraphicsitem_cast<SimpleWithRectangleLabel*>(currentRectItem_);

        if (rectItem) {
            vec2 topLeft(pos.x, pos.y);
            QRectF rect = rectItem->mapRectToScene(rectItem->rect());
            vec2 rectSize(rect.size().width(), rect.size().height());
            topLeft = topLeft - (rectSize/2.0f);
            rectItem->setPos(QPointF(topLeft.x, topLeft.y));
        }
    }
}

void SimpleGraphicsView::setCurrentLabelPositionToTextField() {
    if (currentRectItem_) {
        SimpleWithRectangleLabel* rectItem = qgraphicsitem_cast<SimpleWithRectangleLabel*>(currentRectItem_);

        if (rectItem) {
            ImgRect r;
            vec2 pos(rectItem->scenePos().x(), rectItem->scenePos().y());
            vec2 pos1(rectItem->pos().x(), rectItem->pos().y());
            r.rect_ = QRectF(rectItem->mapRectToScene(rectItem->rect()));
            r.label_ = rectItem->getLabel();
            vec2 topLeft(r.rect_.topLeft().x(), r.rect_.topLeft().y());
            vec2 rectSize(r.rect_.size().width(), r.rect_.size().height());
            vec2 centerPos = topLeft+(rectSize/2.0f);
            emit currentRectItemPositionChanged(centerPos);
        }
    }
}

void SimpleGraphicsView::clearRectItems() {
    scene_->clear();
    currentRectItem_ = 0;
    shadowEffect_ = new QGraphicsDropShadowEffect();
    shadowEffect_->setOffset(3.0);
    shadowEffect_->setBlurRadius(3.0);
}
/////////////////////////////////////////////////
// Image Labeling widget
ImageLabelWidget::ImageLabelWidget() 
    : scene_(0)
    , view_(0)
    , backGroundImage_(0)
    , sceneScaleFactor_(1.2f) {
    
    generateWidget();
}

void ImageLabelWidget::closeEvent(QCloseEvent* event) {
    if (mainParentWidget_)
        mainParentWidget_->saveDialog();

    if (QWidget::isVisible())
        QWidget::hide();

    event->ignore();
}

void ImageLabelWidget::generateWidget() {
    scene_ = new SimpleGraphicsScene(this);
    scene_->setSceneRect(0, 0, 10, 10);
    view_ = new SimpleGraphicsView(this);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setDialogScene(scene_);
    scene_->setBackgroundBrush(QBrush(Qt::lightGray));
    QVBoxLayout* editorLayout = new QVBoxLayout();
    editorLayout->setSpacing(0);
    editorLayout->setMargin(0);
    toolBar_ = new QToolBar();
    saveButton_ = new QToolButton();
    saveButton_->setIcon(QIcon(":/icons/save.png")); // Temporary icon
    saveButton_->setToolTip("Save file");
    unDoButton_ = new QToolButton();
    unDoButton_->setIcon(QIcon(":/icons/arrow_left.png")); // Temporary icon
    unDoButton_->setToolTip("Undo");
    reDoButton_ = new QToolButton();
    reDoButton_->setIcon(QIcon(":/icons/arrow_right.png")); // Temporary icon
    reDoButton_->setToolTip("Redo");
    reLoadButton_ = new QToolButton();
    reLoadButton_->setIcon(QIcon(":/icons/inviwo_tmp.png")); // Temporary icon
    reLoadButton_->setToolTip("Reload");
    toolBar_->addWidget(saveButton_);
    toolBar_->addSeparator();
    toolBar_->addWidget(unDoButton_);
    toolBar_->addSeparator();
    toolBar_->addWidget(reDoButton_);
    toolBar_->addSeparator();
    toolBar_->addWidget(reLoadButton_);
    toolBar_->addSeparator();
    editorLayout->addWidget(toolBar_);
    editorLayout->addWidget(view_);
    editorLayout->addWidget(new QLabel());
    QHBoxLayout* hbox_xy = new QHBoxLayout();
    hbox_xy->setAlignment(Qt::AlignLeft);
    labelPositionX_ = new QLabel(" X : ");
    labelPositionX_->setMaximumWidth(32);
    hbox_xy->addWidget(labelPositionX_);
    positionX_ = new QSpinBox();
    positionX_->setMaximumWidth(64);
    hbox_xy->addWidget(positionX_);
    labelPositionY_ = new QLabel(" Y : ");
    labelPositionY_->setMaximumWidth(32);
    hbox_xy->addWidget(labelPositionY_);
    positionY_ = new QSpinBox();
    positionY_->setMaximumWidth(64);
    hbox_xy->addWidget(positionY_);
    editorLayout->addLayout(hbox_xy);
    positionX_->setRange(0, 1024);
    positionX_->setSingleStep(1);
    positionY_->setRange(0, 1024);
    positionY_->setSingleStep(1);
    connect(positionX_, SIGNAL(valueChanged(int)), this, SLOT(updatePositionX(int)));
    connect(positionY_, SIGNAL(valueChanged(int)), this, SLOT(updatePositionY(int)));
    connect(view_, SIGNAL(currentRectItemPositionChanged(vec2)),this, SLOT(onCurrentItemPositionChange(vec2)));
    setLayout(editorLayout);
    //test rectangle
    //addRectangleTest();
    //connect(unDoButton_,SIGNAL(pressed()),editor_,SLOT(undo()));
    //connect(reDoButton_,SIGNAL(pressed()),editor_,SLOT(redo()));
}

void ImageLabelWidget::setToolBarVisible(bool visible) {
    if (toolBar_) {
        if (visible)
            toolBar_->setVisible(true);
        else
            toolBar_->setVisible(false);
    }
}

void ImageLabelWidget::setReadOnly(bool readOnly) {
    //does not allow creation of new labels
    view_->setReadOnly(readOnly);
}

void ImageLabelWidget::setSceneScaling(float scaling) {
    sceneScaleFactor_ = scaling;
}

void ImageLabelWidget::hideLabelDescription(bool hide) {
    //does not show labels
    view_->hideLabelDescription(hide);
}

void ImageLabelWidget::hideLabels(bool hide) {
    //does not show label and descriptions
    view_->hideLabels(hide);

    if (hide) {
        positionX_->hide();
        positionY_->hide();
        labelPositionX_->hide();
        labelPositionY_->hide();
    }
    else {
        positionX_->show();
        positionY_->show();
        labelPositionX_->show();
        labelPositionY_->show();
    }
}

void ImageLabelWidget::addRectangleTest() {
    QAbstractGraphicsShapeItem* i = scene_->addRect(0, 0, 25, 25);
    i->setFlag(QGraphicsItem::ItemIsMovable);
    i->setBrush(QColor(0,0,128,0));
    i->setPen(QPen(QColor(255, 0, 0), 2));
    i->setZValue(255);
}

void ImageLabelWidget::addBackGroundImage(std::string imagePath) {
    view_->clearRectItems();
    backGroundImage_ = new QImage(imagePath.c_str());
    vec2 unscaledSize(backGroundImage_->width(), backGroundImage_->height());
    vec2 scaledSceneSize(backGroundImage_->width()*sceneScaleFactor_, backGroundImage_->height()*sceneScaleFactor_);
    backGroundImage_->scaled(QSize(scaledSceneSize.x, scaledSceneSize.y));
    QGraphicsPixmapItem* i = scene_->addPixmap(QPixmap(imagePath.c_str()));
    i->setZValue(1);
    scene_->setSceneRect(0, 0, unscaledSize.x, unscaledSize.y);
    view_->setScaleFactor(sceneScaleFactor_);
    resize(QSize(scaledSceneSize.x, scaledSceneSize.y));
}

void ImageLabelWidget::setParent(ImageEditorWidgetQt* tmp) {
    mainParentWidget_ = tmp;
}

void ImageLabelWidget::updatePositionX(int x) {
    ivec2 pos;
    pos[0] =x;
    pos[1] = positionY_->value();
    view_->setCurrentLabelPositionFromTextField(pos);
    //LogWarn("x,y " << pos[0] << " " << pos[1]);
}

void ImageLabelWidget::updatePositionY(int y) {
    ivec2 pos;
    pos[0] = positionX_->value();
    pos[1] = y;
    view_->setCurrentLabelPositionFromTextField(pos);
    //LogWarn("x,y " << pos[0] << " " << pos[1]);
}

void ImageLabelWidget::onCurrentItemPositionChange(vec2 centerPos) {
    ivec2 pos;
    pos[0] = centerPos[0];
    pos[1] = centerPos[1];
    positionX_->setValue(pos[0]);
    positionY_->setValue(pos[1]);
    emit rectItemPositionChanged();
}

/////////////////////////////////////////////////
// Image Editor widget
ImageEditorWidgetQt::ImageEditorWidgetQt(Property* property) : PropertyWidgetQt(property), imageLabelWidget_(0) {
    generateWidget();
    updateFromProperty();
}

ImageEditorWidgetQt::~ImageEditorWidgetQt() {
    if (imageLabelWidget_) {
        imageLabelWidget_->hide();
        delete imageLabelWidget_;
    }
}

void ImageEditorWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    btnEdit_ = new QToolButton();
    btnEdit_->setIcon(QIcon(":/icons/edit.png"));

    if (dynamic_cast<FileProperty*>(property_)) {
        fileWidget_ = new FilePropertyWidgetQt(static_cast<FileProperty*>(property_));
        connect(btnEdit_,SIGNAL(clicked()),this,SLOT(editImageLabel()));
        fileWidget_->layout()->addWidget(btnEdit_);
        hLayout->addWidget(fileWidget_);
    }
    setLayout(hLayout);
    
    imageLabelWidget_= new ImageLabelWidget();
    imageLabelWidget_->setParent(this);
    imageLabelWidget_->hide();
}

void ImageEditorWidgetQt::setPropertyValue() {}

void ImageEditorWidgetQt::editImageLabel() {
    ImageEditorProperty* imageEditorProperty = static_cast<ImageEditorProperty*>(property_);

    if (imageEditorProperty) {
        if (static_cast<FileProperty*>(property_)->get() == "")
            fileWidget_->setPropertyValue();

        connect(imageLabelWidget_->saveButton_, SIGNAL(pressed()), this, SLOT(writeImageLabel()));
        connect(imageLabelWidget_->reLoadButton_, SIGNAL(pressed()), this, SLOT(loadImageLabel()));
        loadImageLabel();
        imageLabelWidget_->show();
    }
}

void ImageEditorWidgetQt::loadImageLabel() {
    if (tmpPropertyValue_!=static_cast<FileProperty*>(property_)->get()) {
        tmpPropertyValue_ = static_cast<FileProperty*>(property_)->get();
        imageLabelWidget_->addBackGroundImage(tmpPropertyValue_);
        ImageEditorProperty* imageProperty  = static_cast<ImageEditorProperty*>(property_);
        const std::vector<ImageLabel> labels = imageProperty->getLabels();

        for (size_t i=0; i<labels.size(); i++) {
            QPointF topLeft(labels[i].getTopLeft()[0], labels[i].getTopLeft()[1]);
            QPointF rectSize(labels[i].getSize()[0], labels[i].getSize()[1]);
            imageLabelWidget_->view_->addRectangle(topLeft, rectSize);
        }
    }
}

//Function writes content of the textEditor_ to the file
bool ImageEditorWidgetQt::writeImageLabel() {
    //Close the file to open it with new flags
    ImageEditorProperty* imageEditorProperty = dynamic_cast<ImageEditorProperty*>(property_);

    if (imageEditorProperty) {
        //Save labels
        std::stringstream ss;
        std::vector<ImgRect> rectList = imageLabelWidget_->view_->getRectList();
        QSize dim = imageLabelWidget_->backGroundImage_->size();
        imageEditorProperty->setDimensions(ivec2(dim.width(), dim.height()));
        //if (rectList.size())
        imageEditorProperty->clearLabels();

        for (size_t i=0; i<rectList.size(); i++) {
            vec2 topLeft(rectList[i].rect_.topLeft().x(), rectList[i].rect_.topLeft().y());
            vec2 rectSize(rectList[i].rect_.size().width(), rectList[i].rect_.size().height());
            imageEditorProperty->addLabel(topLeft, rectSize, rectList[i].label_);
        }
    }

    return true;
}

bool ImageEditorWidgetQt::saveDialog() {
    if (imageLabelWidget_->getView()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"),
                                   tr("The image has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Save)
            writeImageLabel();
        else if (ret == QMessageBox::Cancel)
            return false;
    }

    return true;
}

void ImageEditorWidgetQt::updateFromProperty() {
    FileProperty* fileProp = dynamic_cast<FileProperty*>(property_);

    if (fileProp) {
        fileWidget_->updateFromProperty();
        loadImageLabel();
    }
}


} // namespace
