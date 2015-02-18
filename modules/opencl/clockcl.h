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

#ifndef IVW_CLOCK_CL_H
#define IVW_CLOCK_CL_H

#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/cl.hpp>
#include <inviwo/core/util/clock.h>
#include <inviwo/core/util/assertion.h>

namespace inviwo {

// Note: This file is named clockcl.h (event though it only contains ScopedClockCL) to be consistent with clock.h and clockgl.h 

/** \class ScopedClockCL
 *
 * Scoped timer for OpenCL that prints elapsed time in destructor. 
 * Usage is simplified by the macros (does nothing unless IVW_PROFILING is defined)
 * IVW_OPENCL_PROFILING(profilingEvent, "My message")
 * OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*kernel, cl::NullRange, globalWorkSize, workGroupSize, NULL, profilingEvent);
 *
 */
class IVW_MODULE_OPENCL_API ScopedClockCL {
public:
    ScopedClockCL(cl::Event* profilingEvent, const std::string& logSource, const std::string& message): profilingEvent_(profilingEvent), logSource_(logSource), logMessage_(message) { ivwAssert(profilingEvent != NULL, "Cannot initialize ScopedClockCL with NULL OpenCL event"); }
    virtual ~ScopedClockCL(); 

private:
    // Default constructor not allowed
    ScopedClockCL() {};
    cl::Event* profilingEvent_;
    std::string logSource_; 
    std::string logMessage_;
};

#if IVW_PROFILING 
#define IVW_OPENCL_PROFILING(var,message) \
    std::ostringstream ADDLINE(__stream); ADDLINE(__stream) << message; \
    cl::Event* var = new cl::Event(); \
    ScopedClockCL ADDLINE(__clock)(var, parseTypeIdName(std::string(typeid(this).name())), ADDLINE(__stream).str());
#else
#define IVW_OPENCL_PROFILING(var,message) cl::Event* var = NULL;
#endif
} // namespace

#endif // IVW_CLOCK_CL_H
