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

#ifndef IVW_RANGESLIDER_H
#define IVW_RANGESLIDER_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/userinterfacegl/glui/element.h>

#include <array>

namespace inviwo {

class Texture2DArray;

namespace glui {

class Renderer;

/**
 * \class RangeSlider
 * \brief glui::element representing a range slider, the label is positioned to the right. If the
 * flipped flag is set, the range slider uses the inverted range, i.e. the positions of the min/max
 * values are swapped.
 */
class IVW_MODULE_USERINTERFACEGL_API RangeSlider : public Element {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    RangeSlider(const std::string &label, const ivec2 &value, int minValue, int maxValue,
                int minSeparation, Processor &processor, Renderer &uiRenderer,
                const ivec2 &extent = ivec2(100, 24),
                UIOrientation orientation = UIOrientation::Horizontal);
    virtual ~RangeSlider() = default;

    void set(const ivec2 &value);
    void set(const ivec2 &value, int minValue, int maxValue, int minSeparation);
    const ivec2 &get() const;

    void setStart(int start);
    void setEnd(int stop);

    int getMinValue() const;
    int getMaxValue() const;

    void setMinSeparation(int sep);
    int getMinSeparation() const;

    double getHandleWidth() const;

    void setShowGroove(bool show);
    bool getShowGroove() const;

    void setFlipped(bool flipped);
    bool getFlipped() const;

protected:
    virtual void renderWidget(const ivec2 &origin, const size2_t &canvasDim) override;

    const ivec2 &getPreviousValue() const;

    /**
     * \brief transform mouse movements from pixel to normalized slider range while also
     * considering the slider orientation
     *
     * @param delta   (in screen coords, i.e. pixel)
     * @return delta movement normalized to slider range
     */
    double convertDeltaToSlider(dvec2 delta) const;

private:
    virtual ivec2 computeLabelPos(int descent) const override;
    virtual UIState uiState() const override;
    virtual vec2 marginScale() const override;
    virtual void pushStateChanged() override;

    vec2 getSliderPos() const;

    Texture2DArray *uiTextures_;
    Texture2DArray *centerTextures_;
    Texture2DArray *grooveTextures_;
    std::array<int, 9> uiTextureMap_;

    bool showGroove_ = true;
    bool flipped_ = false;

    ivec2 value_;
    int min_;
    int max_;
    int minSeparation_;

    ivec2 prevValue_;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_RANGESLIDER_H
