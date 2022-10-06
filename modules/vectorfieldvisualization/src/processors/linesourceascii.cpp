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

#include <modules/vectorfieldvisualization/processors/linesourceascii.h>
#include <inviwo/core/util/filesystem.h>
#include <fmt/format.h>
#include <modules/vectorfieldvisualization/util/linemetrics.h>

#include <array>
#include <ctime>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LineSourceASCII::processorInfo_{
    "org.inviwo.LineSourceASCII",  // Class identifier
    "Line Source ASCII",           // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo LineSourceASCII::getProcessorInfo() const { return processorInfo_; }

LineSourceASCII::LineSourceASCII()
    : Processor()
    , linesOut_("linesOut")
    , inputFile_("filePath", "File Path")
    , maxLines_("maxLines", "Max lines?", false)
    , maxNumLines_("maxNumLines", "Number of Lines", 100, {1, ConstraintBehavior::Immutable},
                   {1000, ConstraintBehavior::Ignore})
    , overflowWrapping_("discardWrapping", "Discard wrapped lines?", true)
    , addAcceleration_("addAcceleration", "Add Accelerations", false)
    , filterStartTime_("filterStartTime", "Filter Start Time", false)
    , startTime_("startTime", "Start Time")
    , filterSeed_("filterSeed", "Filter by Starting Point")
    , filterTimeJumps_("filterTimeJumps", "Filter Jumps", true) {

    addPort(linesOut_);
    addProperties(inputFile_, maxLines_, maxNumLines_, overflowWrapping_, addAcceleration_,
                  filterSeed_, filterStartTime_, startTime_, filterTimeJumps_);
    maxNumLines_.visibilityDependsOn(maxLines_, [](auto& prop) { return prop.get(); });
    startTime_.visibilityDependsOn(filterStartTime_, [](auto& prop) { return prop.get(); });
}

void LineSourceASCII::process() {
    /* Data has form:
     * id date time lat lon t ve vn speed varlat varlon vart
     * 104118 2020-06-23 18:00:00  -23.406  -41.907   23.551 (space)
     * -3.342    8.669    9.291  2.214e-04  2.852e-04  1.329e-03
     */
    if (inputFile_.get().empty()) return;
    auto file = filesystem::ifstream(inputFile_.get());

    if (!file.is_open()) {
        throw FileException(std::string("Could not open file \"" + inputFile_.get() + "\"."),
                            IVW_CONTEXT);
    }

    std::string dataLine;
    std::getline(file, dataLine);
    size_t numColumns = std::count(dataLine.begin(), dataLine.end(), ' ') + 1;
    if (numColumns != 12) {
        throw FileException(std::string("Unexpected file header, expected 12 variables"),
                            IVW_CONTEXT);
    }
    // std::cout << ") Header line: " << dataLine << std::endl;

    // STRING VIEW TEST - would probably need to change compiler flag, and don't look forward to
    // recompiling right now...
    auto lines = std::make_shared<IntegralLineSet>(mat4(1.0));
    IntegralLine line;
    std::vector<dvec3>* posVec;

    int lineID = 0;
    size_t numTraversedLines = 0;
    intmax_t startTime = 0;
    struct tm timeStruct;
    std::array<double, 2> position;
    double lonOffset = 0;
    bool skipCurrentLine = false;

    char delim = ' ';
    int year, month, day, hours, minutes, seconds;

    while (std::getline(file, dataLine)) {
        if (maxLines_.get() && numTraversedLines >= maxNumLines_.get()) break;

        size_t wordStart = dataLine.find_first_not_of(delim, 0);
        size_t wordEnd = 0;

        // Get line ID from "  104118 ".
        wordEnd = dataLine.find(delim, wordStart);
        dataLine[wordEnd] = '\0';
        int currentLineID = std::atoi(dataLine.c_str() + wordStart);
        bool newLine = currentLineID != lineID;
        if (newLine) {
            numTraversedLines++;
            if (!skipCurrentLine) {
                // Add the last line to the LineSet.
                if (line.getPositions().size() > 0) {
                    if (lineID && line.getPositions().size() >= 4) {
                        lines->push_back(std::move(line), lineID);
                    }

                    // Start a new line.
                    line = IntegralLine();
                }
                posVec = &line.getPositions();
                // veloVec = &line.getMetaData<dvec3>("velocity", true);
                // tempVec = &line.getMetaData<double>("temperature", true);
                // speedVec = &line.getMetaData<double>("speed", true);
                lineID = currentLineID;
                lonOffset = 0;
            }
            skipCurrentLine = false;
        } else if (skipCurrentLine) {
            continue;
        }
        wordStart = wordEnd + 1;

        // Get date from "2020-06-23 18:00:00".
        std::sscanf(dataLine.c_str() + wordStart, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hours,
                    &minutes, &seconds);
        timeStruct.tm_year = year - 1900;
        timeStruct.tm_mon = month - 1;
        timeStruct.tm_mday = day;
        timeStruct.tm_hour = hours;
        timeStruct.tm_min = minutes;
        timeStruct.tm_sec = seconds;
        timeStruct.tm_isdst = -1;

        intmax_t timeInt = std::mktime(&timeStruct);
        if (!startTime) {
            startTime = timeInt / (60 * 60 * 6);  // Only measure in 6h steps.
            std::cout << fmt::format("Start Date: {}", dataLine) << std::endl;
        }
        timeInt = timeInt / (60 * 60 * 6) - startTime;

        // Stop if seed starts after start time.
        if (filterStartTime_.get() && newLine && timeInt > startTime_.get()) {
            skipCurrentLine = true;
            posVec->clear();
            continue;
        }
        // Ignore point if we are before the start time.
        if (filterStartTime_.get() && timeInt < startTime_.get()) {
            lineID = 0;
            numTraversedLines--;
            continue;
        }

        if (filterStartTime_.get()) timeInt -= startTime_.get();

        static bool first = true;
        if (first) {
            std::cout << "filtered time: " << dataLine << std::endl;
            first = false;
        }

        //     // Delete line if there is a time jump at any point.
        //     if (filterTimeJumps_.get() && posVec->size() &&
        //         (double(timeInt) - posVec->back().z) > 1.1) {
        //         skipCurrentLine = true;
        //         posVec->clear();
        //         continue;
        //     }

        //     // Get position, temperature, velocity and speed from
        //     // "  -23.406  -41.907   23.551  -3.342    8.669    9.291  ".
        //     // Read as 6 scalar values and assign accordingly.

        //     wordStart += 20;
        //     for (size_t var = 0; var < 2; ++var) {
        //         wordStart = dataLine.find_first_not_of(delim, wordStart);
        //         wordEnd = dataLine.find(delim, wordStart);
        //         dataLine[wordEnd] = '\0';
        //         position[var] = std::atof(dataLine.c_str() + wordStart);
        //         wordStart = wordEnd + 1;
        //     }

        //     // Potentially filter by starting position.
        //     if (newLine && filterSeed_) {
        //         dvec2 seed{position[1], position[0]};
        //         skipCurrentLine = !filterSeed_.isInside(seed);
        //         if (skipCurrentLine) continue;
        //     }

        //     // Potentially offset points that would wrap around -180/180 lon to overflow instead.
        //     position[1] += lonOffset;
        //     if (overflowWrapping_.get() && posVec->size()) {
        //         double lonDist = position[1] - posVec->back()[0];
        //         if (lonDist < -300) {
        //             lonOffset += 360;
        //             position[1] += 360;
        //         }
        //         if (lonDist > 300) {
        //             lonOffset -= 360;
        //             position[1] -= 360;
        //         }
        //     }

        //     posVec->emplace_back(position[1], position[0], timeInt);
    }
    // if (!skipCurrentLine && line.getPositions().size() > 0)
    //     lines->push_back(std::move(line), lineID);

    // // Compute velocity manually, and potentially two types of acceleration.
    // util::addVelocity(*lines);
    // if (addAcceleration_.get()) {
    //     util::addAcceleration(*lines, true, "accelerationVelo");
    //     util::addAcceleration(*lines, false, "accelerationPos");
    // }

    // linesOut_.setData(lines);
}

LineSourceASCII::SeedFilter::SeedFilter(const std::string& identifier,
                                        const std::string& displayName)
    : BoolCompositeProperty(identifier, displayName)
    , filterType_("filterType", "Type",
                  {{"Circle", "Circle", FilterType::Circle},
                   {"Rectangle", "Rectangle", FilterType::Rectangle}},
                  0, InvalidationLevel::InvalidResources)
    , center_("center", "Center", dvec3{0, 0, 0}, dvec3{-180, -90, 0}, dvec3{180, 90, 0})
    , radius_("radius", "Radius", 10, 0.001, 45)
    , min_("min", "Min", {0, 0, 0}, {vec3(0, 0, 0), ConstraintBehavior::Ignore},
           {vec3(1, 1, 0), ConstraintBehavior::Ignore})
    , max_("max", "Max", {1, 1, 0}, {vec3(0, 0, 0), ConstraintBehavior::Ignore},
           {vec3(1, 1, 0), ConstraintBehavior::Ignore}) {
    addProperties(filterType_, center_, radius_, min_, max_);
    center_.visibilityDependsOn(filterType_,
                                [](auto& prop) { return prop.get() == FilterType::Circle; });
    radius_.visibilityDependsOn(filterType_,
                                [](auto& prop) { return prop.get() == FilterType::Circle; });

    min_.visibilityDependsOn(filterType_,
                             [](auto& prop) { return prop.get() == FilterType::Rectangle; });
    max_.visibilityDependsOn(filterType_,
                             [](auto& prop) { return prop.get() == FilterType::Rectangle; });
}

bool LineSourceASCII::SeedFilter::isInside(dvec2 pos) {
    double dist;
    switch (filterType_.get()) {
        case FilterType::Circle:
            dist = glm::distance(pos, dvec2(center_.get()));
            return (dist <= radius_.get());
        case FilterType::Rectangle:
            return (pos.x > min_.get().x && pos.x < max_.get().x &&  // x
                    pos.y > min_.get().y && pos.y < max_.get().y);   // y
    }
    return true;
}

}  // namespace inviwo
