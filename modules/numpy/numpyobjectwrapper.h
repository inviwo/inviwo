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

#ifndef IVW_NUMPYOBJECTWRAPPER_H
#define IVW_NUMPYOBJECTWRAPPER_H

#include <modules/numpy/numpymoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/python3/pythonincluder.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <pybind11/pybind11.h>

#include <ndarraytypes.h>

namespace inviwo {
    class BufferBase;
    class Image;
    class Volume;
    class LayerRAM;
    class BasicMesh;

    template<typename T> int df2numpyType() noexcept { return NPY_NOTYPE; }
    template<> IVW_MODULE_NUMPY_API int df2numpyType<bool>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<char>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<unsigned char>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<int>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<unsigned int>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<long>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<unsigned long>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<long long>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<unsigned long long>()noexcept;

    template<> IVW_MODULE_NUMPY_API int df2numpyType<float>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<double>()noexcept;
    template<> IVW_MODULE_NUMPY_API int df2numpyType<long double>()noexcept;


class IVW_MODULE_NUMPY_API NumPyObjectWrapper { 
public:
    using DimData = std::pair<std::vector<int>, void*>;
    using DimConstData = std::pair<std::vector<int>, const void*>;

    /**
     * Takes a pointer to data with given dimensions and type and wraps it in a NumPy object.
     * OBS: Does not take ownership the data and the caller need to ensure the life time of the data
     */
    NumPyObjectWrapper(std::vector<int> dims, int typenum, void *data, bool rowmajor = true);
    NumPyObjectWrapper(std::vector<int> dims, int typenum, const void *data, bool rowmajor = true);

    NumPyObjectWrapper(const DimData &p, int typenum, bool rowmajor = true) : NumPyObjectWrapper(p.first, typenum, p.second, rowmajor) { }
    NumPyObjectWrapper(const DimConstData &p, int typenum, bool rowmajor = true) : NumPyObjectWrapper(p.first, typenum, p.second, rowmajor) { }

    pybind11::object getPyBindObject();
  //  PyObject* getPyObj();
    virtual ~NumPyObjectWrapper() = default;

private:
    


    PyObject* obj_;
};


template<typename T> 
class NumPyObjectWrapperTyped{
public:
    template<typename Callback>
    NumPyObjectWrapperTyped(const T &t, int typenum, bool rowmajor, Callback c)
        : tCopy_(t)
        , obj_(c(tCopy_), typenum, rowmajor) {}

    pybind11::object getPyBindObject() { return obj_.getPyBindObject(); }
    virtual ~NumPyObjectWrapperTyped() = default;

private:
    T tCopy_;
    NumPyObjectWrapper obj_;
};

namespace util{

    IVW_MODULE_NUMPY_API
        std::shared_ptr<NumPyObjectWrapper> toNumPyObject(LayerRAM* layer);

    IVW_MODULE_NUMPY_API
        std::shared_ptr<NumPyObjectWrapper> toNumPyObject(const LayerRAM* layer);

    IVW_MODULE_NUMPY_API
        std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<Image>>> toNumPyObject(std::shared_ptr<Image> layer, size_t colorLayerIdx = 0);


    IVW_MODULE_NUMPY_API
        std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<const Image>>> toNumPyObject(std::shared_ptr<const Image> layer, size_t colorLayerIdx = 0);


    IVW_MODULE_NUMPY_API
        std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<Volume>>> toNumPyObject(std::shared_ptr<Volume> volume);


    IVW_MODULE_NUMPY_API
        std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<const Volume>>> toNumPyObject(std::shared_ptr<const Volume> volume);

    IVW_MODULE_NUMPY_API
        std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<BufferBase>>>  toNumPyObject(std::shared_ptr<BufferBase> buffer);

    template <typename T>
    std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<BufferBase>>>  toNumPyObject(std::shared_ptr<Buffer<T>> buffer) {
        return util::toNumPyObject(std::static_pointer_cast<BufferBase>(buffer));
    }


    template <typename T>
    std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<const BufferBase>>>  toNumPyObject(std::shared_ptr<const Buffer<T>> buffer) {
        return util::toNumPyObject(std::static_pointer_cast<const BufferBase>(buffer));
    }

    IVW_MODULE_NUMPY_API
        std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<const BufferBase>>>  toNumPyObject(std::shared_ptr<const BufferBase> buffer);

    template <typename T>
    std::shared_ptr<NumPyObjectWrapper> toNumPyObject(std::vector<T> &v) {
        using ComponentType = typename util::value_type<T>::type;
        std::vector<int> dims = { static_cast<int>(v.size()) , static_cast<int>(util::extent<T>::value) };
        return std::make_shared<NumPyObjectWrapper>(dims, df2numpyType<ComponentType>(), v.data() , false);
    }

    template <typename T>
    std::shared_ptr<NumPyObjectWrapperTyped<std::vector<T>>> toNumPyObject(const std::vector<T> &v) {
        using ComponentType = typename util::value_type<T>::type;
        std::vector<int> dims = { static_cast<int>(v.size()) , static_cast<int>(util::extent<T>::value) };
        
        return std::make_shared<NumPyObjectWrapperTyped<std::vector<T>>> 
            (v ,  df2numpyType<ComponentType>() , [&]( std::vector<T> &t){  
            return   NumPyObjectWrapper::DimData{dims,t.data()} ; 
        });
    }


    IVW_MODULE_NUMPY_API
    std::shared_ptr<BasicMesh> toMesh(PyObject* obj);

}

} // namespace

#endif // IVW_NUMPYOBJECTWRAPPER_H

