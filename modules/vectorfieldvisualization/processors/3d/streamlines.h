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

protected:
    VolumeInport volume_;
    SeedPointsInport seedPoints_;

    MeshOutport linesStripsMesh_;

    StreamLineProperties streamLineProperties_;

    TransferFunctionProperty tf_;
    FloatProperty velocityScale_;
    StringProperty maxVelocity_;
};

}  // namespace

#endif  // IVW_STREAMLINES_H
