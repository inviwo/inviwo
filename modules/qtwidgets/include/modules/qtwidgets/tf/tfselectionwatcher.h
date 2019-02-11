/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_TFSELECTIONWATCHER_H
#define IVW_TFSELECTIONWATCHER_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/tfprimitive.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QObject>
#include <QColor>
#include <warn/pop>

namespace inviwo {

class TFEditor;
class TFPrimitiveSet;
class Property;

/**
 * \class TFSelectionWatcher
 * \brief observes a selection of primitives in a particular TF and sends signals to inform on
 * position, alpha, and color changes
 */
class IVW_MODULE_QTWIDGETS_API TFSelectionWatcher : public QObject, public TFPrimitiveObserver {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    TFSelectionWatcher(TFEditor *editor, Property *property,
                       const std::vector<TFPrimitiveSet *> &primitiveSets);
    virtual ~TFSelectionWatcher() = default;

signals:
    void updateWidgetPosition(double pos, bool ambiguous = false);
    void updateWidgetAlpha(double alpha, bool ambiguous = false);
    void updateWidgetColor(const QColor &c, bool ambiguous = false);

public slots:
    /**
     * set the position of all currently selected TF primitives
     *
     * @param pos  new position
     */
    void setPosition(double pos);
    /**
     * set the alpha value of all currently selected TF primitives
     *
     * @param alpha  new alpha value
     */
    void setAlpha(double alpha);
    /**
     * set the color of all currently selected TF primitives
     *
     * @param c  the new color
     */
    void setColor(const QColor &c);

    /**
     * updates the selection state and sends out signals for position, alpha, and color.
     * In case, multiple primitives are selected, the values are considered ambiguous unless
     * they all have the same value. The position, alpha, and color are considered separately.
     */
    void updateSelection(const std::vector<TFPrimitive *> selection);

private:
    virtual void onTFPrimitiveChange(const TFPrimitive &p) override;

    void informWidgets();

    std::vector<TFPrimitive *> selectedPrimitives_;

    TFEditor *tfEditor_;
    Property *property_;
    std::vector<TFPrimitiveSet *> tfSets_;

    bool updateInProgress_ = false;

    // cache TF primitive values of last single element selection
    double cachedPos_ = 0.0;
    double cachedAlpha_ = 0.0;
    QColor cachedColor_ = QColor();
};

}  // namespace inviwo

#endif  // IVW_TFSELECTIONWATCHER_H
