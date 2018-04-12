/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#ifndef IVW_PYTHONMESHSCRIPTSOURCE_H
#define IVW_PYTHONMESHSCRIPTSOURCE_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/fileproperty.h>
#include <modules/python3/pythonscript.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

/** \docpage{org.inviwo.PythonScriptProcessor, Python Mesh Script Source}
 * ![](org.inviwo.PythonScriptProcessor.png?classIdentifier=org.inviwo.PythonScriptProcessor)
 * Source processor for loading mesh and volume data using a python script.
 *
 * ### Outports
 *   * __mesh__    mesh data
 *   * __volume__  volume data
 *
 * ### Properties
 *   * __File Name__  file name of the python script
 */

/**
 * \class PythonScriptProcessor
 * \brief Loads a mesh and volume via a python script. The processor is invalidated
 * as soon as the script changes on disk.
 */
class IVW_MODULE_PYTHON3_API PythonScriptProcessor : public Processor {
public:
    PythonScriptProcessor();
    virtual ~PythonScriptProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    PythonScriptDisk script_;
    FileProperty scriptFileName_;

    MeshOutport mesh_;
    VolumeOutport volume_;
};

}  // namespace inviwo

#endif  // IVW_PYTHONMESHSCRIPTSOURCE_H
