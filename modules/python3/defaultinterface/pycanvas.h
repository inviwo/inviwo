/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_PYCANVASMEHTODSINVIWO_H
#define IVW_PYCANVASMEHTODSINVIWO_H



#include <modules/python3/python3moduledefine.h>

#include <modules/python3/pythoninterface/pymethod.h>

namespace inviwo {
PyObject* py_canvascount(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_resizecanvas(PyObject* /*self*/, PyObject* /*args*/);


class IVW_MODULE_PYTHON3_API PyCanvasCountMethod : public PyMethod {
public:
    virtual std::string getName()const {return "canvasCount";}
    virtual std::string getDesc()const {return "Returns the number of canvases in the current network.";}
    virtual PyCFunction getFunc() {return py_canvascount;}
};

class IVW_MODULE_PYTHON3_API PyResizeCanvasMethod : public PyMethod {
public:
    PyResizeCanvasMethod();
    virtual ~PyResizeCanvasMethod() {}
    virtual std::string getName()const {return "resizeCanvas";}
    virtual std::string getDesc()const {return "Resizes the canvas in the network to the given size. Canvas can either be given using a canvas index (starting at 0) or a canvas ID string ";}
    virtual PyCFunction getFunc() {return py_resizecanvas;}
private:
    PyParamVarious canvas_;
    PyParamInt newWidth_;
    PyParamInt newHeight_;
};

} //namespace


#endif // IVW_PYCANVASMEHTODSINVIWO_H


