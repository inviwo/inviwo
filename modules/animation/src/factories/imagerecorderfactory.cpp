/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/animation/factories/imagerecorderfactory.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/factoryutil.h>

namespace inviwo::animation {

namespace {
class ImageRecorder : public Recorder {
public:
    ImageRecorder(InviwoApplication* app, const std::filesystem::path& dir, std::string_view format,
                  std::shared_ptr<DataWriterType<LayerRAM>> writer)
        : Recorder{}
        , app_{app}
        , dir_{dir}
        , format_{format}
        , writer_{std::move(writer)}
        , count_{0} {}

    virtual ~ImageRecorder() = default;
    virtual void record(const LayerRAM& layer) override;

private:
    InviwoApplication* app_;
    std::filesystem::path dir_;
    std::string format_;
    std::shared_ptr<DataWriterType<LayerRAM>> writer_;
    size_t count_;
};

void ImageRecorder::record(const LayerRAM& layer) {
    util::dispatchPool(app_,
                       [writer = writer_, file = dir_ / fmt::format(fmt::runtime(format_), count_),
                        copy = std::shared_ptr<LayerRAM>(layer.clone())]() {
                           writer->writeData(copy.get(), file);
                       });
    ++count_;
}
}  // namespace

ImageRecorderFactory::ImageRecorderFactory(InviwoApplication* app)
    : RecorderFactory{}
    , app_{app}
    , name_{"Image"}
    , options_{"image", "Image"}
    , outputDirectory_{"outputDirectory", "Output Directory"}
    , baseName_{"baseName", "Base Name",
                "The final name will be '[base name][zero padded number].[file extension]'."
                " For example: 'frame0001.png'"_help,
                "frame"}
    , writer_{"writer", "Type"} {

    options_.addProperties(outputDirectory_, baseName_, writer_);
}

const std::string& ImageRecorderFactory::getClassIdentifier() const { return name_; }
BoolCompositeProperty* ImageRecorderFactory::options() {

    if (writer_.size() == 0) {
        auto writerFactory = util::getDataWriterFactory(app_);
        const auto exts = writerFactory->getExtensionsForType<Layer>();
        std::vector<OptionPropertyOption<FileExtension>> opts;
        std::transform(exts.begin(), exts.end(), std::back_inserter(opts),
                       [](const auto& ext) -> OptionPropertyOption<FileExtension> { return ext; });

        writer_.replaceOptions(std::move(opts));
        writer_.setCurrentStateAsDefault();
    }
    return &options_;
}
std::unique_ptr<Recorder> ImageRecorderFactory::create(const RecorderOptions& opts) {

    auto writerFactory = util::getDataWriterFactory(app_);

    std::shared_ptr<DataWriterType<LayerRAM>> writer =
        writerFactory->getWriterForTypeAndExtension<LayerRAM>(writer_.getSelectedValue());

    // digits of the frame counter
    const int digits = [&]() {
        int d = 0;
        int number(opts.expectedNumberOfFrames - 1);
        while (number) {
            number /= 10;
            d++;
        }
        // use at least 4 digits, so we nicely overwrite the files from a previous test rendering
        // with less frames
        return std::max(d, 4);
    }();

    auto format = fmt::format("{}{{:{}}}", baseName_, digits);
    replaceInString(format, "UPN", opts.sourceName);

    return std::make_unique<ImageRecorder>(app_, outputDirectory_.get(), format, writer);
}

}  // namespace inviwo::animation
