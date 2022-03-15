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
    , timeColumn_("timeColumn", "Time Column")
    , startTime_("startTime", "Start Time", 0.06, {0.0, ConstraintBehavior::Ignore},
                 {3.0, ConstraintBehavior::Ignore})
    , maximizeStartTime_("maximizeStartTime", "Maximize Line Output", true)
    , columnsForPosition_("positionData", "Position Columns")
    , columnsForVelocity_("VelocityData", "Velocity Columns", true)
    , columnsForAcceleration_("AccelerationData", "Acceleration Columns", true)
    , updateOutput_("updateOutput", "Update Output", true) {

    addPort(dataIn_);
    addPort(linesOut_);
    addProperties(timeColumn_, startTime_, maximizeStartTime_, columnsForPosition_,
                  columnsForVelocity_, columnsForAcceleration_, updateOutput_);
}
void LinesFromDataFrame::updateColumns() {
    if (!dataIn_.hasData()) {
        columnsForPosition_.clearProperties();
        columnsForVelocity_.clearProperties();
        columnsForAcceleration_.clearProperties();
        return;
    }

    // If this column was here before, keep it.
    std::vector<std::string> selectedPositions, selectedVelocities, selectedAccelerations;
    for (auto prop : columnsForPosition_.getPropertiesByType<BoolProperty>())
        if (prop->get()) selectedPositions.push_back(prop->getIdentifier());
    for (auto prop : columnsForVelocity_.getPropertiesByType<BoolProperty>())
        if (prop->get()) selectedVelocities.push_back(prop->getIdentifier());
    for (auto prop : columnsForAcceleration_.getPropertiesByType<BoolProperty>())
        if (prop->get()) selectedAccelerations.push_back(prop->getIdentifier());
    columnsForPosition_.clearProperties();
    columnsForVelocity_.clearProperties();
    columnsForAcceleration_.clearProperties();

    auto data = dataIn_.getData();
    std::vector<std::string> allNames, allIdentifiers;

    for (size_t i = 1; i < data->getNumberOfColumns(); i++) {
        auto c = data->getColumn(i);
        std::string displayName = c->getHeader();
        std::string identifier = util::stripIdentifier(displayName);
        allNames.push_back(displayName);
        allIdentifiers.push_back(identifier);

        // Create new properties.
        BoolProperty* posProp =
            new BoolProperty(identifier, displayName,
                             std::find(selectedPositions.begin(), selectedPositions.end(),
                                       identifier) != selectedPositions.end());
        BoolProperty* velProp =
            new BoolProperty(identifier, displayName,
                             std::find(selectedVelocities.begin(), selectedVelocities.end(),
                                       identifier) != selectedVelocities.end());
        BoolProperty* accProp =
            new BoolProperty(identifier, displayName,
                             std::find(selectedAccelerations.begin(), selectedAccelerations.end(),
                                       identifier) != selectedAccelerations.end());

        columnsForPosition_.addProperty(posProp);
        columnsForVelocity_.addProperty(velProp);
        columnsForAcceleration_.addProperty(accProp);
    }
    timeColumn_.replaceOptions(allIdentifiers, allNames, allNames);
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

    for (size_t i = 1; i < data->getNumberOfColumns(); i++) {
        auto c = data->getColumn(i);
        std::string displayName = c->getHeader();
        std::string identifier = util::stripIdentifier(displayName);
        if (dynamic_cast<BoolProperty*>(columnsForPosition_.getPropertyByIdentifier(identifier))
                ->get())
            positionColumns.push_back(c);
        if (hasVelocity &&
            dynamic_cast<BoolProperty*>(columnsForVelocity_.getPropertyByIdentifier(identifier))
                ->get())
            velocityColumns.push_back(c);
        if (hasAcceleration &&
            dynamic_cast<BoolProperty*>(columnsForAcceleration_.getPropertyByIdentifier(identifier))
                ->get())
            accelerationColumns.push_back(c);
    }

    if (hasVelocity && !velocityColumns.size()) {
        columnsForVelocity_.setChecked(false);
        hasVelocity = false;
    }
    if (hasAcceleration && !accelerationColumns.size()) {
        columnsForAcceleration_.setChecked(false);
        hasAcceleration = false;
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

    // std::vector<size_t> linesAtMaxTime;

    if (maximizeStartTime_.get()) {
        for (size_t p = 0; p < timeCol->getSize(); ++p) {
            double time = timeCol->getAsDouble(p);
            size_t timeIdx = std::max(0.0, time / timeStep + 0.5);
            if (timeIdx > dataPerFrame.size()) {
                for (size_t app = dataPerFrame.size(); app <= timeIdx; ++app) {
                    dataPerFrame.push_back(0);
                }
            }
            dataPerFrame[timeIdx]++;
        }
        auto maxMidge = std::max_element(dataPerFrame.begin(), dataPerFrame.end());
        startTime = timeStep * (maxMidge - dataPerFrame.begin());
        std::cout << fmt::format("Max {} midges at time {}", *maxMidge, startTime) << std::endl;
        startTime_.set(startTime);

        // linesAtMaxTime.reserve(*maxMidge);
        // for (size_t p = 0; p < timeCol->getSize(); ++p) {
        //     double time = timeCol->getAsDouble(p);
        //     // size_t timeIdx = std::max(0.0, time / timeStep + 0.5);
        //     if (std::abs(time - startTime) < timeStep * 0.1) {
        //         linesAtMaxTime.push_back(idColumn->getAsDouble(p));
        //     }
        // }
    }

    auto lines = std::make_shared<IntegralLineSet>(mat4(1.0));
    std::vector<dvec3>*veloVec, *accVec, *posVec = nullptr;
    double currentId = -1;  // idColumn->getAsDouble(0);
    bool skipLine = false;
    size_t numTraversed = 0;
    IntegralLine currentLine;

    for (size_t p = 1; p < idColumn->getSize(); ++p) {
        double id = idColumn->getAsDouble(p);
        bool newLine = (currentId != id);
        double time = timeCol->getAsDouble(p);
        if (newLine) {
            numTraversed++;

            // bool shouldBeIn = std::find(linesAtMaxTime.begin(), linesAtMaxTime.end(), currentId)
            // !=
            //                   linesAtMaxTime.end();
            // if (shouldBeIn) {
            //     std::cout << "Finished " << currentId << std::endl;
            //     if (skipLine) std::cout << "!  Skipped line?!?!?!?" << std::endl;
            //     if (posVec && posVec->size() <= 3)
            //         std::cout << "!  Too short: " << posVec->size() << std::endl;
            // }
            if (!skipLine && posVec && posVec->size() > 3) {
                std::cout << fmt::format("Added line, size {}", posVec->size()) << std::endl;
                // std::cout << "!  Hallelujah!" << std::endl;
                lines->push_back(std::move(currentLine), currentId);
            }
            skipLine = false;
            currentId = id;

            currentLine = IntegralLine();
            posVec = &currentLine.getPositions();
            if (hasVelocity) veloVec = &currentLine.getMetaData<dvec3>("velocity", true);
            if (hasAcceleration) accVec = &currentLine.getMetaData<dvec3>("acceleration", true);

            if (time - startTime > timeStep * 0.1) {
                // std::cout << "Line starting at " << time << " instead of " << startTime;
                skipLine = true;
                continue;
            }
        }
        newLine = posVec->size() == 0;
        // dataPerFrame[(time - startTime) / 0.01]++;
        if (skipLine) continue;

        if (startTime - time > timeStep * 0.1) continue;
        if (newLine && time - startTime > timeStep * 0.1) {
            skipLine = true;
            continue;
        }

        // if (std::abs(time - startTime - posVec->size() * 0.01) > 0.0001)
        //     std::cout << fmt::format("!!! Line at index {} at time {}, not {}", p, time,
        //                              startTime + (posVec->size() * 0.01))
        //               << std::endl;

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
    numTraversed++;

    linesOut_.setData(lines);
    std::cout << fmt::format("Traversed {} and kept {}", numTraversed, lines->size()) << std::endl;
}

}  // namespace inviwo
