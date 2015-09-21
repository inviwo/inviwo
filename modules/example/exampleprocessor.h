/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_EXAMPLEPROCESSOR_H
#define IVW_EXAMPLEPROCESSOR_H

#include <modules/example/examplemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>


namespace inviwo {

/** \docpage{org.inviwo.ExampleProcessor, Example Processor}
 * ![](org.inviwo.ExampleProcessor.png?classIdentifier=org.inviwo.ExampleProcessor)
 *
 * ...
 * 
 * ### Inports
 *   * __volume.inport__ ...
 * 
 * ### Outports
 *   * __geometry.outport__ ...
 * 
 *
 */
class IVW_MODULE_EXAMPLE_API ExampleProcessor : public Processor {
public:
    ExampleProcessor();
    ~ExampleProcessor();

    InviwoProcessorInfo();

    void initialize() override;
    void deinitialize() override;

protected:
    virtual void process() override;

private:
    VolumeInport inport_;
    MeshOutport outport_;
};

} // namespace

#endif // IVW_EXAMPLEPROCESSOR_H
