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

#include <modules/vectorfieldvisualization/processors/linesfromdataframe.h>
#include <inviwo/core/util/utilities.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LinesFromDataFrame::processorInfo_{
    "org.inviwo.LinesFromDataFrame",  // Class identifier
    "Lines From Data Frame",          // Display name
    "Undefined",                      // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo LinesFromDataFrame::getProcessorInfo() const { return processorInfo_; }

LinesFromDataFrame::LinesFromDataFrame()
    : Processor()
    , dataIn_("dataFrameIn")
    , linesOut_("linesOut")
    , columnsForPosition_("positionData", "Position Columns")
    , columnsForVelocity_("VelocityData", "Velocity Columns")
    , columnsForAcceleration_("AccelerationData", "Acceleration Columns")
    , updateOutput_("updateOutput", "Update Output", true)
    , timeColumn_("timeColumn", "Time Column") {

    addPort(dataIn_);
    addPort(linesOut_);
    addProperties(timeColumn_, columnsForPosition_, columnsForVelocity_, columnsForAcceleration_,
                  updateOutput_);
}
void LinesFromDataFrame::updateColumns() {
    columnsForPosition_.clearProperties();
    columnsForVelocity_.clearProperties();
    columnsForAcceleration_.clearProperties();
    if (!dataIn_.hasData()) {
        columnDataMap_.clear();
        return;
    }

    auto data = dataIn_.getData();
    auto prevColumnDataMap = columnDataMap_;
    columnDataMap_.clear();
    std::vector<std::string> allNames, allIdentifiers;

    for (size_t i = 1; i < data->getNumberOfColumns(); i++) {
        auto c = data->getColumn(i);
        std::string displayName = c->getHeader();
        std::string identifier = util::stripIdentifier(displayName);
        allNames.push_back(displayName);
        allIdentifiers.push_back(identifier);

        // If this column was here before, keep it.
        auto& columnElement = columnDataMap_[displayName];
        columnElement = PointData::None;
        auto prevColumnElement = prevColumnDataMap.find(displayName);
        if (prevColumnElement != prevColumnDataMap.end()) {
            columnElement = prevColumnElement->second;
        }

        // BoolProperty* posProp =
        //     new BoolProperty(identifier, displayName, columnElement == PointData::Position);
        // posProp->onChange([posProp, this]() {
        //     if (posProp->get()) columnDataMap_[posProp] = PointData::Position;
        // });
        auto updateColumn = [this, identifier, displayName]() {
            char flag = 0;
            if (dynamic_cast<BoolProperty*>(columnsForPosition_.getPropertyByIdentifier(identifier))
                    ->get())
                flag |= PointData::Position;
            if (dynamic_cast<BoolProperty*>(columnsForVelocity_.getPropertyByIdentifier(identifier))
                    ->get())
                flag |= PointData::Velocity;
            if (dynamic_cast<BoolProperty*>(
                    columnsForAcceleration_.getPropertyByIdentifier(identifier))
                    ->get())
                flag |= PointData::Acceleration;

            columnDataMap_[displayName] = flag;
        };

        // Create new properties.
        BoolProperty* posProp =
            new BoolProperty(identifier, displayName, columnElement & (char)PointData::Position);
        BoolProperty* velProp =
            new BoolProperty(identifier, displayName, columnElement & (char)PointData::Velocity);
        BoolProperty* accProp = new BoolProperty(identifier, displayName,
                                                 columnElement & (char)PointData::Acceleration);

        posProp->onChange(updateColumn);
        velProp->onChange(updateColumn);
        accProp->onChange(updateColumn);

        columnsForPosition_.addProperty(posProp);
        columnsForVelocity_.addProperty(velProp);
        columnsForAcceleration_.addProperty(accProp);
        timeColumn_.replaceOptions(allIdentifiers, allNames, allNames);
    }
}

void LinesFromDataFrame::process() {
    if (dataIn_.isChanged()) updateColumns();
    if (!dataIn_.hasData()) {
        linesOut_.clear();
        return;
    }
    if (!updateOutput_.get()) return;

    auto data = dataIn_.getData();
    bool hasVelocity = columnsForVelocity_;
    bool hasAcceleration = columnsForAcceleration_;

    using ColumnVector = std::vector<std::shared_ptr<const Column>>;
    ColumnVector positionColumns, velocityColumns, accelerationColumns;

    for (size_t i = 0; i < data->getNumberOfColumns(); i++) {
        auto c = data->getColumn(i);
        std::string displayName = c->getHeader();
        char& columnFlag = columnDataMap_[displayName];
        std::cout << fmt::format("Column '{}' has flag {}", displayName, int(columnFlag))
                  << std::endl;
        if (columnFlag & PointData::Position) positionColumns.push_back(c);
        if (hasVelocity && (columnFlag & PointData::Velocity)) velocityColumns.push_back(c);
        if (hasAcceleration && (columnFlag & PointData::Acceleration))
            accelerationColumns.push_back(c);
    }

    auto idColumn = data->getColumn(1);
    if (!positionColumns.size() || !idColumn->getSize()) {
        linesOut_.clear();
        return;
    }

    auto timeCol = data->getColumn(timeColumn_.get());
    double startTime = startTime_.get();  // timeCol->getAsDouble(0);
    double timeStep = 0.01;
    std::cout << "Start time: " << startTime << std::endl;
    std::vector<size_t> dataPerFrame(30000, 0);

    auto lines = std::make_shared<IntegralLineSet>(mat4(1.0));
    std::vector<dvec3>*veloVec, *accVec, *posVec = nullptr;
    double currentId = -1;  // idColumn->getAsDouble(0);
    bool skipLine = false;
    size_t numSkipped = 0;
    IntegralLine currentLine;

    for (size_t p = 0; p < idColumn->getSize(); ++p) {
        double id = idColumn->getAsDouble(p);
        bool newLine = (currentId != id);
        double time = timeCol->getAsDouble(p);
        if (newLine) {
            if (!skipLine && posVec && posVec->size() > 3) {
                lines->push_back(std::move(currentLine), currentId);
            }
            skipLine = false;
            currentId = id;

            currentLine = IntegralLine();
            posVec = &currentLine.getPositions();
            if (hasVelocity) veloVec = &currentLine.getMetaData<dvec3>("velocity", true);
            if (hasAcceleration) accVec = &currentLine.getMetaData<dvec3>("acceleration", true);

            if (time != startTime) {
                // std::cout << "Line starting at " << time << " instead of " << startTime;
                skipLine = true;
                numSkipped++;
                continue;
            }
        }
        dataPerFrame[(time - startTime) / 0.01]++;
        if (skipLine) continue;

        if (time < startTime) continue;
        if (newLine && time > startTime) {
            skipLine = true;
            continue;
        }

        if (std::abs(time - startTime - posVec->size() * 0.01) > 0.0001)
            std::cout << fmt::format("!!! Line at index {} at time {}, not {}", p, time,
                                     startTime + (posVec->size() * 0.01))
                      << std::endl;

        dvec3 pos(0), vel(0), acc(0);
        for (size_t comp = 0; comp < std::min(size_t(3), positionColumns.size()); ++comp) {
            pos[comp] = positionColumns[comp]->getAsDouble(p);
        }
        posVec->push_back(pos);

        if (hasVelocity) {
            for (size_t comp = 0; comp < std::min(size_t(3), velocityColumns.size()); ++comp) {
                vel[comp] = velocityColumns[comp]->getAsDouble(p);
            }
            veloVec->push_back(vel);
        }

        if (hasAcceleration) {
            for (size_t comp = 0; comp < std::min(size_t(3), accelerationColumns.size()); ++comp) {
                acc[comp] = accelerationColumns[comp]->getAsDouble(p);
            }
            accVec->push_back(acc);
        }
    }
    if (posVec && posVec->size()) {
        lines->push_back(std::move(currentLine), currentId);
    }

    linesOut_.setData(lines);
    std::cout << fmt::format("Skipped {} and kept {}", numSkipped, lines->size()) << std::endl;
    auto maxMidge = std::max_element(dataPerFrame.begin(), dataPerFrame.end());
    std::cout << fmt::format("Max {} midges at time {}", *maxMidge,
                             startTime + timeStep * (maxMidge - dataPerFrame.begin()))
              << std::endl;
}

void LinesFromDataFrame::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    if (!dataIn_.hasData()) return;

    auto data = dataIn_.getData();
    for (size_t i = 1; i < data->getNumberOfColumns(); i++) {
        auto c = data->getColumn(i);
        std::string displayName = c->getHeader();
        std::string identifier = util::stripIdentifier(displayName);
        char flag = 0;

        if (dynamic_cast<BoolProperty*>(columnsForPosition_.getPropertyByIdentifier(identifier))
                ->get())
            flag |= PointData::Position;
        if (dynamic_cast<BoolProperty*>(columnsForVelocity_.getPropertyByIdentifier(identifier))
                ->get())
            flag |= PointData::Velocity;
        if (dynamic_cast<BoolProperty*>(columnsForAcceleration_.getPropertyByIdentifier(identifier))
                ->get())
            flag |= PointData::Acceleration;
        columnDataMap_[displayName] = flag;
        // for (auto prop : columnsForPosition_.getPropertiesByType<BoolProperty>()) {
        //     prop->propertyModified();
        // }
    }
    // d.deserialize("selectedChannels", deserializedChannels_);
}

void LinesFromDataFrame::serialize(Serializer& s) const {
    Processor::serialize(s);
    // s.serialize("selectedChannels", channelString);
}

}  // namespace inviwo
