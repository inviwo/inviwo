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

#ifndef IVW_IMAGEGLPROCESSOR_H
#define IVW_IMAGEGLPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {


/*! \class ImageGLProcessor
 *
 * \brief Base class for image processing on the GPU using OpenGL.
 *
 * The ImageGLProcessor provides the basic structure for image processing on the GPU.
 * Derived shaders have to provide a custom fragment shader which is used during rendering.
 * Optionally, derived classes can overwrite ImageGLProcessor::postProcess() to perform
 * post-processing of the image data set in the outport. Furthermore, it is possible to
 * be notified of changes in the input image by overwriting ImageGLProcessor::afterInportChanged().
 *
 * \see VolumeGLProcessor
 */
class IVW_MODULE_BASEGL_API ImageGLProcessor : public Processor { 
public:
    ImageGLProcessor(std::string fragmentShader);
    virtual ~ImageGLProcessor();

    virtual void initialize() override;

    virtual void process();
protected:
    void markInvalid() { internalInvalid_ = true; }

    /*! \brief this function gets called right before the actual processing but 
     *         after the shader has been activated
     *
     * overwrite this function in the derived class to perform things like custom shader setup
     */
    virtual void preProcess(){}

    /*! \brief this function gets called at the end of the process function
     *
     * overwrite this function in the derived class to perform post-processing
     */
    virtual void postProcess(){}

    /*! \brief this function gets called whenever the inport changes
     *
     * overwrite this function in the derived class to be notified of inport onChange events
     */
    virtual void afterInportChanged() {}

    ImageInport inport_;
    ImageOutport outport_;
    
    bool internalInvalid_;

    std::string fragmentShader_;

    Shader shader_;

    /*! \brief call-back function for onChange events of the inport
     */
    void inportChanged() {
        markInvalid();
        afterInportChanged();
    }
};

} // namespace

#endif // IVW_IMAGEGLPROCESSOR_H

