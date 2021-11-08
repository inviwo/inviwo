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

#include <modules/discretedata/processors/sphericalcoordinates.h>
#include <modules/discretedata/channels/analyticchannel.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SphericalCoordinates::processorInfo_{
    "org.inviwo.SphericalCoordinates",  // Class identifier
    "Spherical Coordinates",            // Display name
    "DiscreteData",                     // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};
const ProcessorInfo SphericalCoordinates::getProcessorInfo() const { return processorInfo_; }

SphericalCoordinates::SphericalCoordinates()
    : Processor()
    , dataIn_("DataIn")
    , dataOut_("DataOut")
    , positions_("positions", "Lon/Lat Channel", &dataIn_,
                 [](auto channel) { return channel->getNumComponents() >= 2; })
    , name_("outName", "Channel Name", "SphericalCoords")
    , autoName_("autoName", "Auto Name?", true)
    , radius_("radius", "Radius", 10, 0.1, 100)
    , verticalScale_("vertScale", "Vertical Scale", 0.01, 0.001, 10, 0.001)
    , velocities_("velocity_m_s", "Velocity in m/s", &dataIn_,
                  [](auto channel) {
                      return channel->getNumComponents() == 2 &&
                             channel->getDataFormatId() == DataFormatId::Float32;
                  })
// , velocities_("velocitiesToTransform", "Velocities to Scale", [&]() {
//     std::vector<std::unique_ptr<Property>> v;
//     v.emplace_back(std::make_unique<DataChannelProperty>(
//         dataIn_, "velocity_m_s", "Velocity in m/s", [](auto channel) {
//             return channel->getNumComponents() == 2 &&
//                    channel->getDataFormatId() == DataFormatId::Float32;
//         }));
//     v.emplace_back(std::make_unique<DataChannelProperty>(
//         dataIn_, "velocity_double_m_s", "Double velocity in m/s", [](auto channel) {
//             return channel->getNumComponents() == 2 &&
//                    channel->getDataFormatId() == DataFormatId::Float64;
//         }));
//     return v;
// }())
{

    addPort(dataIn_);
    addPort(dataOut_);
    addProperties(positions_, name_, autoName_, radius_, verticalScale_, velocities_);
}

void SphericalCoordinates::process() {
    if (!(dataIn_.isChanged() || positions_.isModified() || velocities_.isModified())) return;

    auto channel = positions_.getCurrentChannel();
    if (!channel) return;

    LogInfo("|| SphericalCoordinates Updated! ||");

    if (autoName_.get()) {
        name_.set(fmt::format("Spherical{}", channel->getName()));
    }

    verticalScale_.setVisible(channel->getNumComponents() >= 3);

    std::shared_ptr<DataSet> outData = std::make_shared<DataSet>(*dataIn_.getData());
    std::shared_ptr<const DataChannel<float, 2>> velocityChannel = nullptr;
    if (velocities_.hasSelectableChannels())
        velocityChannel =
            std::dynamic_pointer_cast<const DataChannel<float, 2>>(velocities_.getCurrentChannel());

    detail::SphericalCoordinateDispatcher dispatcher;
    auto sphericals = channeldispatching::dispatch<std::array<std::shared_ptr<const Channel>, 2>,
                                                   dispatching::filter::Scalars, 2,
                                                   DISCRETEDATA_MAX_NUM_DIMENSIONS>(
        channel->getDataFormatId(), channel->getNumComponents(), dispatcher, channel, name_.get(),
        radius_.get(), verticalScale_.get(), velocityChannel);
    outData->addChannel(sphericals[0]);
    if (sphericals[1]) outData->addChannel(sphericals[1]);

    // TMP DEBUG!
    // {
    //     static const float AVG_EARTH_RADIUS = 6371000.0f;
    //     std::array<float, 2> vec;
    //     std::vector<std::array<float, 2>> latLons = {{0, 0}, {0, 0}, {0, 42}, {45, 0}};
    //     std::vector<std::array<float, 2>> uvs = {
    //         {111.32f, 111.32f}, {56, 111.32f}, {111.32f, 111.32f}, {111.32f, 111.32f}};

    //     for (int i = 0; i < latLons.size(); ++i) {
    //         vec[0] = uv[1] / AVG_EARTH_RADIUS;
    //         float latRadius = std::sin(latLon[0] * M_PI / 180.0) * AVG_EARTH_RADIUS;
    //         vec[1] = uv[0] / latRadius;
    //     }
    // }

    // Transform some velocity channels.
    // for (const auto* prop : velocities_.getPropertiesByType<DataChannelProperty>()) {
    // LogWarn("Velocity channel maybe?");
    // std::shared_ptr<const DataChannel<float, 2>> velocityChannel;
    // if (velocities_.hasSelectableChannels() &&
    //     (velocityChannel = std::dynamic_pointer_cast<const DataChannel<float, 2>>(
    //          velocities_.getCurrentChannel()))) {
    //     LogWarn("Adding velocity channel in lat/lon");
    //     // Is it given in meters per second?
    //     // std::string ms = "Velocity in m/s";
    //     // if (prop->getDisplayName().substr(0, ms.length()).compare(ms) == 0) {

    //     {  // TMP DEBUG!
    //        // std::array<float, 2> tmpUV, tmpLatLon;
    //        // tmpUV = {111.32f, };
    //        // vec[1] = uv[0] / AVG_EARTH_RADIUS;
    //        // float latRadius = std::sin(vec[1] * M_PI / 180.0);
    //        // vec[0] = uv[1] / latRadius;
    //     }

    //     // Make new channel.
    //     // // Channel* latLonChannel = new AnalyticChannel<float, 2, std::array<float, 2>>(
    //     // //     [velocityChannel, ](std::array<float, 2>& vec, ind idx) {
    //     // //         static const float AVG_EARTH_RADIUS = 6371000.0f;
    //     // //         // static const float METER_TO_LON = AVG_EARTH_RADIUS * M_PI / 180.0f;

    //     // //         std::array<float, 2> uv{0};
    //     // //         velocityChannel->fill(uv, idx);
    //     // //         vec[0] = uv[1] / AVG_EARTH_RADIUS;
    //     // //         float latRadius = std::sin(vec[0] * M_PI / 180.0) * AVG_EARTH_RADIUS;
    //     // //         vec[1] = uv[0] / latRadius;
    //     // //     },
    //     // //     velocityChannel->size(),
    //     // //     fmt::format("{}_in_lat_lon_per_second", velocityChannel->getName()));
    //     // outData->addChannel(latLonChannel);
    // }
    // }

    dataOut_.setData(outData);

    // auto minmax = dd_util::getMinMax(sphericals[0].get());
    // minmax = dd_util::getMinMax(channel.get());

    // spherical->dispatch<void>([this](auto* channel) {
    //     std::cout << "===> Spherical Channel:  " << channel->getName() << std::endl;
    //     using Vec = typename std::remove_pointer_t<decltype(channel)>::DefaultVec;
    //     // Vec vals[10];
    //     // channel->fill(vals[0], 0, 10);

    //     // for (ind i = 0; i < 10; ++i) {
    //     //     LogWarn(i << ": " << vals[i]);
    //     // }
    //     // delete[] vals;
    // });

    // channel->dispatch<void>([this](auto* channel) {
    //     std::cout << "===> Original Channel:  " << channel->getName() << std::endl;
    //     // using Vec = typename std::remove_pointer_t<decltype(channel)>::DefaultVec;
    //     // Vec val;
    //     // channel->fill(val, 0);
    //     // // for (ind i = 0; i < 3; ++i) std::cout << val[i] << " | ";
    //     // std::cout << val;
    //     // std::cout << "\n" << std::endl;

    //     // for (ind i = 0; i < 10; ++i) {
    //     //     LogWarn(i << ": " << vals[i]);
    //     // }
    //     // delete[] vals;
    // });
}

}  // namespace discretedata
}  // namespace inviwo
