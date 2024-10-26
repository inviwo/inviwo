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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/volume/volume.h>  // for DataInport, Volume
#include <inviwo/core/io/datawriter.h>                 // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>        // for DataWriterException
#include <inviwo/core/ports/volumeport.h>              // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <modules/base/processors/dataexport.h>        // for DataExport

#include <functional>  // for __base
#include <map>         // for map, operator!=
#include <vector>      // for vector

namespace inviwo {

/** \docpage{org.inviwo.VolumeExport, Volume Export}
 * ![](org.inviwo.VolumeExport.png?classIdentifier=org.inviwo.VolumeExport)
 *
 * Export volumes
 *
 * ### Inports
 *   * __Volume__ Volume to export
 *
 * ### Properties
 *   * __Volume file name__ File to export to
 *   * __Export Volume__ Button to execute export
 *   * __Overwrite__ Should existing files be overwritten
 *
 */
class IVW_MODULE_BASE_API VolumeExport : public DataExport<Volume, VolumeInport> {
public:
    VolumeExport(InviwoApplication* app);
    virtual ~VolumeExport() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual const Volume* getData() override;
};

}  // namespace inviwo
