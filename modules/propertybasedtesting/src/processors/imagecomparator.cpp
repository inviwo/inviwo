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

#include <inviwo/propertybasedtesting/html/html.h>

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
	, maxPixelwiseDeviation_("maxPixelwiseDeviation", "Maximum pixelwise deviation (%)", 0, 0, 1)
    , comparisonType_("comparisonType", "Comparison Type (dummy)",
                     {{"diff", "Absolute ARGB differences", ComparisonType::AbsARGB}},
                     0, InvalidationLevel::InvalidOutput)
	, reductionType_("reductionType", "Reduction",
					{{"mean", "Mean", ReductionType::MEAN},
					 {"max", "Maximum", ReductionType::MAX},
					 {"min", "Minimum", ReductionType::MIN},
					 {"sum", "Sum", ReductionType::SUM}},
					 0, InvalidationLevel::InvalidOutput)
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
  addProperty(maxPixelwiseDeviation_);
  addProperty(comparisonType_);
  addProperty(reductionType_);

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

double ImageComparator::difference(const ComparisonType& comp,
			const glm::dvec4& col1, const glm::dvec4& col2) {
	switch(comp) {
		case ComparisonType::AbsARGB:
			return absoluteARGBdifference(col1, col2);
	}
}
double ImageComparator::absoluteARGBdifference(const glm::dvec4& col1, const glm::dvec4& col2) {
	double res = 0;
	for(size_t i = 0; i < DataFormat<decltype(col1)>::components(); i++)
		res += abs(col1[i] - col2[i]);
	return res;
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
  const size_t numPixels = dim1.x * dim1.y;

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

  const ReductionType reduction = reductionType_.get();

  double result = getUnitForReduction<double>(reduction);
  size_t diffPixels = 0;
  for(size_t x = 0; x < dim1.x; x++) {
    for(size_t y = 0; y < dim1.y; y++) {
      const auto col1 = colorLayerRAM1->getAsDVec4(size2_t(x,y));
      const auto col2 = colorLayerRAM2->getAsDVec4(size2_t(x,y));
      const double diff = difference(comparisonType_.get(), col1, col2);
	  const bool pixelDifferent =
		  (diff / std::max({1.0, glm::length(col1), glm::length(col2)}))
			  <= maxPixelwiseDeviation_.get();
	  const double c = pixelDifferent * 255.0;

	  result = combine(reduction, result, diff);
      colorLayerDiff->setFromDVec3(size2_t(x, y), 128.0 + (col1 - col2) / 2.0);
      colorLayerMask->setFromDVec3(size2_t(x, y), dvec3(c, c, c));
      if(pixelDifferent) {
        diffPixels++;
      }
    }
  }
  if(reduction == ReductionType::MEAN) {
	result /= numPixels;
  }

  differencePort_.setData(diffImg);
  maskPort_.setData(maskImg);

  if(result > maxDeviation_.get()) {
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
        , result
		, reduction
        , diffPixels
		, numPixels
        , img1Path
        , img2Path
        , diffPath
        , maskPath});
    }
  }
  createReport();
}

const std::string& reportCssFile =
R""""(
.comparison { background: #f2f2f2; padding: 1em; margin-bottom: 3em; width: 80% }
.data td:nth-child(1) { width: 10em; }"
.data tr:nth-child(even) { background: #e2e2e2; }"
.data { border-collapse: collapse; width: 100%; }"
.data td { border-top: 1px solid black; border-bottom: 1px solid black;}"
.images { width: 100%; padding-top: 1em;}"
img { width: 100%; border: 1px solid black;}"
th { font-weight: normal; width: 25%}"
)"""";

void ImageComparator::createReport() {
	HTML::Body body;
	
	for(const auto& comp : comparisons_) {
		char timeBuffer[sizeof "2012-12-24 12:34:56"];
		strftime(timeBuffer, sizeof timeBuffer, "%F %T", localtime(&comp.timestamp));

		body << (HTML::Div("comparison")
				<< (HTML::Table().addAttribute("class", "data")
					<< (HTML::Row()
						<< HTML::Text("Date")
						<< HTML::Text(timeBuffer))
					<< (HTML::Row()
						<< HTML::Text("Difference result (" + reductionTypeName(comp.reduction) + ")")
						<< HTML::Text(std::to_string(comp.result)))
					<< (HTML::Row()
						<< HTML::Text("Pixel count")
						<< HTML::Text(std::to_string(comp.pixelCount)))
					<< (HTML::Row()
						<< HTML::Text("Different pixels")
						<< HTML::Text(std::to_string(comp.differentPixels)))
					<< (HTML::Row()
						<< HTML::Text("Percent difference")
						<< HTML::Text(std::to_string(100 * comp.differentPixels / comp.pixelCount))))
				<< (HTML::Table().addAttribute("class", "images")
					<< (HTML::HeadRow()
						<< HTML::Text("diff")
						<< HTML::Text("mask")
						<< HTML::Text("img1")
						<< HTML::Text("img2"))
					<< (HTML::Row()
						<< HTML::Image(comp.diff).addAttribute("title", comp.diff.string())
						<< HTML::Image(comp.mask).addAttribute("title", comp.mask.string())
						<< HTML::Image(comp.img1).addAttribute("title", comp.img1.string())
						<< HTML::Image(comp.img2).addAttribute("title", comp.img2.string()))));

	}

  std::ofstream report;
  report.open(reportDir_.get() + "/report.html");
  report << (HTML::HTML()
			<< (HTML::Head() << HTML::Style(reportCssFile)
				<< HTML::Meta().addAttribute("charset","utf-8"))
			<< body);
  report.close();
}

}  // namespace inviwo
