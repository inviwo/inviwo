/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <modules/openglqt/openglqtmoduledefine.h>  // for IVW_MODULE_OPENGLQT_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for LayerType, operator<<
#include <inviwo/core/io/datawriter.h>                    // for Overwrite
#include <inviwo/core/ports/imageport.h>                  // for ImageInport
#include <inviwo/core/processors/exporter.h>              // for Exporter
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>          // for BoolProperty
#include <inviwo/core/properties/optionproperty.h>        // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>       // for IntSize2Property, IntSizeTProperty
#include <inviwo/core/properties/stringproperty.h>        // for StringProperty
#include <inviwo/core/util/staticstring.h>                // for operator+

#include <functional>   // for __base
#include <memory>       // for unique_ptr
#include <optional>     // for optional
#include <string>       // for operator==, string
#include <string_view>  // for operator==, string_view
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {

class Event;
class FileExtension;
class Outport;
class ProcessorWidget;
struct PWObserver;

/** \docpage{org.inviwo.CanvasWithPropertiesProcessor, Canvas With Properties}
 * ![](org.inviwo.CanvasWithPropertiesProcessor.png?classIdentifier=org.inviwo.CanvasWithPropertiesProcessor)
 *
 * Show a processor widget with a split pane with a image on the left, and a list of propertywidgets
 * on the right
 *
 *
 * ### Inports
 *   * __inport__ image to render
 *
 * ### Properties
 *   * __Widget Size__ size of the whole widget, not just the image.
 *   * __Widget Position__ position of the widget
 *   * __Visible__ Toggle widget visibility
 *   * __Full Screen__ Toggle if thw widget should be fullscreen
 *   * __On Top__ Keep the widget on top of the inviwo main window.
 *   * __Visible Layer__ The layer type to render, color, depth of picking
 *   * __Color Layer ID__ The color layer index to render
 *   * __Paths__ A list of processor ids and/or property paths, seperated by new lines to show in
 * the property list in the widget.
 *
 */
class IVW_MODULE_OPENGLQT_API CanvasWithPropertiesProcessor : public Processor, public Exporter {
public:
    CanvasWithPropertiesProcessor();
    virtual ~CanvasWithPropertiesProcessor();

    virtual void process() override;
    virtual void doIfNotReady() override;
    virtual void setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void propagateEvent(Event* event, Outport* source) override;

    /**
     * @see Exporter::exportFile
     */
    virtual std::optional<std::string> exportFile(
        std::string_view path, std::string_view name,
        const std::vector<FileExtension>& candidateExtensions, Overwrite overwrite) const override;

private:
    std::unique_ptr<PWObserver> pwObserver_;
    ImageInport inport_;

    IntSize2Property dimensions_;
    IntVec2Property position_;
    BoolProperty visible_;
    BoolProperty fullScreen_;
    BoolProperty onTop_;

    OptionProperty<LayerType> layerType_;
    IntSizeTProperty layerIndex_;

    StringProperty paths_;
};

}  // namespace inviwo
