/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMECOMBINER_H
#define IVW_VOLUMECOMBINER_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/buffer/framebufferobject.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeCombiner, Volume Combiner}
* Combines two volumes into a single volume. Resolution and data type of the 
* result match the one of input volume A. The input volumes can be scaled individually.
* ![](org.inviwo.VolumeCombiner.png?classIdentifier=org.inviwo.VolumeCombiner)
*
* ### Inports
*   * __VolumeInport__ Input volume A.
*   * __VolumeInport__ Input volume B.
*
* ### Outports
*   * __VolumeOutport__ The output volume. Dimension and data type match input volume A. 
*                       <tt>combine(a,b) = a * volScale1 + b * volScale2</tt>
* 
*
* ### Properties
*   * __Volume 1 Scaling__ Scaling factor for volume 1.
*   * __Volume 2 Scaling__ Scaling factor for volume 2.
*/

/*! \class VolumeCombiner
*
* \brief Combines two volumes.
*/
class IVW_MODULE_BASEGL_API VolumeCombiner : public Processor { 
public:
    InviwoProcessorInfo();
    
    VolumeCombiner();
    virtual ~VolumeCombiner() = default;


    virtual void process() override;
    virtual void initialize() override;
    virtual bool isReady() const override;

private:
    void buildEquation();

    DataInport<Volume, 0> inport_;
    VolumeOutport outport_;
    StringProperty eqn_;
    CompositeProperty scales_;

    Shader shader_;
    FrameBufferObject fbo_;
    bool validEquation_;
};

} // namespace

#endif // IVW_VOLUMECOMBINER_H
