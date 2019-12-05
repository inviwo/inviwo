/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_COLUMN_H
#define IVW_COLUMN_H

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/exception.h>

#include <inviwo/dataframe/datastructures/datapoint.h>

namespace inviwo {

class DataPointBase;
class BufferBase;

class IVW_MODULE_DATAFRAME_API InvalidConversion : public Exception {
public:
    InvalidConversion(const std::string &message = "",
                      ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~InvalidConversion() throw() {}
};

/**
 * \class Column
 * \brief pure interface for representing a data column, i.e. a Buffer with a name
 */
class IVW_MODULE_DATAFRAME_API Column {
public:
    virtual ~Column() = default;

    virtual Column *clone() const = 0;

    virtual const std::string &getHeader() const = 0;
    virtual void setHeader(const std::string &header) = 0;

    virtual void add(const std::string &value) = 0;

    virtual std::shared_ptr<BufferBase> getBuffer() = 0;
    virtual std::shared_ptr<const BufferBase> getBuffer() const = 0;

    virtual size_t getSize() const = 0;

    virtual double getAsDouble(size_t idx) const = 0;
    virtual dvec2 getAsDVec2(size_t idx) const = 0;
    virtual dvec3 getAsDVec3(size_t idx) const = 0;
    virtual dvec4 getAsDVec4(size_t idx) const = 0;

    virtual std::string getAsString(size_t idx) const = 0;
    virtual std::shared_ptr<DataPointBase> get(size_t idx, bool getStringsAsStrings) const = 0;

protected:
    Column() = default;
};

/**
 * \class TemplateColumn
 * \brief Data column used for plotting which represents a named buffer of type T. The name
 * is used as column header.
 */
template <typename T>
class TemplateColumn : public Column {
public:
    using type = T;

    TemplateColumn(const std::string &header,
                   std::shared_ptr<Buffer<T>> buffer = std::make_shared<Buffer<T>>());

    TemplateColumn(const std::string &header, std::vector<T> data);

    TemplateColumn(const TemplateColumn<T> &rhs);
    TemplateColumn(TemplateColumn<T> &&rhs);

    TemplateColumn<T> &operator=(const TemplateColumn<T> &rhs);
    TemplateColumn<T> &operator=(TemplateColumn<T> &&rhs);

    virtual TemplateColumn *clone() const override;

    virtual ~TemplateColumn() = default;

    virtual const std::string &getHeader() const override;
    void setHeader(const std::string &header) override;

    virtual void add(const T &value);
    /**
     * \brief converts given value to type T, which is added to the column
     *
     * @param value
     * @throws InvalidConversion if the value cannot be converted to T
     */
    virtual void add(const std::string &value) override;
    virtual void set(size_t idx, const T &value);

    T get(size_t idx) const;
    T operator[](size_t idx) const;
    /**
     * \brief returns the data value for the given index.
     *
     * @param idx    index
     * @param getStringsAsStrings   if true, strings will be returned for categorical values
     *           instead of their internal representation. This will not affect other column types.
     *
     * \see CategoricalColumn
     */
    virtual std::shared_ptr<DataPointBase> get(size_t idx, bool getStringsAsStrings) const override;

    virtual double getAsDouble(size_t idx) const override;

    virtual dvec2 getAsDVec2(size_t idx) const override;

    virtual dvec3 getAsDVec3(size_t idx) const override;

    virtual dvec4 getAsDVec4(size_t idx) const override;

    void setBuffer(std::shared_ptr<Buffer<T>> buffer);

    virtual std::string getAsString(size_t idx) const override;

    virtual std::shared_ptr<BufferBase> getBuffer() override;
    virtual std::shared_ptr<const BufferBase> getBuffer() const override;

    std::shared_ptr<Buffer<T>> getTypedBuffer();
    std::shared_ptr<const Buffer<T>> getTypedBuffer() const;

    virtual size_t getSize() const override;

protected:
    std::string header_;
    std::shared_ptr<Buffer<T>> buffer_;
};

/**
 * \class CategoricalColumn
 * \brief Specialized data column representing categorical values, i.e. strings.
 * Categorical values are internally mapped to a number representation.
 *
 * For example:
 *    The data column "blue", "blue", "red", "yellow" might internally be represented
 *    by 0, 0, 1, 2.
 *    The original string values can be accessed using CategoricalColumn::get(index, true)
 *
 * \see TemplateColumn, \see CategoricalColumn::get()
 */
class IVW_MODULE_DATAFRAME_API CategoricalColumn : public TemplateColumn<std::uint32_t> {
public:
    CategoricalColumn(const std::string &header);
    CategoricalColumn(const CategoricalColumn &rhs) = default;
    CategoricalColumn(CategoricalColumn &&rhs) = default;

    CategoricalColumn &operator=(const CategoricalColumn &rhs) = default;
    CategoricalColumn &operator=(CategoricalColumn &&rhs) = default;

    virtual CategoricalColumn *clone() const override;

    virtual ~CategoricalColumn() = default;

    virtual std::string getAsString(size_t idx) const override;

    /**
     * \brief returns either the categorical value, i.e. a number representation, or
     * the actual string for the given index.
     *
     * @param idx    index
     * @param getStringsAsStrings   if true, a string will be returned instead of the
     *             internal representation. This will not affect other column types.
     *
     * \see TemplateColumn
     */
    virtual std::shared_ptr<DataPointBase> get(size_t idx, bool getStringsAsStrings) const override;

    using TemplateColumn<std::uint32_t>::set;
    virtual void set(size_t idx, const std::string &str);

    virtual void add(const std::string &value) override;

    /**
     * Returns the unique set of categorical values.
     */
    const std::vector<std::string> &getCategories() const { return lookUpTable_; }

private:
    virtual glm::uint32_t addOrGetID(const std::string &str);

    std::vector<std::string> lookUpTable_;
};

template <typename T>
TemplateColumn<T>::TemplateColumn(const std::string &header, std::shared_ptr<Buffer<T>> buffer)
    : header_(header), buffer_(buffer) {}

template <typename T>
TemplateColumn<T>::TemplateColumn(const std::string &header, std::vector<T> data)
    : header_(header), buffer_(util::makeBuffer(std::move(data))) {}

template <typename T>
TemplateColumn<T>::TemplateColumn(const TemplateColumn &rhs)
    : header_(rhs.getHeader())
    , buffer_(std::shared_ptr<Buffer<T>>(rhs.getTypedBuffer()->clone())) {}

template <typename T>
TemplateColumn<T>::TemplateColumn(TemplateColumn<T> &&rhs)
    : header_(std::move(rhs.header_)), buffer_(std::move(rhs.buffer_)) {}

template <typename T>
TemplateColumn<T> &TemplateColumn<T>::operator=(const TemplateColumn<T> &rhs) {
    if (this != &rhs) {
        header_ = rhs.getHeader();
        buffer_ = std::shared_ptr<Buffer<T>>(rhs.getTypedBuffer()->clone());
    }
    return *this;
}

template <typename T>
TemplateColumn<T> &TemplateColumn<T>::operator=(TemplateColumn<T> &&rhs) {
    if (this != &rhs) {
        header_ = std::move(rhs.header_);
        buffer_ = std::move(rhs.buffer_);
    }
    return *this;
}

template <typename T>
TemplateColumn<T> *TemplateColumn<T>::clone() const {
    return new TemplateColumn(*this);
}

template <typename T>
const std::string &TemplateColumn<T>::getHeader() const {
    return header_;
}

template <typename T>
void TemplateColumn<T>::setHeader(const std::string &header) {
    header_ = header;
}

template <typename T>
void TemplateColumn<T>::add(const T &value) {
    buffer_->getEditableRAMRepresentation()->add(value);
}

namespace detail {

template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
void add(Buffer<T> *buffer, const std::string &value) {
    T result;
    std::istringstream stream(value);
    stream >> result;
    if (stream.fail()) {
        throw InvalidConversion("cannot convert \"" + value + "\" to target type");
    }
    buffer->getEditableRAMRepresentation()->add(result);
}
// Specialization for float and double types, add NaN instead of throwing an error
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
void add(Buffer<T> *buffer, const std::string &value) {
    T result;
    std::istringstream stream(value);
    stream >> result;
    if (stream.fail()) {
        buffer->getEditableRAMRepresentation()->add(std::numeric_limits<T>::quiet_NaN());
    } else {
        buffer->getEditableRAMRepresentation()->add(result);
    }
}

template <typename T,
          typename std::enable_if<!std::is_integral<T>::value && !std::is_floating_point<T>::value,
                                  int>::type = 0>
void add(Buffer<T> * /*buffer*/, const std::string &value) {
    throw InvalidConversion("conversion to target type not implemented (\"" + value + "\")");
}

}  // namespace detail

template <typename T>
void TemplateColumn<T>::add(const std::string &value) {
    detail::add<T>(buffer_.get(), value);
}

template <typename T>
void TemplateColumn<T>::set(size_t idx, const T &value) {
    buffer_->getEditableRAMRepresentation()->set(idx, value);
}

template <typename T>
T TemplateColumn<T>::get(size_t idx) const {
    auto val = buffer_->getRAMRepresentation()->getDataContainer()[idx];
    return val;
}

template <typename T>
double TemplateColumn<T>::getAsDouble(size_t idx) const {
    auto val = buffer_->getRAMRepresentation()->getDataContainer()[idx];
    return util::glm_convert<double>(val);
}

template <typename T>
dvec2 TemplateColumn<T>::getAsDVec2(size_t idx) const {
    auto val = buffer_->getRAMRepresentation()->getDataContainer()[idx];
    return util::glm_convert<dvec2>(val);
}

template <typename T>
dvec3 TemplateColumn<T>::getAsDVec3(size_t idx) const {
    auto val = buffer_->getRAMRepresentation()->getDataContainer()[idx];
    return util::glm_convert<dvec3>(val);
}

template <typename T>
dvec4 TemplateColumn<T>::getAsDVec4(size_t idx) const {
    auto val = buffer_->getRAMRepresentation()->getDataContainer()[idx];
    return util::glm_convert<dvec4>(val);
}

template <typename T>
void TemplateColumn<T>::setBuffer(std::shared_ptr<Buffer<T>> buffer) {
    buffer_ = buffer;
}

template <typename T>
std::string TemplateColumn<T>::getAsString(size_t idx) const {
    std::ostringstream ss;
    ss << buffer_->getRAMRepresentation()->get(idx);
    return ss.str();
}

template <typename T>
std::shared_ptr<DataPointBase> TemplateColumn<T>::get(size_t idx, bool) const {
    return std::make_shared<DataPoint<T>>(buffer_->getRAMRepresentation()->get(idx));
}

template <typename T>
T TemplateColumn<T>::operator[](const size_t idx) const {
    return get(idx);
}

template <typename T>
std::shared_ptr<BufferBase> TemplateColumn<T>::getBuffer() {
    return buffer_;
}

template <typename T>
std::shared_ptr<const BufferBase> TemplateColumn<T>::getBuffer() const {
    return buffer_;
}

template <typename T>
std::shared_ptr<Buffer<T>> TemplateColumn<T>::getTypedBuffer() {
    return buffer_;
}

template <typename T>
std::shared_ptr<const Buffer<T>> TemplateColumn<T>::getTypedBuffer() const {
    return buffer_;
}

template <typename T>
size_t TemplateColumn<T>::getSize() const {
    return buffer_->getSize();
}

}  // namespace inviwo

#endif  // IVW_COLUMN_H
