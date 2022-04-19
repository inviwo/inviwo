/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/properties/datasamplerproperty.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.ExtrudeDataSet, Extrude Data Set}
 * ![](org.inviwo.ExtrudeDataSet.png?classIdentifier=org.inviwo.ExtrudeDataSet)
 * Lift an existing DataSet up one dimension.
 * Take the grid and any number of channels/samplers from one DataSet and extend it into N+1
 * dimensions with 1D channel data from another set.
 * One typical use case are time series, where the grid and coordinates are only given in space,
 * with time stamps as a data series.
 */
class IVW_MODULE_DISCRETEDATA_API ExtrudeDataSet : public Processor {

public:
    ExtrudeDataSet();
    virtual ~ExtrudeDataSet() = default;

    virtual void process() override;
    virtual void deserialize(Deserializer&) override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataSetInport baseDataSetIn_, extrudeDataSetIn_;
    DataSetOutport dataOut_;
    StringProperty dataSetName_;
    IntSizeTProperty numExtrudeElements_;
    ListProperty extrudeDataMembers_;

public:
    template <typename Prop>
    class ExtendObjectProperty : public CompositeProperty {
        // static std::string classIdentifier =
        //     fmt::format("inviwo.discretedata.extendobject{}", Prop::classIdentifier);

    public:
        ExtendObjectProperty(const std::string& identifier, const std::string& displayName,
                             DataSetInport* baseData = nullptr,
                             DataSetInport* extrudeData = nullptr)
            : CompositeProperty(identifier, displayName)
            , dataObject_("base", "Base", baseData)
            , extrudingChannel_("extruding", "Extruding", extrudeData, [](auto& channel) {
                return channel->getNumComponents() == 1 &&
                       channel->getGridPrimitiveType() == GridPrimitive::Vertex;
            }) {
            addProperties(dataObject_, extrudingChannel_);
        }

        ExtendObjectProperty(const ExtendObjectProperty<Prop>& other)
            : CompositeProperty(other)
            , dataObject_(other.dataObject_)
            , extrudingChannel_(other.extrudingChannel_) {
            addProperties(dataObject_, extrudingChannel_);
        }

        ExtendObjectProperty<Prop>& operator=(const ExtendObjectProperty<Prop>& prop) {
            if (prop.extrudingChannel_.channelName_.get().compare("NONE") != 0)
                extrudingChannel_.channelName_.set(prop.extrudingChannel_.channelName_.get());
            if (prop.extrudingChannel_.datasetInput_)
                extrudingChannel_.datasetInput_ = prop.extrudingChannel_.datasetInput_;
            dataObject_ = prop.dataObject_;
            setIdentifier(prop.getIdentifier());
            setDisplayName(prop.getDisplayName());
        }

        virtual ExtendObjectProperty<Prop>* clone() const override {
            return new ExtendObjectProperty<Prop>(*this);
        }

        std::string getClassIdentifier() const;
        void setDatasetInputs(DataSetInport* baseData, DataSetInport* extrudeData) {
            dataObject_.setDatasetInput(baseData);
            extrudingChannel_.setDatasetInput(extrudeData);
        }

        Prop dataObject_;
        DataChannelProperty extrudingChannel_;
    };
};

}  // namespace discretedata

template <>
struct PropertyTraits<
    discretedata::ExtrudeDataSet::ExtendObjectProperty<discretedata::DataChannelProperty>> {
    static const std::string ClassIdentifier;
    static std::string classIdentifier() { return ClassIdentifier; }
};

template <>
struct PropertyTraits<
    discretedata::ExtrudeDataSet::ExtendObjectProperty<discretedata::DataSamplerProperty>> {
    static const std::string ClassIdentifier;
    static std::string classIdentifier() { return ClassIdentifier; }
};

template <typename Prop>
std::string discretedata::ExtrudeDataSet::ExtendObjectProperty<Prop>::getClassIdentifier() const {
    return PropertyTraits<
        discretedata::ExtrudeDataSet::ExtendObjectProperty<Prop>>::classIdentifier();
}

}  // namespace inviwo
