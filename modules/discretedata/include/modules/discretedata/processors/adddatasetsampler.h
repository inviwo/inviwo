/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/channels/formatconversionchannel.h>
#include <inviwo/core/ports/meshport.h>

namespace inviwo {
namespace discretedata {
/** \docpage{org.inviwo.AddDataSetSampler, Add DataSet Sampler}
 * ![](org.inviwo.AddDataSetSampler.png?classIdentifier=org.inviwo.AddDataSetSampler)
 * Add a DataSetSampler to a given DataSet.
 * DataSetSamplers perform cell location and interpolation based on a selected position channel.
 */
class IVW_MODULE_DISCRETEDATA_API AddDataSetSampler : public Processor {
public:
    typedef std::function<std::shared_ptr<DataSetSamplerBase>(
        ind, std::shared_ptr<const Connectivity>, std::shared_ptr<const Channel>,
        const InterpolantBase*)>
        CreateSampler;

    typedef std::function<const InterpolantBase*(ind)> CreateInterpolant;

    AddDataSetSampler();
    virtual ~AddDataSetSampler() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    template <template <unsigned int> class SamplerType>
    static void addSamplerType(std::string identifier, std::string displayName);
    static void addSamplerType(std::string identifier, std::string displayName,
                               CreateSampler creator);

    template <template <unsigned int> class InterpolantType>
    static void addInterpolantType(std::string identifier, std::string displayName);
    static void addInterpolantType(std::string identifier, std::string displayName,
                                   CreateInterpolant creator);

private:
    void fillInterpolationTypes();

    DataSetInport dataIn_;
    DataSetOutport dataOut_;
    MeshOutport meshOut_;
    DataChannelProperty positionChannel_;
    // TemplateOptionProperty<CreateInterpolant> interpolantCreator_;
    OptionPropertyUInt samplerCreator_, interpolantCreator_;
    // TemplateOptionProperty<InterpolationType> interpolationType_;

    const InterpolantBase* interpolant_;
    std::shared_ptr<DataSetSamplerBase> sampler_;

    bool interpolantChanged_, interpolationChanged_, samplerChanged_;

    static std::map<std::string, std::pair<std::string, CreateSampler>> samplerCreatorList_;
    static std::map<std::string, std::pair<std::string, CreateInterpolant>> interpolantCreatorList_;

    template <template <unsigned int> class SamplerType>
    struct SamplerDispatcher {
        template <typename Result, ind N>
        Result operator()(std::shared_ptr<const Connectivity> grid,
                          std::shared_ptr<const Channel> coordinates,
                          const InterpolantBase* interpolant) {

            auto* usableInterpolant = dynamic_cast<const Interpolant<N>*>(interpolant);
            auto usableCoords =
                std::dynamic_pointer_cast<const DataChannel<double, N>>(coordinates);
            if (!usableCoords) {
                dd_detail::CreateFormatConversionChannelToTN<double, N> dispatcher;
                usableCoords = dispatching::dispatch<std::shared_ptr<DataChannel<double, N>>,
                                                     dispatching::filter::Scalars>(
                    coordinates->getDataFormatId(), dispatcher, coordinates,
                    fmt::format("{}_as_Coordinate", coordinates->getName()));
            }
            if (!usableInterpolant || !usableCoords) {
                // std::cout << "Interpolant or coordinates of wrong type!" << std::endl;
                // if (!usableInterpolant) std::cout << "= It's the interpolant!" << std::endl;
                // if (!usableCoords) std::cout << "= It's the coords!" << std::endl;
                return nullptr;
            }
            return std::make_shared<SamplerType<N>>(grid, usableCoords, *usableInterpolant);
        }
    };

    template <template <unsigned int> class InterpolantType>
    struct InterpolantDispatcher {
        template <typename Result, ind N>
        Result operator()() {
            return new InterpolantType<N>();
        }
    };
};

template <template <unsigned int> class SamplerType>
void AddDataSetSampler::addSamplerType(std::string identifier, std::string displayName) {
    AddDataSetSampler::addSamplerType(
        identifier, displayName,
        [](ind baseDim, std::shared_ptr<const Connectivity> grid,
           std::shared_ptr<const Channel> coordinates, const InterpolantBase* interpolant) {
            SamplerDispatcher<SamplerType> dispatcher;
            return channeldispatching::dispatchNumber<std::shared_ptr<DataSetSamplerBase>, 1, 3>(
                baseDim, dispatcher, grid, coordinates, interpolant);
        });
    std::cout << "Added interpolant option " << displayName << std::endl;
}

template <template <unsigned int> class InterpolantType>
void AddDataSetSampler::addInterpolantType(std::string identifier, std::string displayName) {
    AddDataSetSampler::addInterpolantType(identifier, displayName, [](ind baseDim) {
        InterpolantDispatcher<InterpolantType> dispatcher;
        std::cout << "Creating Interpolant!" << std::endl;
        return channeldispatching::dispatchNumber<const InterpolantBase*, 1,
                                                  DISCRETEDATA_MAX_NUM_DIMENSIONS>(baseDim,
                                                                                   dispatcher);
    });
}

}  // namespace discretedata
}  // namespace inviwo
