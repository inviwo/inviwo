/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/datastructures/geometry/mesh.h>        // for Mesh
#include <inviwo/core/interaction/pickingmapper.h>           // for PickingMapper
#include <inviwo/core/util/glmvec.h>                         // for size2_t
#include <modules/basegl/datastructures/splittersettings.h>  // for Direction, Direction::Vertical
#include <modules/opengl/shader/shader.h>                    // for Shader

#include <cstddef>     // for size_t
#include <functional>  // for function
#include <vector>      // for vector

namespace inviwo {

class PickingEvent;
class Processor;

/**
 * \brief utility class for rendering vertical or horizontal splitters and providing drag
 * interactions. This class will invalidate the processor for hover and drag events in order to
 * trigger a redraw.
 */
class IVW_MODULE_BASEGL_API SplitterRenderer {
public:
    using InvalidateCallback = std::function<void()>;
    using DragCallback = std::function<void(float, int)>;

    SplitterRenderer(Processor* processor);
    SplitterRenderer(const SplitterRenderer& rhs) = delete;
    SplitterRenderer(SplitterRenderer&& rhs) = default;
    SplitterRenderer& operator=(const SplitterRenderer& rhs) = delete;
    SplitterRenderer& operator=(SplitterRenderer&& rhs) = default;
    virtual ~SplitterRenderer() = default;

    /**
     * \brief set the invalidation callback. \p callback is called whenever the splitter requires a
     * redraw, for example during dragging or for hover events
     */
    void setInvalidateAction(InvalidateCallback callback);

    /**
     * \brief \p callback will be called when the splitter is moved by dragging via mouse or touch.
     * The arguments of the callback correspond to the new position and the index of the dragged
     * splitter. When only one splitter is rendered the index will be 0.
     */
    void setDragAction(DragCallback callback);

    /*
     * \brief render a splitter at the given positions \pos and \p direction using the \p
     * settings
     *
     * @param settings    used to determine the style of the splitter (color, width, ...)
     * @param direction   splitter orientation
     * @param pos         position of the splitters in normalized screen coordinates [0,1]
     * @param canvasDims  dimensions of the output canvas
     */
    void render(const SplitterSettings& settings, splitter::Direction direction,
                const std::vector<float>& pos, size2_t canvasDims);

private:
    void handlePickingEvent(PickingEvent* e);

    Processor* processor_;
    InvalidateCallback invalidate_;
    DragCallback dragAction_;

    Shader shader_;
    Shader triShader_;
    Mesh mesh_;

    PickingMapper pickingMapper_;
    int hoveredSplitter_ = -1;
    size_t maxSplittersInShader_ = 1;
    splitter::Direction currentDirection_ = splitter::Direction::Vertical;
};

}  // namespace inviwo
