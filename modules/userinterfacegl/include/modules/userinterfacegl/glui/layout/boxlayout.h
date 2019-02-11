/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_GLUIBOXLAYOUT_H
#define IVW_GLUIBOXLAYOUT_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/userinterfacegl/glui/layout/layout.h>

#include <vector>
#include <functional>

namespace inviwo {

namespace glui {

/**
 * \class BoxLayout
 * \brief layout for aligning Elements either horizontally or vertically
 *
 * This class does not take over ownership of the Elements.
 */
class IVW_MODULE_USERINTERFACEGL_API BoxLayout : public Layout {
public:
    enum class LayoutDirection { Vertical, Horizontal };

    BoxLayout(LayoutDirection direction);
    virtual ~BoxLayout() = default;

    void setDirection(LayoutDirection direction);
    LayoutDirection getDirection() const;

    virtual ivec2 getExtent() const override;

    void setSpacing(int spacing);
    int getSpacing() const;

    /**
     * \brief apply the given scaling factor to all widgets
     *
     * @param factor   scaling factor for widget extents
     * @see Element::setScalingFactor
     */
    virtual void setScalingFactor(double factor) override;

    /**
     * \brief render the layout and all its glui::Elements at the given position
     *
     * @param topLeft         defines the top left corner where the UI is positioned
     * @param canvasDim      dimensions of the output canvas
     */
    virtual void render(const ivec2 &topLeft, const size2_t &canvasDim) override;

    /**
     * \brief add a glui::Element to the layout at the end of the layout
     *
     * @param element  glui::Element to be added
     */
    virtual void addElement(Element &element) override;

    /**
     * \brief insert a glui::Element to the layout at the given index. If the index is negative, the
     * UI element is added at the end.
     *
     * @param index    index position of where to insert the glui::Element
     * @param element  glui::Element to be added
     */
    void insertElement(int index, Element &element);

    /**
     * \brief remove the given glui::Element from the layout
     *
     * @param element  glui::Element to be removed
     */
    void removeElement(Element &element) override;

private:
    LayoutDirection direction_;
    int spacing_;

    std::vector<std::reference_wrapper<Element>> uiElements_;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_GLUIBOXLAYOUT_H
