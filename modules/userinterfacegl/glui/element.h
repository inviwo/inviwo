/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_GLUIELEMENT_H
#define IVW_GLUIELEMENT_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/userinterfacegl/glui/renderer.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/pickingmapper.h>

#include <functional>
#include <string>
#include <vector>

namespace inviwo {

class Texture2D;

namespace glui {

/**
 * \class glui::Element
 * \brief graphical UI element for use in combination with glui::Layout
 *
 * \see glui::Layout, glui::Renderer
 */
class IVW_MODULE_USERINTERFACEGL_API Element {
public:
    enum class UIState { Normal, Pressed, Checked };

    Element(const std::string &label, Processor &processor, Renderer &uiRenderer);
    virtual ~Element();

    void setVisible(bool visible = true);
    bool isVisible() const;

    void setEnabled(bool enable = true);
    bool isEnabled() const;

    void setLabel(const std::string &str);
    const std::string &getLabel() const;

    void setLabelBold(bool bold);
    bool isLabelBold() const;

    void setLabelVisible(bool visible = true);
    bool isLabelVisible() const;

    bool isDirty() const;

    const ivec2 &getExtent();

    /**
     * \brief render the widget and its label at the given position
     *
     * @param origin         defines the lower left corner where the widget is positioned
     * @param canvasDim      dimensions of the output canvas
     */
    void render(const ivec2 &origin, const size2_t &canvasDim);

    void setHoverState(bool enable);
    bool isHovered() const { return hovered_; }

    void setPushedState(bool pushed);
    bool isPushed() const;

    void setChecked(bool checked);
    bool isChecked() const;

    /**
     * \brief sets the callback action when the user releases the mouse button
     */
    void setAction(const std::function<void()> &action);

    /**
     * \brief updates the UI state and triggers the callback action set by setAction().
     * This function is called when the user releases the mouse button.
     *
     * \see setAction
     */
    void triggerAction();

    /**
     * \brief set callback function for handling mouse movements based on a delta position.
     * This callback gets called on mouse move events.
     *
     * @param action   function taking one argument (2D delta position in screen coords) returning
     * true if the movement triggers an update of the element
     */
    void setMouseMoveAction(const std::function<bool(const dvec2 &)> &action);

    /**
     * \brief gets called on mouse move events
     *
     * @param delta    delta mouse position in screen coord, i.e. pixels, relative to pressed
     * position
     * @return true if the movement triggers an update of the element
     */
    bool moveAction(const dvec2 &delta);

protected:
    void updateExtent();
    void updateLabelPos();
    void updateLabel();
    virtual ivec2 computeLabelPos(int descent) const = 0;
    virtual UIState uiState() const;
    virtual vec2 marginScale() const;
    /**
     * \brief is called before the action is triggered to update the internal UI state
     *
     * \see triggerAction, setAction
     */
    virtual void updateState(){};

    /**
     * \brief  It is called by setPushState after the internal push state has been updated
     */
    virtual void pushStateChanged(){};

    virtual void renderWidget(const ivec2 &origin, const size2_t &canvasDim) = 0;

    void renderLabel(const ivec2 &origin, const size2_t &canvasDim);

    void handlePickingEvent(PickingEvent *e);

    // reduce saturation and darken color
    static vec4 adjustColor(const vec4 &color);

    std::function<void()>
        action_;  //<! is called by triggerAction() after the internal state has been updated
    std::function<bool(const dvec2 &)> moveAction_;  //!< is called by mouseMoved()

    // UI interaction states
    bool hovered_;  // true as long as the element is under the mouse and element is enabled
    bool pushed_;  // true as long as the mouse button is not released, mouse might not be on top of
                   // UI element any more
    bool checked_;

    bool visible_;
    bool enabled_;  // UI elements will respond to mouse interactions only if enabled

    bool boldLabel_;
    bool labelVisible_;

    // Layout of a UI element:
    //                                                         extent
    //   +----------------------------------------------------+
    //   |                                                    |
    //   |                 widgetExtent                       |
    //   |     +----------+                      labelExtent  |
    //   |     | rendered |        +-----------------+        |
    //   |     | textures |        |  label          |        |
    //   |     |    +     |        +-----------------+        |
    //   |     | picking  |    labelPos                       |
    //   |     +----------+                                   |
    //   |  widgetPos                                         |
    //   |                                                    |
    //   +----------------------------------------------------+
    // (0,0)
    //

    ivec2 extent_;
    ivec2 widgetPos_;
    ivec2 widgetExtent_;
    ivec2 labelPos_;
    ivec2 labelExtent_;

    std::string labelStr_;
    bool labelDirty_;

    std::shared_ptr<Texture2D> labelTexture_;

    Processor *processor_;
    Renderer *uiRenderer_;

    PickingMapper pickingMapper_;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_GLUIELEMENT_H
