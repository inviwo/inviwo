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
#include <inviwo/core/properties/listproperty.h>
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
    DataSetOutport dataOut_;
    DataChannelProperty positions_, velocities_;
    StringProperty name_;
    BoolProperty autoName_;
    DoubleProperty radius_, verticalScale_;

    // ListProperty velocities_;
};

namespace detail {
template <class T, std::size_t N>
std::ostream& operator<<(std::ostream& o, const std::array<T, N>& arr) {
    std::copy(arr.cbegin(), arr.cend(), std::ostream_iterator<T>(o, " "));
    return o;
}

struct SphericalCoordinateDispatcher {

    template <typename Result, typename T, ind N, typename... Args>
    Result operator()(std::shared_ptr<const Channel> channel, const std::string& name,
                      double radius, double verticalScale,
                      std::shared_ptr<const DataChannel<float, 2>>& velocityChannel) {
        std::array<std::shared_ptr<const Channel>, 2> results{nullptr, nullptr};

        auto dataChannel =
            std::dynamic_pointer_cast<const DataChannel<typename T::type, N>>(channel);
        ivwAssert(dataChannel, "Dispatch failed, dynamic cast to specific type failed.");
        results[0] = std::make_shared<AnalyticChannel<double, std::max(ind(3), N)>>(
            [dataChannel, radius, verticalScale](auto& spVec, ind idx) {
                std::array<typename T::type, N> cVec;
                dataChannel->fill(cVec, idx);

                double r = radius;
                if constexpr (N >= 3) {
                    r += verticalScale * cVec[2];
                }
                cVec[0] = glm::radians(static_cast<double>(cVec[0]));
                cVec[1] = glm::radians(static_cast<double>(cVec[1]));

                // Pen & Paper:
                spVec[0] = +r * std::sin(cVec[1]) * std::cos(cVec[0]);
                spVec[1] = -r * std::cos(cVec[1]) * std::cos(cVec[0]);
                spVec[2] = +r * std::sin(cVec[0]);

                for (ind n = 3; n < N; ++n) {
                    spVec[n] = cVec[n];
                }
            },
            channel->size(), name, channel->getGridPrimitiveType());

        // Convert velocity channel from m/s to latlon/s (if any is given).
        if (velocityChannel) {
            results[1] = std::make_shared<AnalyticChannel<double, 2, std::array<double, 2>>>(
                [velocityChannel, dataChannel](std::array<double, 2>& velocityDegrees, ind idx) {
                    static const double AVG_EARTH_RADIUS = 6371000.0f;
                    static const double DEGREE_TO_RAD = M_PI / 180.0;
                    // static const double DEGREE_TO_METER_MULT = 180.0 / (M_PI * AVG_EARTH_RADIUS);
                    std::array<typename T::type, N> latLon;
                    dataChannel->fill(latLon, idx);

                    std::array<float, 2> uv{0};
                    velocityChannel->fill(uv, idx);

                    velocityDegrees[0] = uv[0] / (DEGREE_TO_RAD * AVG_EARTH_RADIUS);
                    double latRadius = std::cos(latLon[0] * DEGREE_TO_RAD) * AVG_EARTH_RADIUS;
                    velocityDegrees[1] = uv[1] / (DEGREE_TO_RAD * latRadius);

                    velocityDegrees[0] = std::cos(latLon[0] * DEGREE_TO_RAD);
                },
                velocityChannel->size(),
                fmt::format("{}_in_lat_lon_per_second", velocityChannel->getName()));
        }

        return results;
    }
};

// struct SphericalVelocityDispatcher {

//     template <typename Result, typename T, ind N, typename... Args>
//     Result operator()(std::shared_ptr<const Channel> positionChannel, std::shared_ptr<const
//     DataChannel<float, 2>> velocityChannel) {

//                       }
// };

}  // namespace detail

}  // namespace discretedata
}  // namespace inviwo
