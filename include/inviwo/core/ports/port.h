/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_PORT_H
#define IVW_PORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/introspection.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

class Processor;

/**
 *    Traits class to make ports and data less intertwined. Port traits will by default ask
 *    it's data for a class identifier, a color code, and data info. You can specialize port
 *  traits for type that does not have those methods, and where you can't add them easily.
 *  Note that if a method is missing we will still compile and fail gracefully.
 */
template <typename T>
struct port_traits {
    static std::string class_identifier() { return util::class_identifier<T>(); }
    static uvec3 color_code() { return util::color_code<T>(); }
    static std::string data_info(const T* data) { return util::data_info<T>(data); }
};

/** \class Port
 * \brief A abstract base class for all ports.
 * A port can be connected to other ports and is owned by a processor.
 */
class IVW_CORE_API Port : public Serializable {
    friend class Processor;

public:
    virtual ~Port() = default;
    std::string getIdentifier() const;
    Processor* getProcessor() const;

    /**
     * Returns the RGB color code used to colorize all ports of this type. This color code is for
     * instance used in the NetworkEditor. To distinguish different port types through their color,
     * this method should be overloaded in derived classes.
     */
    virtual uvec3 getColorCode() const = 0;
    virtual std::string getClassIdentifier() const = 0;
    virtual std::string getContentInfo() const = 0;

    virtual bool isConnected() const = 0;
    virtual bool isReady() const = 0;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    /**
     * Constructor for creating port instances. As this class is abstract, the constructor is not
     * called directly. Instead, constructors of the derived classes call this constructor.
     * @param identifier Port identifier used for serialization. Has to be unique within the scope
     *                   of a processor.
     * @see Processor::addPort()
     */
    Port(std::string identifier = "");

    void setIdentifier(const std::string& name);
    void setProcessor(Processor* processor);

    std::string identifier_;
    Processor* processor_;  //< non-owning reference
};

// Specialization for vectors.
template <typename T, typename Alloc>
struct port_traits<std::vector<T, Alloc>> {
    static std::string class_identifier() { return port_traits<T>::class_identifier() + "Vector"; }
    static uvec3 color_code() { return glm::min(uvec3(30, 30, 30) + port_traits<T>::color_code(), uvec3(255)); }
    static std::string data_info(const std::vector<T, Alloc>* data) {
        return "Vector of size " + toString(data->size()) +
               (!data->empty()
                    ? "<br/>with elements of:<br/>" + port_traits<T>::data_info(&(data->front()))
                    : " ");
    }
};
template <typename T, typename Alloc>
struct port_traits<std::vector<T*, Alloc>> {
    static std::string class_identifier() {
        return port_traits<T>::class_identifier() + "PtrVector";
    }
    static uvec3 color_code() { return glm::min(uvec3(30, 30, 30) + port_traits<T>::color_code(), uvec3(255)); }
    static std::string data_info(const std::vector<T*, Alloc>* data) {
        return "Vector of size " + toString(data->size()) +
               (!data->empty()
                    ? "<br/>with pointers to:<br/>" + port_traits<T>::data_info(data->front())
                    : " ");
    }
};
template <typename T, typename D, typename A>
struct port_traits<std::vector<std::unique_ptr<T, D>, A>> {
    static std::string class_identifier() {
        return port_traits<T>::class_identifier() + "UniquePtrVector";
    }
    static uvec3 color_code() { return glm::min(uvec3(30, 30, 30) + port_traits<T>::color_code(), uvec3(255)); }
    static std::string data_info(const std::vector<std::unique_ptr<T, D>, A>* data) {
        return "Vector of size " + toString(data->size()) +
               (!data->empty()
                    ? "<br/>with unique pointers to:<br/> " +
                          port_traits<T>::data_info(data->front().get())
                    : " ");
    }
};
template <typename T, typename A>
struct port_traits<std::vector<std::shared_ptr<T>, A>> {
    static std::string class_identifier() {
        return port_traits<T>::class_identifier() + "SharedPtrVector";
    }
    static uvec3 color_code() { return glm::min(uvec3(30, 30, 30) + port_traits<T>::color_code(), uvec3(255)); }
    static std::string data_info(const std::vector<std::shared_ptr<T>, A>* data) {
        return "Vector of size " + toString(data->size()) +
               (!data->empty()
                    ? "<br/>with shared pointers to:<br/> " +
                          port_traits<T>::data_info(data->front().get())
                    : " ");
    }
};



template <>
struct port_traits<Vector<2, float>> {
    static std::string class_identifier() { return "vec2"; }
    static uvec3 color_code() { return uvec3(50, 100, 0); }
    static std::string data_info(const Vector<2, float>* data) {
        return class_identifier();
    }
};


template <>
struct port_traits<Vector<2, double>> {
    static std::string class_identifier() { return "dvec2"; }
    static uvec3 color_code() { return uvec3(100, 50, 0); }
    static std::string data_info(const Vector<2, double>* data) {
        return class_identifier();
    }
};


template <>
struct port_traits<Vector<2, int>> {
    static std::string class_identifier() { return "ivec2"; }
    static uvec3 color_code() { return uvec3(0, 50, 100); }
    static std::string data_info(const Vector<2, int>* data) {
        return class_identifier();
    }
};


template <>
struct port_traits<Vector<3, float>> {
    static std::string class_identifier() { return "vec3"; }
    static uvec3 color_code() { return uvec3(75, 150, 0); }
    static std::string data_info(const Vector<3, float>* data) {
        return class_identifier();
    }
};


template <>
struct port_traits<Vector<3, double>> {
    static std::string class_identifier() { return "dvec3"; }
    static uvec3 color_code() { return uvec3(150, 75, 0); }
    static std::string data_info(const Vector<3, double>* data) {
        return class_identifier();
    }
};


template <>
struct port_traits<Vector<3, int>> {
    static std::string class_identifier() { return "ivec3"; }
    static uvec3 color_code() { return uvec3(0, 75, 150); }
    static std::string data_info(const Vector<3, int>* data) {
        return class_identifier();
    }
};


template <>
struct port_traits<Vector<4, float>> {
    static std::string class_identifier() { return "vec4"; }
    static uvec3 color_code() { return uvec3(100, 200, 0); }
    static std::string data_info(const Vector<4, float>* data) {
        return class_identifier();
    }
};


template <>
struct port_traits<Vector<4, double>> {
    static std::string class_identifier() { return "dvec4"; }
    static uvec3 color_code() { return uvec3(200, 100, 0); }
    static std::string data_info(const Vector<4, double>* data) {
        return class_identifier();
    }
};


template <>
struct port_traits<Vector<4, int>> {
    static std::string class_identifier() { return "ivec4"; }
    static uvec3 color_code() { return uvec3(0, 100, 200); }
    static std::string data_info(const Vector<4, int>* data) {
        return class_identifier();
    }
};

}  // namespace




#endif  // IVW_PORT_H
