/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/io/serialization/ticpp.h>

#include <cmath>
#include <streambuf>

namespace inviwo {

TransferFunction::TransferFunction(size_t textureSize)
    : TransferFunction({}, TFPrimitiveSetType::Relative, textureSize) {}

TransferFunction::TransferFunction(const std::vector<TFPrimitiveData>& values, size_t textureSize)
    : TransferFunction(values, TFPrimitiveSetType::Relative, textureSize) {}

TransferFunction::TransferFunction(const std::vector<TFPrimitiveData>& values,
                                   TFPrimitiveSetType type, size_t textureSize)
    : TFPrimitiveSet(values, type)
    , maskMin_(0.0)
    , maskMax_(1.0)
    , invalidData_(true)
    , dataRepr_{std::make_shared<LayerRAMPrecision<vec4>>(size2_t(textureSize, 1))}
    , data_(std::make_unique<Layer>(dataRepr_)) {
    clearMask();
}

TransferFunction::TransferFunction(const TransferFunction& rhs)
    : TFPrimitiveSet(rhs)
    , maskMin_(rhs.maskMin_)
    , maskMax_(rhs.maskMax_)
    , invalidData_(true)
    , dataRepr_(std::shared_ptr<LayerRAMPrecision<vec4>>(rhs.dataRepr_->clone()))
    , data_(std::make_unique<Layer>(dataRepr_)) {}

TransferFunction& TransferFunction::operator=(const TransferFunction& rhs) {
    if (this != &rhs) {
        if (dataRepr_->getDimensions() != rhs.dataRepr_->getDimensions()) {
            dataRepr_ = std::make_shared<LayerRAMPrecision<vec4>>(rhs.dataRepr_->getDimensions());
            data_ = std::make_unique<Layer>(dataRepr_);
        }
        maskMin_ = rhs.maskMin_;
        maskMax_ = rhs.maskMax_;
        invalidData_ = rhs.invalidData_;

        TFPrimitiveSet::operator=(rhs);
    }
    return *this;
}

TransferFunction::~TransferFunction() = default;

const Layer* TransferFunction::getData() const {
    if (invalidData_) calcTransferValues();
    return data_.get();
}

size_t TransferFunction::getTextureSize() const { return dataRepr_->getDimensions().x; }

void TransferFunction::setMaskMin(double maskMin) {
    maskMin_ = maskMin;
    invalidate();
}

double TransferFunction::getMaskMin() const { return maskMin_; }

void TransferFunction::setMaskMax(double maskMax) {
    maskMax_ = maskMax;
    invalidate();
}

double TransferFunction::getMaskMax() const { return maskMax_; }

void TransferFunction::clearMask() {
    maskMin_ =
        (getType() == TFPrimitiveSetType::Relative) ? 0.0 : std::numeric_limits<double>::lowest();
    maskMax_ =
        (getType() == TFPrimitiveSetType::Relative) ? 1.0 : std::numeric_limits<double>::max();
    invalidate();
}

void TransferFunction::invalidate() { invalidData_ = true; }

void TransferFunction::serialize(Serializer& s) const {
    s.serialize("maskMin", maskMin_);
    s.serialize("maskMax", maskMax_);
    TFPrimitiveSet::serialize(s);
}

void TransferFunction::deserialize(Deserializer& d) {
    d.deserialize("maskMin", maskMin_);
    d.deserialize("maskMax", maskMax_);

    TFPrimitiveSet::deserialize(d);
}

vec4 TransferFunction::sample(double v) const { return interpolateColor(v); }

vec4 TransferFunction::sample(float v) const { return interpolateColor(v); }

std::vector<FileExtension> TransferFunction::getSupportedExtensions() const {
    return {{"itf", "Inviwo Transfer Function"},
            {"png", "Transfer Function Image"},
            {"xml", "XML Transfer Function"}};
}

void TransferFunction::saveAsLayer(std::string_view filename, const FileExtension& ext) const {
    if (invalidData_) calcTransferValues();
    // Convert layer to UINT8
    auto uint8DataRepr =
        std::make_shared<LayerRAMPrecision<glm::u8vec4>>(dataRepr_->getDimensions());
    auto unit8Data = std::make_unique<Layer>(uint8DataRepr);

    const auto size = glm::compMul(dataRepr_->getDimensions());
    const auto sptr = dataRepr_->getDataTyped();
    const auto dptr = uint8DataRepr->getDataTyped();

    for (size_t i = 0; i < size; ++i) {
        dptr[i] = static_cast<glm::u8vec4>(glm::clamp(sptr[i] * 255.0f, vec4(0.0f), vec4(255.0f)));
    }

    auto factory = InviwoApplication::getPtr()->getDataWriterFactory();
    auto writer = factory->getWriterForTypeAndExtension<Layer>(ext);
    if (!writer) {
        writer = factory->getWriterForTypeAndExtension<Layer>(ext);
    }
    if (!writer) {
        throw DataWriterException("Data writer not found for requested format", IVW_CONTEXT);
    }
    writer->setOverwrite(Overwrite::Yes);
    writer->writeData(unit8Data.get(), filename);
}

void TransferFunction::loadFromLayer(std::string_view filename, const FileExtension& ext) {
    auto factory = InviwoApplication::getPtr()->getDataReaderFactory();
    auto reader = factory->getReaderForTypeAndExtension<Layer>(ext, filename);
    if (!reader) {
        throw DataReaderException("Data reader not found for requested format", IVW_CONTEXT);
    }
    const auto layer = reader->readData(filename);

    clear();

    layer->getRepresentation<LayerRAM>()->dispatch<void>([this](auto lrprecision) {
        auto data = lrprecision->getDataTyped();
        const auto size = lrprecision->getDimensions().x;

        const auto points = [&]() {
            std::vector<TFPrimitiveData> tmp;
            for (size_t i = 0; i < size; ++i) {
                tmp.push_back({static_cast<double>(i) / (size - 1),
                               util::glm_convert_normalized<vec4>(data[i])});
            }

            if (std::all_of(tmp.cbegin(), tmp.cend(),
                            [](const TFPrimitiveData& p) { return p.color.a == 0.0f; })) {
                std::for_each(tmp.begin(), tmp.end(),
                              [](TFPrimitiveData& p) { return p.color.a = 1.0f; });
            }
            return tmp;
        }();

        const auto simplified = simplify(points, 0.01);
        this->add(simplified);
    });
}

namespace detail {

void parseColorMapNode(TransferFunction& tf, TxElement* node) {
    if (node->Value() != "ColorMap") {
        throw DataReaderException(IVW_CONTEXT_CUSTOM("TransferFunction::loadFromXML()"),
                                  "XML node mismatch (got '{}', expected 'ColorMap')",
                                  node->Value());
    }

    const std::string colorspace = node->GetAttribute("space");
    const std::string colormapName = node->GetAttribute("name");

    auto convert = [&, cs = toLower(colorspace)]() -> std::function<vec3(dvec3)> {
        if ((cs == "lab") || (cs == "cielab")) {
            return [](dvec3 c) { return color::lab2rgb(vec3(c)); };
        } else if ((cs == "luv") || (cs == "cieluv")) {
            return [](dvec3 c) { return color::LuvChromaticity2rgb(vec3(c)); };
        } else if (cs == "hsv") {
            return [](dvec3 c) { return color::hsv2rgb(vec3(c)); };
        } else if (cs == "rgb") {
            return [](dvec3 c) { return vec3{c}; };
        } else {
            throw DataReaderException(IVW_CONTEXT_CUSTOM("TransferFunction::loadFromXML()"),
                                      "Unsupported colorspace '{}'", cs);
        }
    }();

    tf.clear();

    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); ++child) {
        auto elem = child.Get();
        auto value = elem->Value();
        if (value == "Point") {
            auto x = elem->GetAttribute<double>("x");
            auto opacity = elem->GetAttribute<double>("o");
            dvec3 color{elem->GetAttribute<double>("r"), elem->GetAttribute<double>("g"),
                        elem->GetAttribute<double>("b")};
            tf.add(x, vec4(vec3{color}, opacity));
        }
    }
}

void parseColorMapsNode(TransferFunction& tf, TxElement* node) {
    if (node->Value() != "ColorMaps") {
        throw DataReaderException(IVW_CONTEXT_CUSTOM("TransferFunction::loadFromXML()"),
                                  "XML node mismatch (got '{}', expected 'ColorMaps')",
                                  node->Value());
    }

    size_t count = 0;
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); ++child) {
        if (child.Get()->Value() == "ColorMap") {
            ++count;
            if (count > 1) {
                LogWarnCustom("TransferFunction::loadFromXML()", "Detected more than one ColorMap");
                return;
            }
            parseColorMapNode(tf, child.Get());
        }
    }
}

}  // namespace detail

void TransferFunction::saveAsXML(std::string_view filename) const {
    try {
        auto of = filesystem::ofstream(filename, std::ios::binary);
        if (!of) {
            throw DataReaderException(fmt::format("Could not write file '{}'", filename),
                                      IVW_CONTEXT);
        }

        TxElement colormap("ColorMap");
        colormap.SetAttribute("space", "rgb");
        colormap.SetAttribute("name", "Inviwo TransferFunction");

        for (auto p : *this) {
            TxElement point("Point");
            point.SetAttribute<double>("x", p.getPosition());
            point.SetAttribute<double>("o", p.getAlpha());
            const auto color = p.getColor();
            point.SetAttribute<double>("r", color.r);
            point.SetAttribute<double>("g", color.g);
            point.SetAttribute<double>("b", color.b);
            colormap.InsertEndChild(point);
        }

        TxDocument doc;
        doc.InsertEndChild(TxElement("ColorMaps"))->InsertEndChild(colormap);

        TiXmlPrinter printer;
        printer.SetIndent("    ");
        doc.Accept(&printer);

        of << printer.Str();
        of.close();
    } catch (TxException& e) {
        throw DataWriterException(e.what(), IVW_CONTEXT);
    }
}

void TransferFunction::loadFromXML(std::string_view filename) {
    auto in = filesystem::ifstream(filename, std::ios::binary);
    if (!in) {
        throw DataReaderException(fmt::format("Could not read file '{}'", filename), IVW_CONTEXT);
    }

    // determine file size
    in.seekg(0, std::ios::end);
    auto filesize = in.tellg();
    in.seekg(0, std::ios::beg);

    std::string_view xmlHeader{"<?xml version=\"1.0\" standalone=\"yes\"?>"};

    std::string str;
    str.reserve(xmlHeader.size() + filesize + 1);
    str.append(xmlHeader);
    str.append((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    // XML structure
    //  <?xml version="1.0" standalone="yes"?>
    //  <ColorMaps>
    //    <ColorMap space="Lab" indexedLookup="false" group="Interlinked" name="Green/Brown">
    //      <Point x="0" o="1" r="0.921" g="0.9213" b="0.847"/>
    //      <Point x="0" o="1" r="0.921" g="0.9215" b="0.847"/>
    //      <Section colorMapName="Green 1" startIndex="2" endIndex="3"
    //        startPos="0.6" endPos="0.7" startValue="0" endValue="1"
    //        flipped="true" startAlpha="1" endAlpha="1"/>
    //      <Section colorMapName="Brown 9" startIndex="0" endIndex="1"
    //        startPos="0" endPos="1" startValue="-0.075" endValue="1.31"
    //        flipped="false" startAlpha="1" endAlpha="1"/>
    //    </ColorMap>
    //  </ColorMaps>

    try {
        TxDocument doc;
        doc.Parse(str, true, TIXML_ENCODING_UTF8);

        auto root = doc.FirstChildElement();
        if (!root) {
            throw DataReaderException(fmt::format("No XML root node found in '{}'", filename),
                                      IVW_CONTEXT);
        }
        detail::parseColorMapsNode(*this, root);
    } catch (TxException& e) {
        throw DataReaderException(e.what(), IVW_CONTEXT);
    }
}

void TransferFunction::save(const std::string& filename, const FileExtension& ext) const {
    std::string extension = toLower(filesystem::getFileExtension(filename));

    if (ext.extension_ == "itf" || (ext.empty() && extension == "itf")) {
        Serializer serializer(filename);
        serialize(serializer);
        serializer.writeFile();
    } else if (ext.extension_ == "xml" || (ext.empty() && extension == "xml")) {
        saveAsXML(filename);
    } else {
        saveAsLayer(filename, ext);
    }
}

void TransferFunction::load(const std::string& filename, const FileExtension& ext) {
    if (ext.extension_ == "itf" ||
        (ext.empty() && iCaseCmp(filesystem::getFileExtension(filename), "itf"))) {
        Deserializer deserializer(filename);
        deserialize(deserializer);
    } else if (ext.extension_ == "xml" ||
               (ext.empty() && iCaseCmp(filesystem::getFileExtension(filename), "xml"))) {
        loadFromXML(filename);
    } else {
        loadFromLayer(filename, ext);
    }
}

std::vector<TFPrimitiveData> TransferFunction::simplify(const std::vector<TFPrimitiveData>& points,
                                                        double delta) {
    if (points.size() < 3) return points;
    std::vector<TFPrimitiveData> simple{points};

    // Calculate the error resulting from using a linear interpolation between the prev and next
    // point instead of including the current one
    const auto error = [&](size_t i) {
        const auto& prev = simple[i - 1];
        const auto& curr = simple[i];
        const auto& next = simple[i + 1];

        const double x = (curr.pos - prev.pos) / (next.pos - prev.pos);
        return glm::compMax(glm::abs(glm::mix(prev.color, next.color, x) - curr.color));
    };

    // Find the point which will result in the smallest error when removed.
    const auto nextToRemove = [&]() {
        const auto index = util::make_sequence<size_t>(1, simple.size() - 1, 1);
        return *std::min_element(index.begin(), index.end(),
                                 [&](size_t a, size_t b) { return error(a) < error(b); });
    };

    // Iteratively remove the point with the smallest error until the error gets larger then delta
    // or only 2 points are left
    auto toRemove = nextToRemove();
    while (error(toRemove) < delta && simple.size() > 2) {
        simple.erase(simple.begin() + toRemove);
        toRemove = nextToRemove();
    }

    return simple;
}

void TransferFunction::calcTransferValues() const {
    IVW_ASSERT(std::is_sorted(sorted_.begin(), sorted_.end(), comparePtr{}), "Should be sorted");

    // We assume the the points a sorted here.
    auto dataArray = dataRepr_->getDataTyped();
    const auto size = dataRepr_->getDimensions().x;

    interpolateAndStoreColors(dataArray, size);

    for (size_t i = 0; i < size_t(maskMin_ * size); i++) dataArray[i].a = 0.0;
    for (size_t i = size_t(maskMax_ * size); i < size; i++) dataArray[i].a = 0.0;

    data_->invalidateAllOther(dataRepr_.get());

    invalidData_ = false;
}

std::string_view TransferFunction::getTitle() const { return "Transfer Function"; }

std::string_view TransferFunction::serializationKey() const { return "Points"; }

std::string_view TransferFunction::serializationItemKey() const { return "Point"; }

bool operator==(const TransferFunction& lhs, const TransferFunction& rhs) {
    if (lhs.maskMin_ != rhs.maskMin_) return false;
    if (lhs.maskMax_ != rhs.maskMax_) return false;

    return static_cast<const TFPrimitiveSet&>(lhs) == static_cast<const TFPrimitiveSet&>(rhs);
}

bool operator!=(const TransferFunction& lhs, const TransferFunction& rhs) {
    return !operator==(lhs, rhs);
}

}  // namespace inviwo
