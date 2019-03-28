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

#ifndef IVW_GLUILAYOUT_H
#define IVW_GLUILAYOUT_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/interaction/pickingmapper.h>

#include <vector>

namespace inviwo {

namespace glui {

class Element;

/**
 * \class Layout
 * \brief base class for layouting glui::Elements
 *
 * \see glui::Element
 */
class IVW_MODULE_USERINTERFACEGL_API Layout {
public:
    Layout();
    virtual ~Layout() = default;

    virtual ivec2 getExtent() const = 0;

    void setMargins(int top, int left, int bottom, int right);
    /**
     * \brief set layout margins. \p margins correspond to margins in the order: top, left, bottom,
     * and right.
     *
     * @param margins   new margins (top, left, bottom, right)
     */
    void setMargins(const ivec4 &margins);
    const ivec4 &getMargins() const;

    /**
     * \brief apply the given scaling factor to all widgets
     *
     * @param factor   scaling factor for widget extents
     * @see Element::setScalingFactor
     */
    virtual void setScalingFactor(double factor) = 0;

    /**
     * \brief render the layout and all its UI elements at the given position
     *
     * @param topLeft         defines the top left corner where the UI is positioned
     * @param canvasDim      dimensions of the output canvas
     */
    virtual void render(const ivec2 &topLeft, const size2_t &canvasDim) = 0;

    /**
     * \brief add a UI element to the layout at the end of the layout
     *
     * @param element  UI element to be added
     */
    virtual void addElement(Element &element) = 0;

    /**
     * \brief remove the given UI element from the layout
     *
     * @param element  UI element to be removed
     */
    virtual void removeElement(Element &element) = 0;

protected:
    ivec4 margins_ = ivec4(10, 10, 10, 10);  //!< top, left, bottom, right
    double scaling_ = 1.0;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_GLUILAYOUT_H
