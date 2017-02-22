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


#include <modules/numpy/numpyobjectwrapper.h>

#include <arrayobject.h>
#include <ndarrayobject.h>


#include <inviwo/core/datastructures/image/image.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>


namespace {
    PyObject* init(std::vector<int> dims, int typenum, void * data, int flags) {
        ivwAssert(dims.size() <= 4, "Up to 4 dimensions supported");

        while(dims.back() == 1||dims.back() == 0) dims.pop_back(); // if the last dim is 0 or 1 it means we have a scalar field 
        npy_intp ndims[4] = { 0, 0, 0, 0 };
        for (int i = 0; i < dims.size(); i++) {
            ndims[i] = dims[i];
        }

        int nd = static_cast<int>(dims.size());
        return PyArray_New(&PyArray_Type, nd, ndims, typenum, NULL, data, 0, flags, NULL);
    }
}


namespace inviwo {

    template <>
    int df2numpyType<bool>() noexcept {
        return NPY_BOOL;
    }
    template <>
    int df2numpyType<char>() noexcept {
        return NPY_BYTE;
    }
    template <>
    int df2numpyType<unsigned char>() noexcept {
        return NPY_UBYTE;
    }
    template <>
    int df2numpyType<int>() noexcept {
        return NPY_INT;
    }
    template <>
    int df2numpyType<unsigned int>() noexcept {
        return NPY_UINT;
    }
    template <>
    int df2numpyType<long>() noexcept {
        return NPY_LONG;
    }
    template <>
    int df2numpyType<unsigned long>() noexcept {
        return NPY_ULONG;
    }
    template <>
    int df2numpyType<long long>() noexcept {
        return NPY_ULONGLONG;
    }
    template <>
    int df2numpyType<unsigned long long>() noexcept {
        return NPY_ULONGLONG;
    }

    template <>
    int df2numpyType<float>() noexcept {
        return NPY_FLOAT;
    }
    template <>
    int df2numpyType<double>() noexcept {
        return NPY_DOUBLE;
    }
    template <>
    int df2numpyType<long double>() noexcept {
        return NPY_LONGDOUBLE;
    }



    NumPyObjectWrapper::NumPyObjectWrapper(std::vector<int> dims, int typenum, void* data, bool rowmajor)
        : obj_(nullptr) {

        int flags = NPY_ARRAY_BEHAVED | (rowmajor ? NPY_ARRAY_F_CONTIGUOUS : NPY_ARRAY_C_CONTIGUOUS);
        obj_ = init(dims, typenum, data, flags);
    }




    NumPyObjectWrapper::NumPyObjectWrapper(std::vector<int> dims, int typenum, const void* data, bool rowmajor)
        : obj_(nullptr) {

        int flags = NPY_ARRAY_ALIGNED | (rowmajor ? NPY_ARRAY_F_CONTIGUOUS : NPY_ARRAY_C_CONTIGUOUS);
        obj_ = init(dims, typenum, const_cast<void*>( data ), flags);
    }

    pybind11::object NumPyObjectWrapper::getPyBindObject() { return pybind11::reinterpret_borrow<pybind11::object>(obj_); }
    //pybind11::object NumPyObjectWrapper::getPyBindObject() { return pybind11::object(pybind11::handle(obj_), false); }
//PyObject* NumPyObjectWrapper::getPyObj() { return obj_; }







namespace util{

    std::shared_ptr<NumPyObjectWrapper> toNumPyObject(LayerRAM* layer) {
        return layer->dispatch<std::shared_ptr<NumPyObjectWrapper>>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            using ComponentType = typename util::value_type<ValueType>::type;

            std::vector<int> dims = { static_cast<int>(layer->getDimensions().x)
                                    , static_cast<int>(layer->getDimensions().y)
                                    , static_cast<int>(layer->getDataFormat()->getComponents())
                                      };

            return std::make_shared<NumPyObjectWrapper>(dims, df2numpyType<ComponentType>(),
                typed->getData());
        });
    }


    std::shared_ptr<NumPyObjectWrapper> toNumPyObject(const LayerRAM* layer) {
        return layer->dispatch<std::shared_ptr<NumPyObjectWrapper>>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            using ComponentType = typename util::value_type<ValueType>::type;

            std::vector<int> dims = { static_cast<int>(layer->getDimensions().x)
                                    , static_cast<int>(layer->getDimensions().y)
                                    , static_cast<int>(layer->getDataFormat()->getComponents())
            };

            return std::make_shared<NumPyObjectWrapper>(dims, df2numpyType<ComponentType>(),
                typed->getData());
        });
    }

    std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<Image>>> toNumPyObject( std::shared_ptr<Image> img, size_t colorLayerIdx) {
        using RetType = NumPyObjectWrapperTyped<std::shared_ptr<Image>>;

        auto layer = img->getColorLayer(colorLayerIdx)->getEditableRepresentation<LayerRAM>();

        return layer->dispatch<std::shared_ptr<RetType>>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            using ComponentType = typename util::value_type<ValueType>::type;

            std::vector<int> dims = { static_cast<int>(layer->getDimensions().x)
                                    , static_cast<int>(layer->getDimensions().y)
                                    , static_cast<int>(layer->getDataFormat()->getComponents()) 
            };

            return std::make_shared<RetType>(
                img, df2numpyType<ComponentType>(), true, [&](std::shared_ptr<Image>& t) {
                return NumPyObjectWrapper::DimData{ dims, typed->getData() };
            });
        });
    }

    std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<const Image>>> toNumPyObject(std::shared_ptr<const Image> img, size_t colorLayerIdx) {
        using RetType = NumPyObjectWrapperTyped<std::shared_ptr<const Image>>;

        auto layer = img->getColorLayer(colorLayerIdx)->getRepresentation<LayerRAM>();

        return layer->dispatch<std::shared_ptr<RetType>>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            using ComponentType = typename util::value_type<ValueType>::type;

            std::vector<int> dims = { static_cast<int>(layer->getDimensions().x)
                                    , static_cast<int>(layer->getDimensions().y)
                                    , static_cast<int>(layer->getDataFormat()->getComponents()) 
            };

            return std::make_shared<RetType>(
                img, df2numpyType<ComponentType>(), true , [&](std::shared_ptr<const Image>& t) {
                return NumPyObjectWrapper::DimConstData{ dims, typed->getData() };
            });
        });
    }

    std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<Volume>>> toNumPyObject(std::shared_ptr<Volume> volume) {
        using RetType = NumPyObjectWrapperTyped<std::shared_ptr<Volume>>;

        auto volRAM = volume->getEditableRepresentation<VolumeRAM>();

        return volRAM->dispatch<std::shared_ptr<RetType>>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            using ComponentType = typename util::value_type<ValueType>::type;

            std::vector<int> dims = { static_cast<int>(volRAM->getDimensions().x)
                                    , static_cast<int>(volRAM->getDimensions().y)
                                    , static_cast<int>(volRAM->getDimensions().z)
                                    , static_cast<int>(volRAM->getDataFormat()->getComponents())
            };

            return std::make_shared<RetType>(
                volume, df2numpyType<ComponentType>(),true, [&](std::shared_ptr<Volume>& t) {
                return NumPyObjectWrapper::DimData{ dims, typed->getData() };
            });
        });


    }


    std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<const Volume>>> toNumPyObject(std::shared_ptr<const Volume> volume) {
        using RetType = NumPyObjectWrapperTyped<std::shared_ptr<const Volume>>;

        auto volRAM = volume->getRepresentation<VolumeRAM>();

        return volRAM->dispatch<std::shared_ptr<RetType>>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            using ComponentType = typename util::value_type<ValueType>::type;

            std::vector<int> dims = { static_cast<int>(volRAM->getDimensions().x)
                                    , static_cast<int>(volRAM->getDimensions().y)
                                    , static_cast<int>(volRAM->getDimensions().z)
                                    , static_cast<int>(volRAM->getDataFormat()->getComponents()) };

            return std::make_shared<RetType>(
                volume, df2numpyType<ComponentType>(),true, [&](std::shared_ptr<const Volume>& t) {
                return NumPyObjectWrapper::DimConstData{ dims, typed->getData() };
            });
        });


    }

    std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<BufferBase>>>  toNumPyObject(std::shared_ptr<BufferBase> buffer) {
        using RetType = NumPyObjectWrapperTyped<std::shared_ptr<BufferBase>>;

        return buffer->getEditableRepresentation<BufferRAM>()
            ->dispatch<std::shared_ptr<RetType>>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            using ComponentType = typename util::value_type<ValueType>::type;
            std::vector<int> dims = {static_cast<int>(typed->getSize()),
                                     static_cast<int>(typed->getDataFormat()->getComponents())};

            return std::make_shared<RetType>(
                buffer, df2numpyType<ComponentType>(),true, [&](std::shared_ptr<BufferBase>& t) {
                return NumPyObjectWrapper::DimData{ dims, typed->getData() };
            });
        });
    }

    std::shared_ptr<NumPyObjectWrapperTyped<std::shared_ptr<const BufferBase>>>  toNumPyObject(std::shared_ptr<const BufferBase> buffer) {
        using RetType = NumPyObjectWrapperTyped<std::shared_ptr<const BufferBase>>;

        return buffer->getRepresentation<BufferRAM>()
            ->dispatch<std::shared_ptr<RetType>>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            using ComponentType = typename util::value_type<ValueType>::type;

            std::vector<int> dims = {static_cast<int>(typed->getSize()),
                                     static_cast<int>(typed->getDataFormat()->getComponents())};

            return std::make_shared<RetType>(
                buffer, df2numpyType<ComponentType>(),true, [&](std::shared_ptr<const BufferBase>& t) {
                return NumPyObjectWrapper::DimConstData{ dims, typed->getData() };
            });
        });
    }

    template <typename T>
    std::vector<T> toVector(PyObject* obj) {
        using ComponentType = util::value_type<T>::type;
        std::vector<T> v;

        if (!PyArray_Check(obj)) {
            LogErrorCustom("numoyutils::toVector", "Not a NumPy Array");
            return v;
        }

        PyArrayObject* arrobj;

        if (!PyArray_OutputConverter(obj, &arrobj)) {
            LogErrorCustom("numoyutils::toVector", "Something went wrong");
            return v;
        }

        auto nd = PyArray_NDIM(arrobj);
        auto dims = PyArray_DIMS(arrobj);
        int extent = 0;
        if (nd > 2) {
            LogErrorCustom("numoyutils::toVector", "Cant handle dimensions of size " << nd);
            return v;
        }

        if (nd == 2) {
            extent = dims[1];
        }

        if (extent != util::extent<T>::value) {
            LogErrorCustom("numoyutils::toVector",
                           "The extent of the NumPy vector does not match the output vector, got "
                               << extent << " expected " << util::extent<T>::value);
            return v;
        }

        if (PyArray_EquivTypenums(PyArray_TYPE(arrobj), df2numpyType<ComponentType>()) ==
            NPY_FALSE) {
            LogErrorCustom("numoyutils::toVector", "Data types do not match");
            LogInfoCustom("", PyArray_TYPE(arrobj));
            LogInfoCustom("", df2numpyType<ComponentType>());
            return v;
        }

        v.resize(dims[0]);
        memcpy(v.data(), PyArray_DATA(arrobj), dims[0] * sizeof(T));  //*/
        return v;
    }

    std::shared_ptr<BasicMesh> toMesh(PyObject* obj) {
        if(!obj){
            LogErrorCustom("numoyutils::toMesh" , "Not a BasicMesh, got nullptr");
            return nullptr;
        }
        if(strcmp(obj->ob_type->tp_name , "BasicMesh")!=0){
            LogErrorCustom("numoyutils::toMesh" , "Not a BasicMesh, got " << obj->ob_type->tp_name);
            return nullptr;
        }



        auto verticesPy = PyObject_GetAttrString(obj, "vertices");
        auto texCoordsPy = PyObject_GetAttrString(obj, "texCoords");
        auto normalsPy = PyObject_GetAttrString(obj, "normals");
        auto colorsPy = PyObject_GetAttrString(obj, "colors");
        auto triangleListPy = PyObject_GetAttrString(obj, "triangleList");

        auto vertices = util::toVector<vec3>(verticesPy);
        auto texCoords = util::toVector<vec3>(texCoordsPy);
        auto normals = util::toVector<vec3>(normalsPy);
        auto colors = util::toVector<vec4>(colorsPy);
        auto trianlges = util::toVector<glm::uint32>(triangleListPy);


        
        auto mesh = std::make_shared<BasicMesh>();
        auto ind = mesh->addIndexBuffer(DrawType::Triangles,ConnectivityType::None);
        

        auto insert = [](auto &v1,auto &v2){
            v1.insert(v1.begin(),v2.begin(),v2.end());
        };
        insert(mesh->getEditableVertices()->getEditableRAMRepresentation()->getDataContainer(), vertices);
        insert(mesh->getEditableTexCoords()->getEditableRAMRepresentation()->getDataContainer(), texCoords);
        insert(mesh->getEditableNormals()->getEditableRAMRepresentation()->getDataContainer(), normals);
        insert(mesh->getEditableColors()->getEditableRAMRepresentation()->getDataContainer(), colors);

        insert(ind->getDataContainer(), trianlges);


       

        return mesh;

    }

} //namspace util




}  // namespace inviwo
