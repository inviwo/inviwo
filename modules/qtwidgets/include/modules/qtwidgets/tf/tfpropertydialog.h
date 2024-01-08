/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/datastructures/tfprimitive.h>               // for TFPrimitive
#include <inviwo/core/datastructures/tfprimitiveset.h>            // for TFPrimitiveSetObserver
#include <inviwo/core/processors/processor.h>                     // for Processor, Processor::N...
#include <inviwo/core/properties/transferfunctionproperty.h>      // for TFPropertyObserver, Tra...
#include <inviwo/core/util/glmvec.h>                              // for dvec2
#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>  // for PropertyEditorWidgetQt

#include <functional>  // for function
#include <memory>      // for unique_ptr, shared_ptr
#include <string>      // for string
#include <vector>      // for vector

#include <QSize>  // for QSize

class QColorDialog;
class QComboBox;
class QLabel;
class QResizeEvent;
class QShowEvent;

namespace inviwo {

class ColorWheel;
class IsoTFProperty;
class IsoValueProperty;
class Property;
class RangeSliderQt;
class TFColorEdit;
class TFEditor;
class TFEditorView;
class TFLineEdit;
class TFSelectionWatcher;
namespace util {
struct TFPropertyConcept;
}  // namespace util

class IVW_MODULE_QTWIDGETS_API TFPropertyDialog : public PropertyEditorWidgetQt,
                                                  public TFPrimitiveSetObserver,
                                                  public TFPropertyObserver {
public:
    TFPropertyDialog(TransferFunctionProperty* tfProperty);
    TFPropertyDialog(IsoValueProperty* isoProperty);
    TFPropertyDialog(IsoTFProperty* isotfProperty);
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

    void updateTitleFromProperty();
    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;

private:
    TFPropertyDialog(std::unique_ptr<util::TFPropertyConcept> model,
                     std::vector<TFPrimitiveSet*> tfSets);

    void updateTFPreview();
    /**
     * calculate the horizontal and vertical offset in scene coordinates based on the current
     * viewport size and zoom. The offset then corresponds to defaultOffset pixels on screen.
     */
    dvec2 getRelativeSceneOffset() const;

    const int sliderRange_;
    static constexpr int verticalSliderRange_ = 1000;
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

    QLabel* domainMin_;
    QLabel* domainMax_;

    // widgets for directly editing the currently selected TF primitives
    TFLineEdit* primitivePos_;
    TFLineEdit* primitiveAlpha_;
    TFColorEdit* primitiveColor_;

    QLabel* tfPreview_;  ///< View that contains the scene for the painted transfer function

    RangeSliderQt* zoomVSlider_;
    RangeSliderQt* zoomHSlider_;

    bool ongoingUpdate_ = false;
    Processor::NameDispatcherHandle onNameChange_;
    std::vector<std::shared_ptr<std::function<void()>>> portCallbacks_;
};

}  // namespace inviwo
