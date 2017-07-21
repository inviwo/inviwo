/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_DATAFRAME_H
#define IVW_DATAFRAME_H

#include <modules/plotting/plottingmoduledefine.h>
#include <modules/plotting/datastructures/datapoint.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <unordered_map>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/datainport.h>

namespace inviwo {
class DataPointBase;
class BufferBase;
class BufferRAM;

class IVW_MODULE_PLOTTING_API DataFrame {
public:
    class IVW_MODULE_PLOTTING_API Column {
    public:
        virtual ~Column() = default;
        const std::string &getHeader() const { return header_; }
        void setHeader(std::string header) { header_ = header; }

        virtual Column *clone() const = 0;

        virtual std::shared_ptr<BufferBase> getBuffer() = 0;
        virtual std::shared_ptr<const BufferBase> getBuffer() const = 0;

        virtual size_t getSize() const { return buffer_->getSize(); }

        virtual double getAsDouble(size_t idx) const = 0;
        virtual dvec2 getAsDVec2(size_t idx) const = 0;
        virtual dvec3 getAsDVec3(size_t idx) const = 0;
        virtual dvec4 getAsDVec4(size_t idx) const = 0;

        virtual std::string getAsString(size_t idx) const = 0;
        virtual std::shared_ptr<DataPointBase> get(size_t idx, bool getStringsAsStrings) const = 0;

    protected:
        Column(std::string header, std::shared_ptr<BufferBase> buffer)
            : header_(header), buffer_(buffer) {}

        std::shared_ptr<BufferBase> buffer_;

    private:
        std::string header_;
    };

    template <typename T>
    class TemplateColumn : public Column {
    public:
        TemplateColumn(std::string header) : Column(header, std::make_shared<Buffer<T>>()) {
            typedBuffer_ = std::static_pointer_cast<Buffer<T>>(buffer_);
        }

        TemplateColumn(const TemplateColumn &rhs)
            : Column(rhs.getHeader(), std::shared_ptr<Buffer<T>>(rhs.getTypedBuffer()->clone())) {}

        virtual TemplateColumn *clone() const override { return new TemplateColumn(*this); }

        virtual ~TemplateColumn() = default;

        virtual double getAsDouble(size_t idx) const override {
            auto val = typedBuffer_->getRAMRepresentation()->getDataContainer()[idx];
            return util::glm_convert<double>(val);
        }

        virtual dvec2 getAsDVec2(size_t idx) const override {
            auto val = typedBuffer_->getRAMRepresentation()->getDataContainer()[idx];
            return util::glm_convert<dvec2>(val);
        }

        virtual dvec3 getAsDVec3(size_t idx) const override {
            auto val = typedBuffer_->getRAMRepresentation()->getDataContainer()[idx];
            return util::glm_convert<dvec3>(val);
        }

        virtual dvec4 getAsDVec4(size_t idx) const override {
            auto val = typedBuffer_->getRAMRepresentation()->getDataContainer()[idx];
            return util::glm_convert<dvec4>(val);
        }

        void setBuffer(std::shared_ptr<Buffer<T>> buffer) { typedBuffer_ = buffer; }

        virtual std::string getAsString(size_t idx) const override {
            std::ostringstream ss;
            ss << typedBuffer_->getRAMRepresentation()->get(idx);
            return ss.str();
        }

        virtual std::shared_ptr<DataPointBase> get(size_t idx,
                                                   bool getStringsAsStrings) const override {
            return std::make_shared<DataPoint<T>>(typedBuffer_->getRAMRepresentation()->get(idx));
        }

        virtual std::shared_ptr<BufferBase> getBuffer() override { return typedBuffer_; }
        virtual std::shared_ptr<const BufferBase> getBuffer() const override {
            return typedBuffer_;
        }

        std::shared_ptr<Buffer<T>> getTypedBuffer() { return typedBuffer_; }
        std::shared_ptr<const Buffer<T>> getTypedBuffer() const { return typedBuffer_; }

        std::shared_ptr<Buffer<T>> typedBuffer_;
    };

    class IVW_MODULE_PLOTTING_API CategoricalColumn : public TemplateColumn<std::uint32_t> {
    public:
        CategoricalColumn(std::string header) : TemplateColumn<std::uint32_t>(header) {}

        CategoricalColumn(const CategoricalColumn &rhs) : TemplateColumn<std::uint32_t>(rhs) {}

        virtual CategoricalColumn *clone() const { return new CategoricalColumn(*this); }

        virtual ~CategoricalColumn() = default;

        virtual std::string getAsString(size_t idx) const override {
            auto asdf = typedBuffer_->getRAMRepresentation()->getDataContainer()[idx];
            return lookUpTable_[asdf];
        }

        virtual std::shared_ptr<DataPointBase> get(size_t idx, bool getStringsAsStrings) const {
            if (getStringsAsStrings) {
                return std::make_shared<DataPoint<std::string>>(getAsString(idx));
            } else {
                return TemplateColumn<std::uint32_t>::get(idx, getStringsAsStrings);
            }
        }

        virtual void set(size_t idx, const std::string &str) {
            auto id = addOrGetID(str);
            getTypedBuffer()->getEditableRAMRepresentation()->set(idx, id);
        }

        virtual void add(const std::string &str) {
            auto id = addOrGetID(str);
            getTypedBuffer()->getEditableRAMRepresentation()->add(id);
        }

    private:
        virtual glm::uint32_t addOrGetID(const std::string &str) {
            auto it = std::find(lookUpTable_.begin(), lookUpTable_.end(), str);
            if (it != lookUpTable_.end()) {
                return static_cast<glm::uint32_t>(std::distance(lookUpTable_.begin(), it));
            }
            lookUpTable_.push_back(str);
            return static_cast<glm::uint32_t>(lookUpTable_.size() - 1);
        }

        std::vector<std::string> lookUpTable_;
    };

    using DataItem = std::vector<std::shared_ptr<DataPointBase>>;
    using LookupTable = std::unordered_map<glm::u64, std::string>;

    DataFrame(const DataFrame &df) {
        for (const auto &col : df.columns_) {
            columns_.emplace_back(col->clone());
        }
    }

    DataFrame(glm::u32 size = 0);
    virtual ~DataFrame() = default;

    std::shared_ptr<Column> addColumnFromBuffer(const std::string &identifier,
                                                std::shared_ptr<const BufferBase> buffer);

    template <typename T>
    std::shared_ptr<TemplateColumn<T>> addColumn(const std::string &header, size_t size = 0);

    std::shared_ptr<CategoricalColumn> addCategoricalColumn(const std::string &header,
                                                            size_t size = 0);

    DataItem getDataItem(size_t index, bool getStringsAsStrings = false) const;

    const std::vector<std::pair<std::string, const DataFormatBase *>> getHeaders() const {
        std::vector<std::pair<std::string, const DataFormatBase *>> headers;
        for (const auto &c : columns_) {
            headers.emplace_back(c->getHeader(), c->getBuffer()->getDataFormat());
        }
        return headers;
    }
    std::string getHeader(size_t idx) const { return columns_[idx]->getHeader(); }

    std::shared_ptr<Column> getColumn(size_t index) { return columns_[index]; }
    std::shared_ptr<const Column> getColumn(size_t index) const { return columns_[index]; }

    std::shared_ptr<TemplateColumn<glm::u32>> getIndexColumn() {
        return std::dynamic_pointer_cast<TemplateColumn<glm::u32>>(columns_[0]);
    }
    std::shared_ptr<const TemplateColumn<glm::u32>> getIndexColumn() const {
        return std::dynamic_pointer_cast<const TemplateColumn<glm::u32>>(columns_[0]);
    }

    size_t getSize() const;
    size_t getNumberOfColumns() const;
    size_t getNumberOfRows() const;

    std::vector<std::shared_ptr<Column>>::iterator begin() { return columns_.begin(); }
    std::vector<std::shared_ptr<Column>>::iterator end() { return columns_.end(); }
    std::vector<std::shared_ptr<Column>>::const_iterator begin() const { return columns_.begin(); }
    std::vector<std::shared_ptr<Column>>::const_iterator end() const { return columns_.end(); }

    void updateIndexBuffer();

private:
    std::vector<std::shared_ptr<Column>> columns_;
};

using DataFrameOutport = DataOutport<DataFrame>;
using DataFrameInport = DataInport<DataFrame>;

template <typename T>
std::shared_ptr<DataFrame::TemplateColumn<T>> DataFrame::addColumn(const std::string &header,
                                                                   size_t size /*= 0*/) {
    auto col = std::make_shared<TemplateColumn<T>>(header);
    col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer().resize(size);
    columns_.push_back(col);
    return col;
}

template <>
struct port_traits<DataFrame> {
    static std::string class_identifier() { return "DataFrame"; }
    static uvec3 color_code() { return uvec3(153, 76, 0); }
    static std::string data_info(const DataFrame *data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "DataFrame", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("Number of columns: "), data->getNumberOfColumns());

        for (size_t i = 0; i < data->getNumberOfColumns(); i++) {
            std::ostringstream oss;
            oss << "Column " << (i + 1) << ": " << data->getHeader(i);
            tb(H(oss.str()), "");

            tb("size", data->getColumn(i)->getBuffer()->getSize());
            tb("Dataformat", data->getColumn(i)->getBuffer()->getDataFormat()->getString());
        }

        return doc;
    }
};

}  // namespace inviwo

#endif  // IVW_DATAFRAME_H
