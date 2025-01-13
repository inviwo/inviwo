/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <inviwo/core/io/transferfunctionxmlreader.h>
#include <inviwo/core/io/serialization/ticpp.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/colorconversion.h>

#include <inviwo/core/io/serialization/serializebase.h>

#include <fmt/std.h>

namespace inviwo {

namespace {

void parseColorMapNode(TransferFunction& tf, TxElement* node) {
    if (node->Value() != "ColorMap") {
        throw DataReaderException(IVW_CONTEXT_CUSTOM("TransferFunctionXMLReader::loadFromXML()"),
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
            throw DataReaderException(
                IVW_CONTEXT_CUSTOM("TransferFunctionXMLReader::loadFromXML()"),
                "Unsupported colorspace '{}'", cs);
        }
    }();

    const auto toDouble = [](std::optional<std::string_view> str) {
        double dest{};
        if (str) {
            detail::fromStr(*str, dest);
        }
        return dest;
    };

    for (TiXmlElement* child = node->FirstChildElement("Point"); child;
         child = child->NextSiblingElement("Point")) {

        const auto x = toDouble(child->Attribute("x"));
        const auto opacity = toDouble(child->Attribute("o"));
        const dvec3 color{toDouble(child->Attribute("r")), toDouble(child->Attribute("g")),
                          toDouble(child->Attribute("b"))};
        tf.add(x, vec4(vec3{color}, opacity));
    }
}

void parseColorMapsNode(TransferFunction& tf, TxElement* node) {
    if (node->Value() != "ColorMaps") {
        throw DataReaderException(IVW_CONTEXT_CUSTOM("TransferFunction::loadFromXML()"),
                                  "XML node mismatch (got '{}', expected 'ColorMaps')",
                                  node->Value());
    }

    size_t count = 0;

    for (TiXmlElement* child = node->FirstChildElement("ColorMap"); child;
         child = child->NextSiblingElement("ColorMap")) {
        ++count;
        if (count > 1) {
            log::warn("Detected more than one ColorMap");
            return;
        }
        parseColorMapNode(tf, child);
    }
}

}  // namespace

TransferFunctionXMLReader::TransferFunctionXMLReader() {
    addExtension({"xml", "XML Transfer Function"});
}

TransferFunctionXMLReader* TransferFunctionXMLReader::clone() const {
    return new TransferFunctionXMLReader{*this};
}

std::shared_ptr<TransferFunction> TransferFunctionXMLReader::readData(
    const std::filesystem::path& filePath) {
    auto in = open(filePath);

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

    auto data = std::make_shared<TransferFunction>();

    try {
        TxDocument doc;
        doc.Parse(str.c_str());

        auto root = doc.FirstChildElement();
        if (!root) {
            throw DataReaderException(IVW_CONTEXT, "No XML root node found in '{}'", filePath);
        }
        parseColorMapsNode(*data, root);
    } catch (const TiXmlError& e) {
        throw DataReaderException(e.what(), IVW_CONTEXT);
    }

    return data;
};

}  // namespace inviwo
