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

#pragma once

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/channels/analyticchannel.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.SphericalCoordinates, Spherical Coordinates}
 * ![](org.inviwo.SphericalCoordinates.png?classIdentifier=org.inviwo.SphericalCoordinates)
 * Take in longitude/latitude coordinates and transform them to a sphere.
 */
class IVW_MODULE_DISCRETEDATA_API SphericalCoordinates : public Processor {
public:
    SphericalCoordinates();
    virtual ~SphericalCoordinates() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataSetInport dataIn_;
    DataSetInport dataOut_;
    DataChannelProperty positions_;
    StringProperty name_;
    BoolProperty autoName_;
    DoubleProperty radius_, verticalScale_;
};

namespace detail {
struct SphericalCoordinateDispatcher {

    template <typename Result, typename T, ind N, typename... Args>
    Result operator()(std::shared_ptr<const Channel> channel, const std::string& name,
                      double radius, double verticalScale) {
        auto dataChannel =
            std::dynamic_pointer_cast<const DataChannel<typename T::type, N>>(channel);
        ivwAssert(dataChannel, "Dispatch failed, dynamic cast to specific type failed.");
        return std::make_shared<AnalyticChannel<double, std::min(ind(3), N)>>(
            [dataChannel, radius, verticalScale](auto spVec, ind idx) {
                std::array<typename T::type, N> cVec;
                dataChannel->fill(cVec, idx);
                double r = radius;
                if constexpr (N >= 3) {
                    r += verticalScale * cVec[2];
                }
                cVec[0] = glm::radians(static_cast<double>(cVec[0]));
                cVec[1] = glm::radians(static_cast<double>(cVec[1]));

                // // Wiki:
                // spVec[0] = r * std::sin(cVec[1]) * std::cos(cVec[0]);
                // spVec[1] = r * std::sin(cVec[1]) * std::sin(cVec[0]);
                // spVec[2] = r * std::cos(cVec[1]);

                // Pen & Paper:
                spVec[0] = +r * std::sin(cVec[0]) * std::cos(cVec[1]);
                spVec[1] = -r * std::cos(cVec[0]) * std::cos(cVec[1]);
                spVec[2] = +r * std::sin(cVec[1]);

                for (ind n = 3; n < N; ++n) {
                    spVec[n] = cVec[n];
                }
            },
            channel->size(), name, channel->getGridPrimitiveType());
    }
};
}  // namespace detail

}  // namespace discretedata
}  // namespace inviwo
