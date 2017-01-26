#ifndef IVW_STREAMLINES_H
#define IVW_STREAMLINES_H

#include <inviwo/core/common/inviwo.h>
#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>

#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/vectorfieldvisualization/streamlinetracer.h>

#include <modules/vectorfieldvisualization/ports/seedpointsport.h>
#include <modules/vectorfieldvisualization/properties/streamlineproperties.h>
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>

namespace inviwo {

class SimpleMesh;
class VolumeRAM;

/** \docpage{org.inviwo.StreamLines, StreamLines}
 * ![](org.inviwo.StreamLines.png?classIdentifier=org.inviwo.StreamLines)
 *
 * ...
 *
 * ### Inports
 *   * __seedpoints__ ...
 *   * __vectorvolume__ ...
 *
 * ### Outports
 *   * __linesStripsMesh___ ...
 *
 * ### Properties
 *   * __Velocity Scale__ ...
 *   * __Velocity Range__ ...
 *   * __StepSize__ ...
 *   * __Number of Steps__ ...
 *   * __Step Direction__ ...
 *   * __Transfer Function__ ...
 *
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API StreamLines : public Processor {
public:
    StreamLines();
    virtual ~StreamLines();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

    virtual bool isReady() const override {
        if (Processor::isReady()) {
            return true;
        }

        if (!seedPoints_.isReady()) return false;

        if (sampler_.isConnected()) {
            return sampler_.isReady();
        }
        if (volume_.isConnected()) {
            return volume_.isReady();
        }
        return false;
    }

protected:
    DataInport<SpatialSampler<3, 3, double>> sampler_;
    SeedPoints3DInport seedPoints_;
    VolumeInport volume_;
    MeshOutport linesStripsMesh_;
    IntegralLineSetOutport lines_;

    StreamLineProperties streamLineProperties_;

    TransferFunctionProperty tf_;
    FloatProperty velocityScale_;
    StringProperty maxVelocity_;

    BoolProperty useOpenMP_;
};

}  // namespace

#endif  // IVW_STREAMLINES_H
