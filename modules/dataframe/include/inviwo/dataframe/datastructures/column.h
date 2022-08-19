/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2021 Inviwo Foundation
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

#pragma once

#include <inviwo/dataframe/dataframemoduledefine.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/transformiterator.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/fmtutils.h>
#include <inviwo/core/metadata/metadataowner.h>

#include <modules/base/algorithm/dataminmax.h>

#include <optional>
#include <iosfwd>

namespace inviwo {

class BufferBase;

class IVW_MODULE_DATAFRAME_API InvalidConversion : public Exception {
public:
    InvalidConversion(const std::string& message = "",
                      ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~InvalidConversion() throw() {}
};

enum class ColumnType { Index, Ordinal, Categorical };
IVW_MODULE_DATAFRAME_API std::string_view enumToStr(ColumnType b);
IVW_MODULE_DATAFRAME_API std::ostream& operator<<(std::ostream& ss, ColumnType type);

/**
 * @brief Pure interface for representing a data column with a header, optional units, and data
 * range.
 * @ingroup datastructures
 */
class IVW_MODULE_DATAFRAME_API Column : public MetaDataOwner {
public:
    virtual ~Column() = default;

    virtual Column* clone() const = 0;
    virtual Column* clone(const std::vector<std::uint32_t>& rowSelection) const = 0;

    virtual ColumnType getColumnType() const = 0;

    virtual const std::string& getHeader() const = 0;
    virtual void setHeader(std::string_view header) = 0;

    virtual Unit getUnit() const = 0;
    virtual void setUnit(Unit unit) = 0;

    /**
     * Set a custom range for the column which can be used for normalization, plotting, color
     * mapping, etc.
     */
    virtual void setCustomRange(std::optional<dvec2>) = 0;

    /**
     * Returns the custom column range. If no range has been set previously, the return
     * value will be std::nullopt.
     */
    virtual std::optional<dvec2> getCustomRange() const = 0;

    /**
     * Returns the range (i.e. the min/max) of the data, ignoring any custom range
     */
    virtual dvec2 getDataRange() const = 0;

    /**
     * Returns the custom range if set or else the range (i.e. the min/max) of the data
     */
    virtual dvec2 getRange() const = 0;

    virtual void add(std::string_view value) = 0;
    /**
     * Appends all rows from column \p col
     * @param col
     */
    virtual void append(const Column& col) = 0;

    virtual std::shared_ptr<BufferBase> getBuffer() = 0;
    virtual std::shared_ptr<const BufferBase> getBuffer() const = 0;

    virtual size_t getSize() const = 0;

    virtual double getAsDouble(size_t idx) const = 0;

    virtual std::string getAsString(size_t idx) const = 0;

protected:
    Column() = default;
};

/**
 * @brief Data column used for plotting which represents a named buffer of type T. The name
 * is used as column header.
 * @ingroup datastructures
 */
template <typename T>
class TemplateColumn : public Column {
public:
    using type = T;

    explicit TemplateColumn(std::string_view header,
                            std::shared_ptr<Buffer<T>> buffer = std::make_shared<Buffer<T>>(),
                            Unit unit = Unit{}, std::optional<dvec2> range = std::nullopt);

    TemplateColumn(std::string_view header, std::vector<T> data, Unit unit = Unit{},
                   std::optional<dvec2> range = std::nullopt);

    TemplateColumn(std::string_view header, size_t size, Unit unit = Unit{},
                   std::optional<dvec2> range = std::nullopt);

    TemplateColumn(const TemplateColumn<T>& rhs);
    TemplateColumn(const TemplateColumn<T>& rhs, const std::vector<std::uint32_t>& rowSelection);
    TemplateColumn(TemplateColumn<T>&& rhs);

    TemplateColumn<T>& operator=(const TemplateColumn<T>& rhs);
    TemplateColumn<T>& operator=(TemplateColumn<T>&& rhs);

    virtual TemplateColumn* clone() const override;
    virtual TemplateColumn* clone(const std::vector<std::uint32_t>& rowSelection) const override;

    virtual ~TemplateColumn() = default;

    virtual ColumnType getColumnType() const override;

    virtual const std::string& getHeader() const override;
    void setHeader(std::string_view header) override;

    virtual Unit getUnit() const override;
    virtual void setUnit(Unit unit) override;

    virtual void setCustomRange(std::optional<dvec2> range) override;
    virtual std::optional<dvec2> getCustomRange() const override;
    virtual dvec2 getDataRange() const override;
    virtual dvec2 getRange() const override;

    virtual void add(const T& value);
    /**
     * \brief Converts given value to type T, which is added to the column
     *
     * @param value
     * @throws InvalidConversion if the value cannot be converted to T
     */
    virtual void add(std::string_view value) override;

    /**
     * @copydoc inviwo::Column::append(const Column&)
     * @throws Exception if data format does not match
     */
    virtual void append(const Column& col) override;

    virtual void set(size_t idx, const T& value);

    T get(size_t idx) const;
    T operator[](size_t idx) const;

    virtual double getAsDouble(size_t idx) const override;

    void setBuffer(std::shared_ptr<Buffer<T>> buffer);

    virtual std::string getAsString(size_t idx) const override;

    virtual std::shared_ptr<BufferBase> getBuffer() override;
    virtual std::shared_ptr<const BufferBase> getBuffer() const override;

    std::shared_ptr<Buffer<T>> getTypedBuffer();
    std::shared_ptr<const Buffer<T>> getTypedBuffer() const;

    virtual size_t getSize() const override;

    auto begin() { return buffer_->getEditableRAMRepresentation()->getDataContainer().begin(); }
    auto end() { return buffer_->getEditableRAMRepresentation()->getDataContainer().end(); }
    auto begin() const { return buffer_->getRAMRepresentation()->getDataContainer().begin(); }
    auto end() const { return buffer_->getRAMRepresentation()->getDataContainer().end(); }

protected:
    std::string header_;
    Unit unit_;
    std::optional<dvec2> range_;
    std::shared_ptr<Buffer<T>> buffer_;
};

class IVW_MODULE_DATAFRAME_API IndexColumn : public TemplateColumn<std::uint32_t> {
public:
    IndexColumn(std::string_view header, std::shared_ptr<Buffer<std::uint32_t>> buffer =
                                             std::make_shared<Buffer<std::uint32_t>>());
    IndexColumn(std::string_view header, std::vector<std::uint32_t> data);
    IndexColumn(const IndexColumn& rhs) = default;
    IndexColumn(const IndexColumn& rhs, const std::vector<std::uint32_t>& rowSelection);
    IndexColumn(IndexColumn&& rhs) = default;
    IndexColumn& operator=(const IndexColumn& rhs) = default;
    IndexColumn& operator=(IndexColumn&& rhs) = default;

    virtual IndexColumn* clone() const override;
    virtual IndexColumn* clone(const std::vector<std::uint32_t>& rowSelection) const override;

    virtual ~IndexColumn() = default;

    virtual ColumnType getColumnType() const override;
};

namespace detail {
inline auto categoricalTransform(const std::vector<std::string>& table) {
    return [&table](std::uint32_t idx) -> const std::string& { return table[idx]; };
}
}  // namespace detail

/**
 * \brief Specialized data column representing categorical values, i.e. strings.
 * Categorical values are internally mapped to a number representation.
 *
 * For example:
 *    The data column "blue", "blue", "red", "yellow" might internally be represented
 *    by 0, 0, 1, 2.
 *    The original string values can be accessed using CategoricalColumn::get(index, true)
 *
 * \see CategoricalColumn::get()
 * @ingroup datastructures
 */
class IVW_MODULE_DATAFRAME_API CategoricalColumn : public Column {
    class AddMany;

public:
    using type = std::uint32_t;
    using ConstIterator =
        util::TransformIterator<decltype(detail::categoricalTransform(
                                    std::declval<const std::vector<std::string>&>())),
                                std::vector<std::uint32_t>::const_iterator>;

    CategoricalColumn(std::string_view header, const std::vector<std::string>& values = {},
                      Unit unit = Unit{}, std::optional<dvec2> range = std::nullopt);
    CategoricalColumn(const CategoricalColumn& rhs);
    CategoricalColumn(const CategoricalColumn& rhs, const std::vector<std::uint32_t>& rowSelection);
    CategoricalColumn(CategoricalColumn&& rhs) = default;
    CategoricalColumn& operator=(const CategoricalColumn& rhs);
    CategoricalColumn& operator=(CategoricalColumn&& rhs);

    virtual CategoricalColumn* clone() const override;
    virtual CategoricalColumn* clone(const std::vector<std::uint32_t>& rowSelection) const override;

    virtual ~CategoricalColumn() = default;

    virtual ColumnType getColumnType() const override;

    virtual const std::string& getHeader() const override;
    void setHeader(std::string_view header) override;

    virtual Unit getUnit() const override;
    virtual void setUnit(Unit unit) override;

    virtual void setCustomRange(std::optional<dvec2> range) override;
    virtual std::optional<dvec2> getCustomRange() const override;
    virtual dvec2 getDataRange() const override;
    virtual dvec2 getRange() const override;

    virtual size_t getSize() const override;

    /**
     * Set the value of row @p idx to @p str.
     */
    void set(size_t idx, std::string_view str);
    /**
     * Set the value of row @p idx by using the numerical @p id of a category.
     * @throws RangeException if @p id does not correspond to a registered category
     * @see addCategory
     */
    void set(size_t idx, std::uint32_t id);

    ///@{
    /**
     * Return the categorical value in row @p idx.
     */
    const std::string& get(size_t idx) const;
    virtual std::string getAsString(size_t idx) const override;
    ///@}

    ///@{
    /**
     * Return the numerical id of the category in row @p idx.
     */
    std::uint32_t getId(size_t idx) const;
    virtual double getAsDouble(size_t idx) const override;
    ///@}

    virtual void add(std::string_view value) override;

    /*
     * For CategoricalColumn you cannot do the trick of pulling out the data container
     * `auto& data = templateColumn->getEditableRAMRepresentation()->getDataContainer();`
     * like for regular TemplateColumns to efficiently add many elements.
     * This is because adding element to the CategoricalColumn first has to find the string in the
     * lookup table and then insert the corresponding index into the data container. And we can't
     * do that from the "outside". This function returns a callable object `AddMany` that enables
     * this optimization for CategoricalColumns.
     * For example:
     * ```{.cpp}
     * CategoricalColumn col{...};
     * auto adder = col.addMany();
     * for (const auto& str : some_strings) {
     *     adder(str);
     * }
     * ```
     */
    AddMany addMany();

    /**
     * \brief \copybrief Column::append(const Column&) and builds a union of all
     * categorical values
     *
     * @param col
     * @throws Exception if data format does not match
     */
    virtual void append(const Column& col) override;

    /**
     * \brief Append the categorical values given in \p data
     *
     * @param data    categorical values
     */
    void append(const std::vector<std::string>& data);

    /**
     * Returns the unique set of categorical values.
     */
    const std::vector<std::string>& getCategories() const { return lookUpTable_; }

    /**
     * \brief Returns column contents as list of categorical values
     *
     * @return all categorical values stored in the column
     * @see values
     */
    [[deprecated("use iterators or values() instead")]] std::vector<std::string> getValues() const;

    /**
     * Returns a const iterator range over all rows of the column holding the corresponding
     * categorical values as <tt>const std::string&</tt>.
     *
     * Example:
     * \code{.cpp}
     * CategoricalColumn col("example", {"first", "second"});
     * for (auto v : col.values()) {
     *     std::cout << "value: " << v;
     * }
     * for (auto&& [row, v] : util::enumerate<std::uint32_t>(col.values())) {
     *     std::cout << fmt::format("Row: {}, value: {}", row, v);
     * }
     * \endcode
     *
     * @return iterator range over all categorical values stored in the column
     * @see getValues
     */
    util::iter_range<ConstIterator> values() const;

    /**
     * \brief Add a category \p cat. It will not be added if the category already exists.
     *
     * @return index of the category
     */
    std::uint32_t addCategory(std::string_view cat);

    ///@{
    /**
     * Returns a const iterator over all rows of the column holding the corresponding
     * categorical values as <tt>const std::string&</tt>.
     * @return iterator over categorical values stored in the column
     * @see values
     */
    ConstIterator begin() const;
    ConstIterator end() const;
    ///@}

    ///@{
    /**
     * Return the buffer holding the internal representations (@c std::uint32_t) of the categorical
     * values for the entire column.
     */
    virtual std::shared_ptr<BufferBase> getBuffer() override;
    virtual std::shared_ptr<const BufferBase> getBuffer() const override;

    std::shared_ptr<Buffer<std::uint32_t>> getTypedBuffer();
    std::shared_ptr<const Buffer<std::uint32_t>> getTypedBuffer() const;
    ///@}

private:
    class IVW_MODULE_DATAFRAME_API AddMany {
    public:
        inline void operator()(std::string_view value) const {
            const auto id = column_->addOrGetID(value);
            buffer_->getDataContainer().push_back(id);
        }

    private:
        friend CategoricalColumn;
        AddMany(CategoricalColumn* column, BufferRAMPrecision<std::uint32_t>* buffer)
            : column_{column}, buffer_{buffer} {}

        CategoricalColumn* column_;
        BufferRAMPrecision<std::uint32_t>* buffer_;
    };

    virtual std::uint32_t addOrGetID(std::string_view str);

    std::string header_;
    Unit unit_;
    std::optional<dvec2> range_;
    std::shared_ptr<Buffer<std::uint32_t>> buffer_;
    std::vector<std::string> lookUpTable_;
    std::map<std::string, std::uint32_t, std::less<>> lookupMap_;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename T>
TemplateColumn<T>::TemplateColumn(std::string_view header, std::shared_ptr<Buffer<T>> buffer,
                                  Unit unit, std::optional<dvec2> range)
    : header_{header}, unit_{unit}, range_{range}, buffer_{buffer} {}

template <typename T>
TemplateColumn<T>::TemplateColumn(std::string_view header, std::vector<T> data, Unit unit,
                                  std::optional<dvec2> range)
    : TemplateColumn(header, util::makeBuffer(std::move(data)), unit, range) {}

template <typename T>
TemplateColumn<T>::TemplateColumn(std::string_view header, size_t size, Unit unit,
                                  std::optional<dvec2> range)
    : TemplateColumn(header, std::make_shared<Buffer<T>>(size), unit, range) {}

template <typename T>
TemplateColumn<T>::TemplateColumn(const TemplateColumn& rhs)
    : header_(rhs.getHeader())
    , unit_(rhs.unit_)
    , range_(rhs.range_)
    , buffer_(std::shared_ptr<Buffer<T>>(rhs.buffer_->clone())) {}

template <typename T>
TemplateColumn<T>::TemplateColumn(TemplateColumn<T>&& rhs)
    : header_(std::move(rhs.header_))
    , unit_(rhs.unit_)
    , range_(rhs.range_)
    , buffer_(std::move(rhs.buffer_)) {}

template <typename T>
TemplateColumn<T>::TemplateColumn(const TemplateColumn& rhs,
                                  const std::vector<std::uint32_t>& rowSelection)
    : header_(rhs.getHeader())
    , unit_(rhs.unit_)
    , range_(rhs.range_)
    , buffer_(std::make_shared<Buffer<T>>(rowSelection.size())) {

    const auto& src = rhs.getTypedBuffer()->getRAMRepresentation()->getDataContainer();
    auto& dst = buffer_->getEditableRAMRepresentation()->getDataContainer();
    for (size_t i = 0; i < rowSelection.size(); ++i) {
        dst[i] = src[rowSelection[i]];
    }
}

template <typename T>
TemplateColumn<T>& TemplateColumn<T>::operator=(const TemplateColumn<T>& rhs) {
    if (this != &rhs) {
        header_ = rhs.getHeader();
        unit_ = rhs.unit_;
        range_ = rhs.range_;
        buffer_ = std::shared_ptr<Buffer<T>>(rhs.buffer_->clone());
    }
    return *this;
}

template <typename T>
TemplateColumn<T>& TemplateColumn<T>::operator=(TemplateColumn<T>&& rhs) {
    if (this != &rhs) {
        header_ = std::move(rhs.header_);
        unit_ = rhs.unit_;
        range_ = rhs.range_;
        buffer_ = std::move(rhs.buffer_);
    }
    return *this;
}

template <typename T>
TemplateColumn<T>* TemplateColumn<T>::clone() const {
    return new TemplateColumn(*this);
}

template <typename T>
TemplateColumn<T>* TemplateColumn<T>::clone(const std::vector<std::uint32_t>& rowSelection) const {
    return new TemplateColumn(*this, rowSelection);
}

template <typename T>
ColumnType TemplateColumn<T>::getColumnType() const {
    return ColumnType::Ordinal;
}

template <typename T>
const std::string& TemplateColumn<T>::getHeader() const {
    return header_;
}

template <typename T>
void TemplateColumn<T>::setHeader(std::string_view header) {
    header_ = header;
}

template <typename T>
Unit TemplateColumn<T>::getUnit() const {
    return unit_;
}

template <typename T>
void TemplateColumn<T>::setUnit(Unit unit) {
    unit_ = unit;
}

template <typename T>
void TemplateColumn<T>::setCustomRange(std::optional<dvec2> range) {
    range_ = range;
}

template <typename T>
std::optional<dvec2> TemplateColumn<T>::getCustomRange() const {
    return range_;
}

template <typename T>
dvec2 TemplateColumn<T>::getDataRange() const {
    const auto [min, max] = util::bufferMinMax(buffer_.get(), IgnoreSpecialValues::Yes);
    return {*std::min_element(glm::value_ptr(min), glm::value_ptr(min) + util::extent_v<T>),
            *std::max_element(glm::value_ptr(max), glm::value_ptr(max) + util::extent_v<T>)};
}

template <typename T>
dvec2 TemplateColumn<T>::getRange() const {
    if (range_) {
        return *range_;
    } else {
        return getDataRange();
    }
}

template <typename T>
void TemplateColumn<T>::add(const T& value) {
    buffer_->getEditableRAMRepresentation()->add(value);
}

namespace detail {

template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
void add(Buffer<T>* buffer, std::string_view value) {
    T result;
    if (value.empty()) {
        result = T{0};  // no special value indicating missing data for integral types
    } else {
        std::stringstream stream;
        stream << value;
        stream >> result;
        if (stream.fail()) {
            throw InvalidConversion(fmt::format("cannot convert \"{}\" to target type", value));
        }
    }
    buffer->getEditableRAMRepresentation()->add(result);
}
// Specialization for float and double types, add NaN instead of throwing an error
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
void add(Buffer<T>* buffer, std::string_view value) {
    T result;
    std::stringstream stream;
    stream << value;
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
void add(Buffer<T>* /*buffer*/, std::string_view value) {
    throw InvalidConversion(
        fmt::format("conversion to target type not implemented (\"{}\")", value));
}

}  // namespace detail

template <typename T>
void TemplateColumn<T>::add(std::string_view value) {
    detail::add<T>(buffer_.get(), value);
}

template <typename T>
void TemplateColumn<T>::append(const Column& col) {
    if (auto srccol = dynamic_cast<const TemplateColumn<T>*>(&col)) {
        buffer_->getEditableRAMRepresentation()->append(
            srccol->buffer_->getRAMRepresentation()->getDataContainer());
    } else {
        throw Exception("data formats of columns do not match", IVW_CONTEXT);
    }
}

template <typename T>
void TemplateColumn<T>::set(size_t idx, const T& value) {
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

inline auto CategoricalColumn::begin() const -> ConstIterator {
    return util::TransformIterator(detail::categoricalTransform(lookUpTable_),
                                   buffer_->getRAMRepresentation()->getDataContainer().begin());
}

inline auto CategoricalColumn::end() const -> ConstIterator {
    return util::TransformIterator(detail::categoricalTransform(lookUpTable_),
                                   buffer_->getRAMRepresentation()->getDataContainer().end());
}

inline auto CategoricalColumn::values() const -> util::iter_range<ConstIterator> {
    return util::as_range(begin(), end());
}

#endif

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::ColumnType> : inviwo::FlagFormatter<inviwo::ColumnType> {};
