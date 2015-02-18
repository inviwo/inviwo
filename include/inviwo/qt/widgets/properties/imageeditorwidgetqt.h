/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_IMAGEEDITORWIDGETQT_H
#define IVW_IMAGEEDITORWIDGETQT_H

//QT includes
#include <QFile>
#include <QCheckBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>
#include <QMainWindow>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QLabel>
#include <QImage>
#include <QProgressDialog>
#include <QPixmap>
#include <QMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QSpinBox>
#include <QGraphicsDropShadowEffect>


//Property includes
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/imageeditorproperty.h>
#include <inviwo/core/properties/property.h>

//Widget includes
#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/properties/buttonpropertywidgetqt.h>
#include <inviwo/qt/widgets/properties/filepropertywidgetqt.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/qt/widgets/properties/stringpropertywidgetqt.h>
#include <inviwo/qt/widgets/labelgraphicsitem.h>
#include <inviwo/qt/widgets/editablelabelqt.h>


namespace inviwo {

/////////////////////////////////////////////////
// Simple Graphics scene
class IVW_QTWIDGETS_API SimpleGraphicsScene : public QGraphicsScene {
    Q_OBJECT
public:
    SimpleGraphicsScene(QWidget* parent=0);
signals:
    void status(const QString&);
};



/////////////////////////////////////////////////
// Simple Graphics Rectangle Item with label
// used by Simple Graphics View
class IVW_QTWIDGETS_API SimpleWithRectangleLabel : public QGraphicsRectItem {
public:
    SimpleWithRectangleLabel(QPointF mendPoint, QGraphicsScene* scene, int index=0);
    SimpleWithRectangleLabel();
    ~SimpleWithRectangleLabel();
    void updateLabelPosition();
    void setLabel(std::string label);
    std::string getLabel();
    void editLabel();
    int getIndex();
private:
    LabelGraphicsItem* label_;
    int uniqueIndex_; //to keep track of added rectangles
};

/////////////////////////////////////////////////
// Simple Graphics view

struct ImgRect {
    QRectF rect_;
    std::string label_;
    int uniqueIndex_;
};

class IVW_QTWIDGETS_API SimpleGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    SimpleGraphicsView(QWidget* parent=0);
    virtual ~SimpleGraphicsView();
    void setDialogScene(QGraphicsScene* scene);
    void addRectangle(QPointF mstartPoint, QPointF mendPoint,ivec3 color = ivec3(0,0,255), int uniqueIndex=0);
    std::vector<ImgRect> getRectList();
    void setReadOnly(bool readOnly);
    void hideLabelDescription(bool hide);
    void hideLabels(bool hide);
    void filledRectangles(bool fill);
    void setCurrentLabelPositionFromTextField(ivec2 pos);
    void setCurrentLabelPositionToTextField();
    void setScaleFactor(float scaling);
    void clearRectItems();
protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
signals:
    void currentRectItemPositionChanged(vec2 pos);
private:
    QGraphicsScene* scene_;
    QPoint startPoint_;
    bool rubberBandActive_;
    bool readOnly_;
    bool hideLabels_;
    bool hideLabelDescriptions_;
    bool fillRectangle_;
    QGraphicsRectItem* currentRectItem_;
    QGraphicsDropShadowEffect* shadowEffect_;
};


/////////////////////////////////////////////////
// Image Labeling widget
class ImageEditorWidgetQt;
class IVW_QTWIDGETS_API ImageLabelWidget : public QWidget {
    Q_OBJECT
public:
    ImageLabelWidget();
    virtual ~ImageLabelWidget(){
        delete backGroundImage_;
        delete view_;
    }
    bool saveDialog();
    void setParent(ImageEditorWidgetQt*);
    QGraphicsScene* getScene() {return scene_;}
    QGraphicsView* getView() {return view_;}
    void addRectangleTest();
    void addBackGroundImage(std::string imagePath);
    void generateWidget();
    void setReadOnly(bool readOnly);
    void hideLabelDescription(bool hide);
    void hideLabels(bool hide);
    ImageEditorWidgetQt* mainParentWidget_;
    QToolBar* toolBar_;
    QToolButton* reDoButton_;
    QToolButton* reLoadButton_;
    QToolButton* saveButton_;
    QToolButton* unDoButton_;
    std::string tmpPropertyValue_;
    SimpleGraphicsScene* scene_;
    SimpleGraphicsView* view_;
    QImage* backGroundImage_;
    void setToolBarVisible(bool visible);
    void setSceneScaling(float scaling);
public slots:
    void updatePositionX(int);
    void updatePositionY(int);
    void onCurrentItemPositionChange(vec2 pos);
signals:
    void rectItemPositionChanged();
protected:
    void closeEvent(QCloseEvent*);
    QSpinBox* positionX_;
    QSpinBox* positionY_;
    QLabel* labelPositionX_;
    QLabel* labelPositionY_;
    float sceneScaleFactor_;
};

/////////////////////////////////////////////////
// Image Editor widget
class IVW_QTWIDGETS_API ImageEditorWidgetQt : public PropertyWidgetQt {
    Q_OBJECT
public:
    ImageEditorWidgetQt(Property* property);
    virtual ~ImageEditorWidgetQt();
    void updateFromProperty();
    bool saveDialog();
private:
    QToolButton* btnEdit_;
    FilePropertyWidgetQt* fileWidget_;
    ImageLabelWidget* imageLabelWidget_;
    std::string tmpPropertyValue_;
    QCheckBox* checkBox_;
    EditableLabelQt* label_;

    void generateWidget();
public slots:
    void loadImageLabel();
    void editImageLabel();
    void setPropertyValue();
    bool writeImageLabel();
};


}//namespace

#endif //IVW_IMAGEEDITORWIDGETQT_H