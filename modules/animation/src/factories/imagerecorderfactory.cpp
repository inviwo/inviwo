/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <fmt/format.h>

namespace inviwo::animation {

class ExceptionPropagator {
    class State {
    public:
        State() = default;
        State(const State&) = delete;
        State(State&&) = delete;
        State& operator=(const State&) = delete;
        State& operator=(State&&) = delete;
        ~State() noexcept {
            try {
                throwOnError();
            } catch (const Exception& e) {
                util::log(e.getContext(), e.getMessage());
            } catch (const std::exception& e) {
                util::log(IVW_CONTEXT, e.what());
            } catch (...) {
                util::log(IVW_CONTEXT, "unknown error");
            }
        }
        void setException() {
            std::scoped_lock lock{exceptionMutex_};
            if (!exception_) {
                exception_ = std::current_exception();
            }
        }
        void throwOnError() {
            std::scoped_lock lock{exceptionMutex_};
            if (exception_) {
                std::rethrow_exception(std::exchange(exception_, nullptr));
            }
        }

    private:
        std::mutex exceptionMutex_;
        std::exception_ptr exception_;
    };

public:
    ExceptionPropagator() : state_{std::make_shared<State>()} {}

    void setException() { state_->setException(); }
    void throwOnError() { state_->throwOnError(); }

    std::shared_ptr<State> get() { return state_; }

private:
    std::shared_ptr<State> state_;
};

namespace {
class ImageRecorder : public Recorder {
public:
    ImageRecorder(InviwoApplication* app, const std::filesystem::path& dir, std::string_view format,
                  std::shared_ptr<DataWriterType<Layer>> writer)
        : Recorder{}
        , app_{app}
        , dir_{dir}
        , format_{format}
        , writer_{std::move(writer)}
        , count_{1} {}

    virtual ~ImageRecorder() = default;
    virtual void record(const Layer& layer) override;

private:
    InviwoApplication* app_;
    std::filesystem::path dir_;
    std::string format_;
    std::shared_ptr<DataWriterType<Layer>> writer_;
    size_t count_;
    ExceptionPropagator exceptionProp_;
};

void ImageRecorder::record(const Layer& layer) {
    exceptionProp_.throwOnError();

    // Hackish: Make sure LayerRAM is the last valid rep, so
    // that it is the one that will be cloned. This also
    // forces the download to happen on the main thread
    // instead of in the background.
    layer.getRepresentation<LayerRAM>();
    util::dispatchPool(
        app_, [writer = writer_, file = dir_ / fmt::format(fmt::runtime(format_), count_),
               copy = std::shared_ptr<Layer>(layer.clone()), exception = exceptionProp_.get()]() {
            try {
                writer->writeData(copy.get(), file);
            } catch (...) {
                exception->setException();
            }
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
    , writer_{"writer", "Writer"}
    , overwrite_{"overwrite", "Overwrite", false} {

    options_.addProperties(outputDirectory_, baseName_, writer_, overwrite_);
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
    auto writer = writerFactory->getWriterForTypeAndExtension<Layer>(writer_.getSelectedValue());
    if (!writer) {
        throw Exception(IVW_CONTEXT, "No writer found for extension {}",
                        writer_.getSelectedValue());
    }

    writer->setOverwrite(overwrite_ ? Overwrite::Yes : Overwrite::No);

    const auto digits = std::max(fmt::formatted_size("{}", opts.expectedNumberOfFrames), size_t{4});

    auto format = fmt::format("{}{{:0{}}}.{}", baseName_.get(), digits,
                              writer_.getSelectedValue().extension_);
    replaceInString(format, "UPN", opts.sourceName);

    return std::make_unique<ImageRecorder>(app_, outputDirectory_.get(), format, std::move(writer));
}

}  // namespace inviwo::animation
