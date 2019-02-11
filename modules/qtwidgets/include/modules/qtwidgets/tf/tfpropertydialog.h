/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_TRANSFERFUNCTIONPROPERTYDIALOG_H
#define IVW_TRANSFERFUNCTIONPROPERTYDIALOG_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <modules/qtwidgets/tf/tfeditor.h>
#include <modules/qtwidgets/tf/tfeditorview.h>
#include <modules/qtwidgets/properties/ordinalminmaxpropertywidgetqt.h>
#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>
#include <modules/qtwidgets/properties/optionpropertywidgetqt.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/tfpropertyconcept.h>
#include <inviwo/core/util/observer.h>

class QPushButton;
class QComboBox;
class QLabel;
class QResizeEvent;
class QShowEvent;
class QColorDialog;

namespace inviwo {

class ColorWheel;
class RangeSliderQt;
class TFPropertyWidgetQt;
class TFSelectionWatcher;
class TFLineEdit;
class TFColorEdit;
class TransferFunctionProperty;
class IsoValueProperty;
class IsoTFProperty;

class IVW_MODULE_QTWIDGETS_API TFPropertyDialog : public PropertyEditorWidgetQt,
                                                  public TFPrimitiveSetObserver,
                                                  public TFPropertyObserver {
public:
    TFPropertyDialog(TransferFunctionProperty* tfProperty,
                     const std::vector<TFPrimitiveSet*>& primitiveSets = {});
    TFPropertyDialog(IsoValueProperty* isoProperty,
                     const std::vector<TFPrimitiveSet*>& primitiveSets = {});
    TFPropertyDialog(IsoTFProperty* isotfProperty,
                     const std::vector<TFPrimitiveSet*>& primitiveSets = {});
    ~TFPropertyDialog();

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    void updateFromProperty();
    TFEditorView* getEditorView() const;

protected:
    virtual void onTFPrimitiveAdded(TFPrimitive& p) override;
    virtual void onTFPrimitiveRemoved(TFPrimitive& p) override;
    virtual void onTFPrimitiveChanged(const TFPrimitive& p) override;
    virtual void onTFTypeChanged(const TFPrimitiveSet& primitiveSet) override;
    void onTFTypeChangedInternal();

    virtual void onMaskChange(const dvec2& mask) override;
    virtual void onZoomHChange(const dvec2& zoomH) override;
    virtual void onZoomVChange(const dvec2& zoomV) override;

    virtual void setReadOnly(bool readonly) override;

    void changeVerticalZoom(int zoomMin, int zoomMax);
    void changeHorizontalZoom(int zoomMin, int zoomMax);
    void showHistogram(int type);
    void changeMoveMode(int i);

    virtual void resizeEvent(QResizeEvent*) override;
    virtual void showEvent(QShowEvent*) override;

private:
    void initializeDialog();
    void updateTFPreview();
    /**
     * calculate the horizontal and vertical offset in scene coordinates based on the current
     * viewport size and zoom. The offset then corresponds to defaultOffset pixels on screen.
     */
    dvec2 getRelativeSceneOffset() const;

    const int sliderRange_;
    const int defaultOffset_ = 5;  //!< offset in pixel

    std::unique_ptr<util::TFPropertyConcept> propertyPtr_;
    std::vector<TFPrimitiveSet*> tfSets_;

    std::unique_ptr<ColorWheel> colorWheel_;
    std::unique_ptr<QColorDialog> colorDialog_;

    std::unique_ptr<TFEditor> tfEditor_;  //!< inherited from QGraphicsScene

    std::unique_ptr<TFSelectionWatcher> tfSelectionWatcher_;

    TFEditorView* tfEditorView_;  //!< View that contains the editor
    QComboBox* chkShowHistogram_;

    QComboBox* pointMoveMode_;

    // widgets for directly editing the currently selected TF primitives
    TFLineEdit* primitivePos_;
    TFLineEdit* primitiveAlpha_;
    TFColorEdit* primitiveColor_;

    QLabel* tfPreview_;  ///< View that contains the scene for the painted transfer function

    RangeSliderQt* zoomVSlider_;
    RangeSliderQt* zoomHSlider_;
};

}  // namespace inviwo

#endif  // IVW_TRANSFERFUNCTIONPROPERTYDIALOG_H
