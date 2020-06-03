/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/processors/imagecomparator.h>

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/imageramutils.h>
#include <filesystem>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageComparator::processorInfo_{
    "org.inviwo.ImageComparator",      // Class identifier
    "Image Comparator",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo ImageComparator::getProcessorInfo() const { return processorInfo_; }

ImageComparator::ImageComparator()
    : Processor()
    , inport1_("inport1")
    , inport2_("inport2")
    , differencePort_("difference")
    , maskPort_("mask")
    , maxDeviation_("maxDeviation", "Maximum deviation", 0, 0, std::numeric_limits<float>::max(), 1, InvalidationLevel::Valid, PropertySemantics::Text)
    , comparisonType_("comparisonType", "Comparison Type (dummy)",
                     {{"diff", "Sum of ARGB differences", ComparisonType::Diff},
                      {"perceptual", "Perceptual Difference", ComparisonType::Perceptual},
                      {"global", "Global Difference", ComparisonType::Global},
                      {"local", "Local Difference", ComparisonType::Local}},
                     0, InvalidationLevel::InvalidResources)
    , reportDir_("reportDir",
                 "Report Directory",
                 "")
    {

  addPort(inport1_);
  addPort(inport2_);
  addPort(differencePort_);
  addPort(maskPort_);
  addProperty(reportDir_);
  addProperty(maxDeviation_);
  addProperty(comparisonType_);

  isReady_.setUpdate([&]() {
      if(!allInportsAreReady())
        return false;
      auto img1 = inport1_.getData();
      auto img2 = inport2_.getData();

      const auto dim1 = img1->getDimensions();
      const auto dim2 = img2->getDimensions();
      if(dim1 != dim2) {
        std::stringstream str;
        str << getIdentifier() << ": Images do not have same dimensions: " << dim1 << " != " << dim2;
        util::log(IVW_CONTEXT, str.str(), LogLevel::Info, LogAudience::User);
        return false;
      }
      return true;
    });
}

void ImageComparator::setNetwork(ProcessorNetwork* network) {
    if (network) network->addObserver(this);

    Processor::setNetwork(network);
}

void ImageComparator::process() {
  if(reportDir_.get() != "") {
    if (! std::filesystem::exists(reportDir_.get())) {
      if (std::filesystem::create_directories(reportDir_.get())) {
        std::stringstream str;
        str << getIdentifier() << ": Using: " << reportDir_ << " to store images.";
        util::log(IVW_CONTEXT, str.str(), LogLevel::Info, LogAudience::User);
      }
    }
  }

  const auto img1 = inport1_.getData();
  const auto img2 = inport2_.getData();

  const auto dim1 = img1->getDimensions();
  const auto dim2 = img2->getDimensions();

  if(dim1 != dim2) {
    std::stringstream str;
    str << getIdentifier() << ": Error: Image dimensions do not agree: " << dim1 << " != " << dim2;
    util::log(IVW_CONTEXT, str.str(), LogLevel::Info, LogAudience::User);
    return;
  }

  auto diffImg = std::make_shared<Image>(dim1, DataVec3UInt8::get());
  auto maskImg = std::make_shared<Image>(dim1, DataVec3UInt8::get());

  auto imgRAM1 = img1->getRepresentation<ImageRAM>();
  auto imgRAM2 = img2->getRepresentation<ImageRAM>();
  auto diffRAM = diffImg->getEditableRepresentation<ImageRAM>();
  auto maskRAM = maskImg->getEditableRepresentation<ImageRAM>();

  auto colorLayerRAM1 = imgRAM1->getColorLayerRAM();
  auto colorLayerRAM2 = imgRAM2->getColorLayerRAM();
  auto colorLayerDiff = diffRAM->getColorLayerRAM();
  auto colorLayerMask = maskRAM->getColorLayerRAM();

  double diffSum = 0;
  double diffPixels = 0;
  for(size_t x = 0; x < dim1.x; x++) {
    for(size_t y = 0; y < dim1.y; y++) {
      const auto col1 = colorLayerRAM1->getAsDVec3(size2_t(x,y));
      const auto col2 = colorLayerRAM2->getAsDVec3(size2_t(x,y));
      const double diff = glm::length(col1 - col2);
      diffSum += diff;
      colorLayerDiff->setFromDVec3(size2_t(x, y), 128.0 + (col1 - col2) / 2.0);
      double c = col1 == col2 ? 255.0 : 0.0;
      colorLayerMask->setFromDVec3(size2_t(x, y), dvec3(c, c, c));
      if(col1 != col2) {
        diffPixels++;
      }
    }
  }
  differencePort_.setData(diffImg);
  maskPort_.setData(maskImg);

  if(diffSum > maxDeviation_.get()) {
    if(reportDir_.get() != "" && std::filesystem::exists(reportDir_.get())) {

      imageCompCount_++;
      const auto dir = std::filesystem::path(reportDir_.get());
      const auto suffix = std::to_string(imageCompCount_) + std::string(".png");
      const auto img1Path = dir / (std::string("img1_") + suffix);
      const auto img2Path = dir / (std::string("img2_") + suffix);
      const auto diffPath = dir / (std::string("diff_") + suffix);
      const auto maskPath = dir / (std::string("mask_") + suffix);
      static const auto pngExt = inviwo::FileExtension::createFileExtensionFromString(std::string("png"));
      inviwo::util::saveLayer(*img1->getColorLayer(), img1Path.string(), pngExt);
      inviwo::util::saveLayer(*img2->getColorLayer(), img2Path.string(), pngExt);
      inviwo::util::saveLayer(*diffImg->getColorLayer(), diffPath.string(), pngExt);
      inviwo::util::saveLayer(*maskImg->getColorLayer(), maskPath.string(), pngExt);

      comparisons_.push_back(
        { time(0)
        , diffSum
        , diffPixels
        , (double) dim1.x * (double) dim1.y
        , img1Path
        , img2Path
        , diffPath
        , maskPath});
    }
  }
  createReport();
}

void ImageComparator::createReport() {
  std::ofstream report;
  report.open(reportDir_.get() + "/report.html");
  report << "<html>" << std::endl;
  report << "<head>" << std::endl;
  report << "<meta charset=\"utf8\"/>" << std::endl;
  report << "<style>" << std::endl;
    report << ".comparison { background: #f2f2f2; padding: 1em; margin-bottom: 3em; width: 80% }" << std::endl;
    report << ".data td:nth-child(1) { width: 10em; }" << std::endl;
    report << ".data tr:nth-child(even) { background: #e2e2e2; }" << std::endl;
    report << ".data { border-collapse: collapse; width: 100%; }" << std::endl;
    report << ".data td { border-top: 1px solid black; border-bottom: 1px solid black;}" << std::endl;
    report << ".images { width: 100%; padding-top: 1em;}" << std::endl;
    report << "img { width: 100%; border: 1px solid black;}" << std::endl;
    report << "th { font-weight: normal; width: 25%}" << std::endl;
  report << "</style>" << std::endl;
  report << "</head>" << std::endl;
  report << "<body>" << std::endl;
  for(auto &comp : comparisons_) {
    report << "<div class=\"comparison\">" << std::endl;
      report << "<table class=\"data\">" << std::endl;
        report << "<tr>" << std::endl;
          report << "<td>Date</td>" << std::endl;
          char timeBuffer[sizeof "2012-12-24 12:34:56"];
          strftime(timeBuffer, sizeof timeBuffer, "%F %T", localtime(&comp.timestamp));
          report << "<td>" << timeBuffer << "</td>" << std::endl;
        report << "</tr>" << std::endl;
        report << "<tr>" << std::endl;
          report << "<td>Difference sum</td>" << std::endl;
          report << "<td>" << comp.diffSum << "</td>" << std::endl;
        report << "</tr>" << std::endl;
        report << "<tr>" << std::endl;
          report << "<td>Pixel count</td>" << std::endl;
          report << "<td>" << comp.pixelCount << "</td>" << std::endl;
        report << "</tr>" << std::endl;
        report << "<tr>" << std::endl;
          report << "<td>Different pixels</td>" << std::endl;
          report << "<td>" << comp.differentPixels << "</td>" << std::endl;
        report << "</tr>" << std::endl;
        report << "<tr>" << std::endl;
          report << "<td>Percent difference</td>" << std::endl;
          report << "<td>" << (100 * comp.differentPixels / comp.pixelCount) << "</td>" << std::endl;
        report << "</tr>" << std::endl;
      report << "</table>" << std::endl;
      report << "<table class=\"images\">" << std::endl;
        report << "<tr>" << std::endl;
          report << "<th>diff</th>" << std::endl;
          report << "<th>mask</th>" << std::endl;
          report << "<th>img1</th>" << std::endl;
          report << "<th>img2</th>" << std::endl;
        report << "</tr>" << std::endl;
        report << "<tr>" << std::endl;
          report << "<td><img src=\"" << comp.diff << "\" title=\"" << comp.diff << "\"/></td>" << std::endl;
          report << "<td><img src=\"" << comp.mask << "\" title=\"" << comp.mask << "\"/></td>" << std::endl;
          report << "<td><img src=\"" << comp.img1 << "\" title=\"" << comp.img1 << "\"/></td>" << std::endl;
          report << "<td><img src=\"" << comp.img2 << "\" title=\"" << comp.img2 << "\"/></td>" << std::endl;
        report << "</tr>" << std::endl;
      report << "</table>" << std::endl;
    report << "</div>" << std::endl;
  }
  report << "</body>" << std::endl;
  report << "</html>" << std::endl;
  report.close();
}

}  // namespace inviwo
