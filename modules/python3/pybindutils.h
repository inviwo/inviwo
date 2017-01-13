/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_PYBINDUTILS_H
#define IVW_PYBINDUTILS_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/python3/pythonincluder.h>
#include <pybind11/pybind11.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>


namespace inviwo {

    namespace pyutil {

        /*IVW_MODULE_PYTHON3_API InviwoApplication* getApplication();
        IVW_MODULE_PYTHON3_API ProcessorNetwork* getNetwork();
        IVW_MODULE_PYTHON3_API Processor* getProcessor(std::string id);*/

       /* template <typename T>
        T* getTypedProcessor(std::string id) {
            auto p = getProcessor(id);
            auto pt = dynamic_cast<T*>(p);
            if (!pt) {
                std::stringstream ss;
                ss << "Wrong Processor type. Processor with identifier " << id << " is of type"
                   << p->getClassIdentifier();

                throw std::exception(ss.str().c_str());
            }
            return pt;
        }

*/
        /*template<typename T>
        T parse(PyObject *obj) {
            return pybind11::handle(obj).cast<T>();
        }*/

       /* template<typename T>
        T parse(const pybind11::object &obj) {
            return obj.cast<T>();
        }*/

        /*  template<typename T>
          bool is(PyObject *obj) {
              auto type1 = pybind11::handle::handle(obj).get_type();
              auto type2 = pybind11::cast<T>(T()).get_type();
              return type1 == type2;
          }


          template<typename T>
          bool is(const pybind11::object &obj) {
              auto type1 = obj.get_type();
              auto type2 = pybind11::cast<T>(T()).get_type();
              return type1 == type2;
          }

          
          
          template<typename T> pybind11::object toPyBindObject(const T &t) {
              return pybind11::cast(t);
          }
          */

    }


} // namespace

#endif // IVW_NUMPYUTILS_H

