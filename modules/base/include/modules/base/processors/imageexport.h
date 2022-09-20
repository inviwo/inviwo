/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>                 // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/image/layer.h>        // for Layer
#include <inviwo/core/io/datawriter.h>                     // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>            // for DataWriterException
#include <inviwo/core/network/processornetworkobserver.h>  // for ProcessorNetworkObserver
#include <inviwo/core/ports/imageport.h>                   // for ImageInport
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>        // for IntSize2Property
#include <inviwo/core/util/glmvec.h>                       // for size2_t
#include <modules/base/processors/dataexport.h>            // for DataExport

#include <map>                                             // for operator!=, map

namespace inviwo {
class PortConnection;
class ProcessorNetwork;

/** \docpage{org.inviwo.ImageExport, Image Export}
 * ![](org.inviwo.ImageExport.png?classIdentifier=org.inviwo.ImageExport)
 *
 * A procesor to save images to disk
 *
 * ### Inports
 *   * __image__ The image to save.
 *
 *
 * ### Properties
 *   * __Export Image__ Save the image to disk.
 *   * __Image file name__ Filename to use.
 *   * __Overwrite__ Force overwrite.
 *
 */
class IVW_MODULE_BASE_API ImageExport : public DataExport<Layer, ImageInport>,
                                        public ProcessorNetworkObserver {
public:
    ImageExport();
    virtual ~ImageExport() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    BoolProperty outportDeterminesSize_;
    IntSize2Property imageSize_;

    virtual void setNetwork(ProcessorNetwork* network) override;

protected:
    void sendResizeEvent();

    virtual const Layer* getData() override;
    virtual void onProcessorNetworkDidAddConnection(const PortConnection&) override;
    virtual void onProcessorNetworkDidRemoveConnection(const PortConnection&) override;

    size2_t prevSize_;
};

}  // namespace inviwo
