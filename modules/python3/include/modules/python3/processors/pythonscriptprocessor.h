/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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
#include <pybind11/pybind11.h>

namespace inviwo {

/** \docpage{org.inviwo.PythonScriptProcessor, Python Mesh Script Source}
 * ![](org.inviwo.PythonScriptProcessor.png?classIdentifier=org.inviwo.PythonScriptProcessor)
 * Processor defined by a python script.
 *
 * ### Properties
 *   * __File Name__  file name of the python script
 *
 * ### Example script
 * \code{.py}
 * import inviwopy
 * from inviwopy.glm import ivec3, dvec2
 * from inviwopy.properties import IntVec3Property
 * from inviwopy.data import VolumeOutport
 * from inviwopy.data import Volume
 * import numpy
 *
 * """
 * The PythonScriptProcessor will run this script on construction and whenever this
 * it changes. Hence one needs to take care not to add ports and properties multiple times.
 * The PythonScriptProcessor is exposed as the local variable 'self'.
 * """
 *
 * if not "dim" in self.properties:
 *     self.addProperty(IntVec3Property("dim", "dim", ivec3(5), ivec3(0), ivec3(20)))
 *
 * if not "outport" in self.outports:
 *     self.addOutport(VolumeOutport("outport"))
 *
 * def process(self):
 *     """
 *     The PythonScriptProcessor will call this process function whenever the processor process
 *     function is called. The argument 'self' represents the PythonScriptProcessor.
 *     """
 *     # create a small float volume filled with random noise
 *     numpy.random.seed(546465)
 *     dim = self.properties.dim.value;
 *     volume = Volume(numpy.random.rand(dim[0], dim[1], dim[2]).astype(numpy.float32))
 *     volume.dataMap.dataRange = dvec2(0.0, 1.0)
 *     volume.dataMap.valueRange = dvec2(0.0, 1.0)
 *     self.outports.outport.setData(volume)
 *
 * def initializeResources(self):
 *     pass
 *
 * # Tell the PythonScriptProcessor about the 'initializeResources' function we want to use
 * self.setInitializeResources(initializeResources)
 *
 * # Tell the PythonScriptProcessor about the 'process' function we want to use
 * self.setProcess(process)
 * \endcode
 */

/**
 * \class PythonScriptProcessor
 * \brief Loads a mesh and volume via a python script. The processor is invalidated
 * as soon as the script changes on disk.
 */
class IVW_MODULE_PYTHON3_API PythonScriptProcessor : public Processor {
public:
    PythonScriptProcessor(InviwoApplication* app);
    virtual ~PythonScriptProcessor() = default;

    virtual void initializeResources() override;
    virtual void process() override;

    void setInitializeResources(pybind11::function func);
    void setProcess(pybind11::function func);

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    FileProperty scriptFileName_;
    PythonScriptDisk script_;

    pybind11::function initializeResources_;
    pybind11::function process_;
};

}  // namespace inviwo

#endif  // IVW_PYTHONMESHSCRIPTSOURCE_H
