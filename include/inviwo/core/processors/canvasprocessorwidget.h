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

#ifndef IVW_CANVASPROCESSORWIDGET_H
#define IVW_CANVASPROCESSORWIDGET_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/network/processornetworkobserver.h>

namespace inviwo {

class Canvas;

/**
 * \brief A processor widget that has a canvas.
 * CanvasProcessorWidget is the base class for all processor widgets with canvases.
 *
 * The CanvasProcessorWidget is responsible for sending ResizeEvents up the network whenever there
 * are connections added or removed to the network to make sure that all the image ports in the
 * network above have an up-to-date view on which image sizes to use.
 * @see ResizeEvent
 */
class IVW_CORE_API CanvasProcessorWidget : public ProcessorWidget, public ProcessorNetworkObserver {
public:
    CanvasProcessorWidget(Processor* p);
    virtual Canvas* getCanvas() const = 0;

private:
    virtual void onProcessorNetworkDidAddConnection(const PortConnection&) override;
    virtual void onProcessorNetworkDidRemoveConnection(const PortConnection&) override;
};

}  // namespace inviwo

#endif  // IVW_CANVASPROCESSORWIDGET_H
