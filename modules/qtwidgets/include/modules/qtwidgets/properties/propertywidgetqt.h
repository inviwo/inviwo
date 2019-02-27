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

#ifndef IVW_PROPERTYWIDGETQT_H
#define IVW_PROPERTYWIDGETQT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/properties/propertyvisibility.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/propertyobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>

class QMenu;
class QMimeData;

namespace inviwo {

class Property;

class IVW_MODULE_QTWIDGETS_API PropertyWidgetQt : public QWidget,
                                                  public PropertyWidget,
                                                  public PropertyObserver {
public:
    using BaseCallBack = std::function<void()>;

    PropertyWidgetQt(Property* property = nullptr);
    virtual ~PropertyWidgetQt();

    // Should be called first thing after the property has been added to a layout.
    virtual void initState();

    static const double minimumWidthEm;
    static const double spacingEm;
    static const double marginEm;

    static const int minimumWidth;
    static const int spacing;
    static const int margin;

    void setSpacingAndMargins(QLayout* layout);
    static void setSpacingAndMargins(QWidget* w, QLayout* layout);

    virtual void onChildVisibilityChange(PropertyWidgetQt* child);

    virtual void setReadOnly(bool readonly);

    // QWidget overrides
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    void setNestedDepth(int depth);
    int getNestedDepth() const;

    PropertyWidgetQt* getParentPropertyWidget() const;
    void setParentPropertyWidget(PropertyWidgetQt* parent);

    virtual std::unique_ptr<QMenu> getContextMenu();

    /*
     * Serializes the property and sets the output xml as MIME data.
     * MIME type is set to "application/x.vnd.inviwo.property+xml"
     * and "text/plain".
     * Returns empty QMimeData if property is nullptr.
     */
    virtual std::unique_ptr<QMimeData> getPropertyMimeData() const;

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    // Create drag event if left button is pressed and moved
    // further than QApplication::startDragDistance()
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    // PropertyObservable overrides
    virtual void onSetReadOnly(Property* property, bool readonly) override;
    virtual void onSetVisible(Property* property, bool visible) override;
    virtual void onSetUsageMode(Property* property, UsageMode usageMode) override;

    virtual void setVisible(bool visible) override;
    UsageMode getApplicationUsageMode();

    virtual bool event(QEvent* event) override;  //< for custom tooltips.
    virtual void paintEvent(QPaintEvent* pe) override;

    QPoint mousePressedPosition_;  /// Assigned on mousePressEvent

    int getSpacing() const;

private:
    void addModuleMenuActions(QMenu* menu, InviwoApplication* app);
    void addPresetMenuActions(QMenu* menu, InviwoApplication* app);

    PropertyWidgetQt* parent_;

    const BaseCallBack* appModeCallback_;

    const int maxNumNestedShades_;  //< This number has do match the number of shades in the qss.
    int nestedDepth_;
};

}  // namespace inviwo

#endif  // IVW_PROPERTYWIDGETQT_H
