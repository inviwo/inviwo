# Inviwo Core
project(inviwo-core)

set(HEADER_FILES
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/base64.h
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/boundingbox.h
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/camerautils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/cubeplaneintersection.h
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/histogram1d.h
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/linearmap.h
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/markdown.h
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/permutations.h
    ${IVW_INCLUDE_DIR}/inviwo/core/algorithm/searchdsl.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/defaulttohighperformancegpu.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/factoryutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwo.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwoapplication.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwoapplicationutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwocore.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwocoredefine.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwomodule.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwomodulefactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwomodulelibraryobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/moduleaction.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/modulecallback.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/modulecontainer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/modulemanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/modulepath.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/runtimemoduleregistration.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/version.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/bitset.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/buffer/buffer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/buffer/bufferram.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/buffer/bufferramprecision.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/buffer/bufferrepresentation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/camera.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/camera/camera.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/camera/camerafactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/camera/camerafactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/camera/cameratools.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/camera/orthographiccamera.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/camera/perspectivecamera.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/camera/skewedperspectivecamera.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/coordinatetransformer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/data.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/datagroup.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/datagrouprepresentation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/datamapper.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/datarepresentation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/datasequence.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/datatraits.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/diskrepresentation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/basicmesh.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/edge.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/geometrytype.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/mesh.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/meshram.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/meshrepresentation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/plane.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/polygon.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/simplemesh.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/simplemeshcreator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/typedmesh.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/histogram.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/histogramtools.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/image.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/imageram.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/imagerepresentation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/imagetypes.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/layer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/layerconfig.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/layerdisk.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/layerram.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/layerramconverter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/layerramprecision.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/layerrepresentation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/layerutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/isovaluecollection.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/light/baselightsource.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/light/diffuselight.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/light/directionallight.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/light/lightingconfig.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/light/lightingstate.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/light/pointlight.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/light/spotlight.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/nodata.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationconverter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationconverterfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationconvertermetafactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationfactorymanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationmetafactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationtraits.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/representationutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/spatialdata.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/tfdata.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/tflookuptable.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/tfprimitive.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/tfprimitiveset.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/transferfunction.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/unitsystem.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volume.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volumeborder.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volumeconfig.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volumedisk.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volumeram.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volumeramconverter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volumeramprecision.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volumerepresentation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/cameratrackball.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/event.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/eventhandler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/eventlistener.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/eventmatcher.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/eventpropagator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/eventutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/gestureevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/gesturestate.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/interactionevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/keyboardevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/keyboardkeys.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/mousebuttons.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/mousecursors.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/mouseevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/mouseinteractionevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/pickingevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/resizeevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/touchevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/touchstate.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/viewevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/events/wheelevent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/interactionhandler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/pickingaction.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/pickingcontroller.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/pickingcontrollermousestate.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/pickingcontrollertouchstate.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/pickingmanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/pickingmapper.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/pickingstate.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/trackball.h
    ${IVW_INCLUDE_DIR}/inviwo/core/interaction/trackballobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/bytereaderutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/curlutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/datareader.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/datareaderexception.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/datareaderfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/datawriter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/datawriterexception.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/datawriterfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/datawriterutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/imagewriterutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/isovaluecollectioniivreader.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/isovaluecollectioniivwriter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/rawvolumeramloader.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/rawvolumereader.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/deserializer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/nodedebugger.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/serializable.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/serialization.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/serializationexception.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/serializebase.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/serializeconstants.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/serializer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/ticpp.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/versionconverter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/tempfilehandle.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/textfilereader.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/transferfunctionitfreader.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/transferfunctionitfwriter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/transferfunctionlayerreader.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/transferfunctionlayerwriter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/transferfunctionxmlreader.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/transferfunctionxmlwriter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/volumedatareaderdialog.h
    ${IVW_INCLUDE_DIR}/inviwo/core/links/linkevaluator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/links/propertylink.h
    ${IVW_INCLUDE_DIR}/inviwo/core/metadata/containermetadata.h
    ${IVW_INCLUDE_DIR}/inviwo/core/metadata/metadata.h
    ${IVW_INCLUDE_DIR}/inviwo/core/metadata/metadatafactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/metadata/metadatamap.h
    ${IVW_INCLUDE_DIR}/inviwo/core/metadata/metadataowner.h
    ${IVW_INCLUDE_DIR}/inviwo/core/metadata/positionmetadata.h
    ${IVW_INCLUDE_DIR}/inviwo/core/metadata/processormetadata.h
    ${IVW_INCLUDE_DIR}/inviwo/core/metadata/processorwidgetmetadata.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/autolinker.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/evaluationerrorhandler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/lambdanetworkvisitor.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/networkedge.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/networklock.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/networkutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/networkvisitor.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/portconnection.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/processornetwork.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/processornetworkconverter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/processornetworkevaluationobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/processornetworkevaluator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/processornetworkobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/workspaceannotations.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/workspacemanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/network/workspaceutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/bufferport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/datainport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/dataoutport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/imageport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/inport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/inportiterable.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/layerport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/meshport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/outport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/outportiterable.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/port.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/portfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/portfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/portinspector.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/portinspectorfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/portinspectorfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/portinspectormanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/porttraits.h
    ${IVW_INCLUDE_DIR}/inviwo/core/ports/volumeport.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/activityindicator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/canvasprocessor.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/canvasprocessorwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/compositeprocessor.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/compositeprocessorfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/compositeprocessorutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/compositesink.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/compositesource.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/exporter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/poolprocessor.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processor.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorinfo.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorpair.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorstate.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorstatus.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processortags.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processortraits.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorwidgetfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processorwidgetfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/progressbar.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/progressbarowner.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/advancedmaterialproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/boolcompositeproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/boolproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/buttongroupproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/buttonproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/cameraproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/compositeproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/compositepropertyobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/constraintbehavior.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/directoryproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/eventproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/filepatternproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/fileproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/imageeditorproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/invalidationlevel.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/isotfproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/isovalueproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/listproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/marginproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/minmaxproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/multifileproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/optionproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/optionpropertytraits.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/ordinalproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/ordinalrefproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/planeproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/positionproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/property.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyconverter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyconvertermanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyeditorwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyowner.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyownerobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertypresetmanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertysemantics.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertywidget.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertywidgetfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertywidgetfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/raycastingproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/selectioncolorproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/simplelightingproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/simpleraycastingproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/stringproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/stringsproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/templateproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/transferfunctionproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/valuewrapper.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/volumeindicatorproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/rendering/datavisualizer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/rendering/datavisualizermanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/rendering/meshdrawer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/rendering/meshdrawerfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/resourcemanager/resource.h
    ${IVW_INCLUDE_DIR}/inviwo/core/resourcemanager/resourcemanager.h
    ${IVW_INCLUDE_DIR}/inviwo/core/resourcemanager/resourcemanagerobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/assertion.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/brickiterator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/bufferutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/buildinfo.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/callback.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/canvas.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/capabilities.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/chronoutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/clock.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/cloneableptr.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/colorbrewer-generated.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/colorbrewer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/colorconversion.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/commandlineparser.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/consolelogger.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/constexprhash.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/datetime.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/defaultvalues.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/demangle.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/detected.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/dialog.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/dialogfactory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/dialogfactoryobject.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/dispatcher.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/docbuilder.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/document.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/docutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/enumtraits.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/exception.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/factory.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/filedialog.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/filedialogstate.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/fileextension.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/filelogger.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/fileobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/filesystem.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/filesystemobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/fmtutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/foreach.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/foreacharg.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/formatconversion.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/formatdispatching.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/formats.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glm.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmcomp.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmconvert.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmfmt.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmmat.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmmatext.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmvec.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/hashcombine.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/imagecache.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/imageramutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/imagesampler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/indexmapper.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/indirectiterator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/interpolation.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/intersection/rayplaneintersection.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/intersection/raysphereintersection.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/introspection.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/inviwosetupinfo.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/isstreaminsertable.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/iterrange.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/licenseinfo.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/localetools.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/logcentral.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/logerrorcounter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/logfilter.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/logstream.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/memoryfilehandle.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/metadatatoproperty.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/moduleutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/moveonlyvalue.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/networkdebugobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/observer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/ostreamjoiner.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/pathtype.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/pmrutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/raiiutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/rendercontext.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/safecstr.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/settings/linksettings.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/settings/settings.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/settings/systemsettings.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/settings/unitsettings.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/sharedlibrary.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/shuntingyard.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/singlefileobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/singleton.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/sourcecontext.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/spatial4dsampler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/spatialsampler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/stacktrace.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/statecoordinator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/staticstring.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/stdextensions.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/stdfuture.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/stringconversion.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/stringlogger.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/templatesampler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/threadpool.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/threadutil.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/timer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/transformiterator.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/transparentmaps.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/typetraits.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/unindent.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/utilities.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/vectoroperations.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/volumeramutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/volumesampler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/volumesequencesampler.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/volumesequenceutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/volumeutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/zip.h
)
ivw_group("Header Files" BASE "${IVW_INCLUDE_DIR}/inviwo/core" ${HEADER_FILES})

set(SOURCE_FILES
    algorithm/base64.cpp
    algorithm/boundingbox.cpp
    algorithm/camerautils.cpp
    algorithm/cubeplaneintersection.cpp
    algorithm/histogram1d.cpp
    algorithm/linearmap.cpp
    algorithm/markdown.cpp
    algorithm/permutations.cpp
    algorithm/searchdsl.cpp
    common/factoryutil.cpp
    common/inviwoapplication.cpp
    common/inviwoapplicationutil.cpp
    common/inviwocore.cpp
    common/inviwomodule.cpp
    common/inviwomodulefactoryobject.cpp
    common/inviwomodulelibraryobserver.cpp
    common/moduleaction.cpp
    common/modulecontainer.cpp
    common/modulemanager.cpp
    common/modulepath.cpp
    common/version.cpp
    datastructures/bitset.cpp
    datastructures/buffer/buffer.cpp
    datastructures/buffer/bufferram.cpp
    datastructures/buffer/bufferrepresentation.cpp
    datastructures/camera/camera.cpp
    datastructures/camera/camerafactoryobject.cpp
    datastructures/camera/cameratools.cpp
    datastructures/camera/orthographiccamera.cpp
    datastructures/camera/perspectivecamera.cpp
    datastructures/camera/skewedperspectivecamera.cpp
    datastructures/coordinatetransformer.cpp
    datastructures/datamapper.cpp
    datastructures/datarepresentation.cpp
    datastructures/datasequence.cpp
    datastructures/datatraits.cpp
    datastructures/geometry/basicmesh.cpp
    datastructures/geometry/geometrytype.cpp
    datastructures/geometry/mesh.cpp
    datastructures/geometry/meshram.cpp
    datastructures/geometry/meshrepresentation.cpp
    datastructures/geometry/plane.cpp
    datastructures/geometry/simplemesh.cpp
    datastructures/geometry/simplemeshcreator.cpp
    datastructures/histogram.cpp
    datastructures/histogramtools.cpp
    datastructures/image/image.cpp
    datastructures/image/imageram.cpp
    datastructures/image/imagerepresentation.cpp
    datastructures/image/imagetypes.cpp
    datastructures/image/layer.cpp
    datastructures/image/layerconfig.cpp
    datastructures/image/layerdisk.cpp
    datastructures/image/layerram.cpp
    datastructures/image/layerramconverter.cpp
    datastructures/image/layerramprecision.cpp
    datastructures/image/layerrepresentation.cpp
    datastructures/image/layerutil.cpp
    datastructures/isovaluecollection.cpp
    datastructures/light/baselightsource.cpp
    datastructures/light/diffuselight.cpp
    datastructures/light/directionallight.cpp
    datastructures/light/lightingconfig.cpp
    datastructures/light/lightingstate.cpp
    datastructures/light/pointlight.cpp
    datastructures/light/spotlight.cpp
    datastructures/representationconvertermetafactory.cpp
    datastructures/representationfactory.cpp
    datastructures/representationfactorymanager.cpp
    datastructures/representationfactoryobject.cpp
    datastructures/representationmetafactory.cpp
    datastructures/representationutil.cpp
    datastructures/spatialdata.cpp
    datastructures/tfdata.cpp
    datastructures/tflookuptable.cpp
    datastructures/tfprimitive.cpp
    datastructures/tfprimitiveset.cpp
    datastructures/transferfunction.cpp
    datastructures/unitsystem.cpp
    datastructures/volume/volume.cpp
    datastructures/volume/volumeborder.cpp
    datastructures/volume/volumeconfig.cpp
    datastructures/volume/volumedisk.cpp
    datastructures/volume/volumeram.cpp
    datastructures/volume/volumeramconverter.cpp
    datastructures/volume/volumeramprecision.cpp
    datastructures/volume/volumerepresentation.cpp
    interaction/cameratrackball.cpp
    interaction/events/event.cpp
    interaction/events/eventhandler.cpp
    interaction/events/eventlistener.cpp
    interaction/events/eventmatcher.cpp
    interaction/events/eventutil.cpp
    interaction/events/gestureevent.cpp
    interaction/events/gesturestate.cpp
    interaction/events/interactionevent.cpp
    interaction/events/keyboardevent.cpp
    interaction/events/keyboardkeys.cpp
    interaction/events/mousebuttons.cpp
    interaction/events/mousecursors.cpp
    interaction/events/mouseevent.cpp
    interaction/events/mouseinteractionevent.cpp
    interaction/events/pickingevent.cpp
    interaction/events/resizeevent.cpp
    interaction/events/touchevent.cpp
    interaction/events/touchstate.cpp
    interaction/events/viewevent.cpp
    interaction/events/wheelevent.cpp
    interaction/interactionhandler.cpp
    interaction/pickingaction.cpp
    interaction/pickingcontroller.cpp
    interaction/pickingcontrollermousestate.cpp
    interaction/pickingcontrollertouchstate.cpp
    interaction/pickingmanager.cpp
    interaction/pickingmapper.cpp
    interaction/pickingstate.cpp
    interaction/trackball.cpp
    io/bytereaderutil.cpp
    io/curlutils.cpp
    io/datareader.cpp
    io/datareaderexception.cpp
    io/datareaderfactory.cpp
    io/datawriter.cpp
    io/datawriterexception.cpp
    io/datawriterfactory.cpp
    io/datawriterutil.cpp
    io/imagewriterutil.cpp
    io/isovaluecollectioniivreader.cpp
    io/isovaluecollectioniivwriter.cpp
    io/rawvolumeramloader.cpp
    io/rawvolumereader.cpp
    io/serialization/deserializer.cpp
    io/serialization/nodedebugger.cpp
    io/serialization/serializationexception.cpp
    io/serialization/serializebase.cpp
    io/serialization/serializeconstants.cpp
    io/serialization/serializer.cpp
    io/serialization/versionconverter.cpp
    io/tempfilehandle.cpp
    io/textfilereader.cpp
    io/transferfunctionitfreader.cpp
    io/transferfunctionitfwriter.cpp
    io/transferfunctionlayerreader.cpp
    io/transferfunctionlayerwriter.cpp
    io/transferfunctionxmlreader.cpp
    io/transferfunctionxmlwriter.cpp
    io/volumedatareaderdialog.cpp
    links/linkevaluator.cpp
    links/propertylink.cpp
    metadata/metadata.cpp
    metadata/metadatamap.cpp
    metadata/metadataowner.cpp
    metadata/positionmetadata.cpp
    metadata/processormetadata.cpp
    metadata/processorwidgetmetadata.cpp
    network/autolinker.cpp
    network/evaluationerrorhandler.cpp
    network/lambdanetworkvisitor.cpp
    network/networkedge.cpp
    network/networklock.cpp
    network/networkutils.cpp
    network/networkvisitor.cpp
    network/portconnection.cpp
    network/processornetwork.cpp
    network/processornetworkconverter.cpp
    network/processornetworkevaluationobserver.cpp
    network/processornetworkevaluator.cpp
    network/processornetworkobserver.cpp
    network/workspaceannotations.cpp
    network/workspacemanager.cpp
    network/workspaceutils.cpp
    ports/imageport.cpp
    ports/inport.cpp
    ports/layerport.cpp
    ports/outport.cpp
    ports/port.cpp
    ports/portfactory.cpp
    ports/portfactoryobject.cpp
    ports/portinspector.cpp
    ports/portinspectorfactory.cpp
    ports/portinspectorfactoryobject.cpp
    ports/portinspectormanager.cpp
    ports/porttraits.cpp
    processors/activityindicator.cpp
    processors/canvasprocessor.cpp
    processors/canvasprocessorwidget.cpp
    processors/compositeprocessor.cpp
    processors/compositeprocessorfactoryobject.cpp
    processors/compositeprocessorutils.cpp
    processors/compositesink.cpp
    processors/compositesource.cpp
    processors/exporter.cpp
    processors/poolprocessor.cpp
    processors/processor.cpp
    processors/processorfactory.cpp
    processors/processorinfo.cpp
    processors/processorpair.cpp
    processors/processorstate.cpp
    processors/processorstatus.cpp
    processors/processortags.cpp
    processors/processorutils.cpp
    processors/processorwidget.cpp
    processors/processorwidgetfactory.cpp
    processors/processorwidgetfactoryobject.cpp
    processors/progressbar.cpp
    processors/progressbarowner.cpp
    properties/advancedmaterialproperty.cpp
    properties/boolcompositeproperty.cpp
    properties/boolproperty.cpp
    properties/buttongroupproperty.cpp
    properties/buttonproperty.cpp
    properties/cameraproperty.cpp
    properties/compositeproperty.cpp
    properties/compositepropertyobserver.cpp
    properties/constraintbehavior.cpp
    properties/directoryproperty.cpp
    properties/eventproperty.cpp
    properties/filepatternproperty.cpp
    properties/fileproperty.cpp
    properties/imageeditorproperty.cpp
    properties/invalidationlevel.cpp
    properties/isotfproperty.cpp
    properties/isovalueproperty.cpp
    properties/listproperty.cpp
    properties/marginproperty.cpp
    properties/multifileproperty.cpp
    properties/optionproperty.cpp
    properties/optionpropertytraits.cpp
    properties/ordinalproperty.cpp
    properties/ordinalrefproperty.cpp
    properties/planeproperty.cpp
    properties/positionproperty.cpp
    properties/property.cpp
    properties/propertyconverter.cpp
    properties/propertyconvertermanager.cpp
    properties/propertyeditorwidget.cpp
    properties/propertyfactory.cpp
    properties/propertyfactoryobject.cpp
    properties/propertyobserver.cpp
    properties/propertyowner.cpp
    properties/propertyownerobserver.cpp
    properties/propertypresetmanager.cpp
    properties/propertysemantics.cpp
    properties/propertywidget.cpp
    properties/propertywidgetfactory.cpp
    properties/propertywidgetfactoryobject.cpp
    properties/raycastingproperty.cpp
    properties/selectioncolorproperty.cpp
    properties/simplelightingproperty.cpp
    properties/simpleraycastingproperty.cpp
    properties/stringproperty.cpp
    properties/stringsproperty.cpp
    properties/transferfunctionproperty.cpp
    properties/volumeindicatorproperty.cpp
    rendering/datavisualizer.cpp
    rendering/datavisualizermanager.cpp
    rendering/meshdrawerfactory.cpp
    resourcemanager/resource.cpp
    resourcemanager/resourcemanager.cpp
    resourcemanager/resourcemanagerobserver.cpp
    util/assertion.cpp
    util/brickiterator.cpp
    util/bufferutils.cpp
    util/canvas.cpp
    util/capabilities.cpp
    util/chronoutils.cpp
    util/clock.cpp
    util/colorbrewer-generated.cpp
    util/colorbrewer.cpp
    util/colorconversion.cpp
    util/commandlineparser.cpp
    util/consolelogger.cpp
    util/defaultvalues.cpp
    util/demangle.cpp
    util/detected.cpp
    util/dialogfactory.cpp
    util/dialogfactoryobject.cpp
    util/docbuilder.cpp
    util/document.cpp
    util/docutils.cpp
    util/enumtraits.cpp
    util/exception.cpp
    util/factory.cpp
    util/filedialog.cpp
    util/filedialogstate.cpp
    util/fileextension.cpp
    util/filelogger.cpp
    util/fileobserver.cpp
    util/filesystem.cpp
    util/filesystemobserver.cpp
    util/fmtutils.cpp
    util/foreacharg.cpp
    util/formatconversion.cpp
    util/formatdispatching.cpp
    util/formats.cpp
    util/glm.cpp
    util/glmcomp.cpp
    util/glmconvert.cpp
    util/glmfmt.cpp
    util/glmmat.cpp
    util/glmmatext.cpp
    util/glmutils.cpp
    util/glmvec.cpp
    util/hashcombine.cpp
    util/imagecache.cpp
    util/imageramutils.cpp
    util/imagesampler.cpp
    util/indirectiterator.cpp
    util/introspection.cpp
    util/inviwosetupinfo.cpp
    util/isstreaminsertable.cpp
    util/iterrange.cpp
    util/licenseinfo.cpp
    util/localetools.cpp
    util/logcentral.cpp
    util/logerrorcounter.cpp
    util/logfilter.cpp
    util/logstream.cpp
    util/memoryfilehandle.cpp
    util/metadatatoproperty.cpp
    util/moduleutils.cpp
    util/moveonlyvalue.cpp
    util/networkdebugobserver.cpp
    util/observer.cpp
    util/pmrutils.cpp
    util/rendercontext.cpp
    util/safecstr.cpp
    util/settings/linksettings.cpp
    util/settings/settings.cpp
    util/settings/systemsettings.cpp
    util/settings/unitsettings.cpp
    util/sharedlibrary.cpp
    util/shuntingyard.cpp
    util/singlefileobserver.cpp
    util/sourcecontext.cpp
    util/spatial4dsampler.cpp
    util/stacktrace.cpp
    util/statecoordinator.cpp
    util/staticstring.cpp
    util/stdextensions.cpp
    util/stdfuture.cpp
    util/stringconversion.cpp
    util/stringlogger.cpp
    util/threadpool.cpp
    util/threadutil.cpp
    util/timer.cpp
    util/transparentmaps.cpp
    util/typetraits.cpp
    util/unindent.cpp
    util/utilities.cpp
    util/vectoroperations.cpp
    util/volumesampler.cpp
    util/volumesequencesampler.cpp
    util/volumesequenceutils.cpp
    util/volumeutils.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# include implementation for build info - the build info will be
# updated during every compilation

option(IVW_CFG_EMBED_BUILDINFO "Embed buildinfo into the binary file instead of using a ini file, will force inviwo-core to recompile every build" OFF)
if(IVW_CFG_EMBED_BUILDINFO)
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/src/util/inviwo_buildinfo.cpp")
    ivw_group("Source Files" BASE "${CMAKE_CURRENT_BINARY_DIR}/src/"
        "${CMAKE_CURRENT_BINARY_DIR}/src/util/inviwo_buildinfo.cpp")
else()
    list(APPEND SOURCE_FILES util/buildinfoini.cpp)
    ivw_group("Source Files" util/buildinfoini.cpp)
endif()

configure_file(${IVW_CMAKE_TEMPLATES}/inviwocommondefines_template.h
               ${CMAKE_CURRENT_BINARY_DIR}/include/inviwo/core/common/inviwocommondefines.h
               @ONLY IMMEDIATE)
list(APPEND HEADER_FILES "${CMAKE_CURRENT_BINARY_DIR}/include/inviwo/core/common/inviwocommondefines.h")
ivw_group("Header Files" BASE "${CMAKE_CURRENT_BINARY_DIR}/include/inviwo/core/"
    ${CMAKE_CURRENT_BINARY_DIR}/include/inviwo/core/common/inviwocommondefines.h)

set(TEST_FILES
    tests/unittests/bitset-test.cpp
    tests/unittests/brickiterator-test.cpp
    tests/unittests/colorconversion-test.cpp
    tests/unittests/commandlineparser-test.cpp
    tests/unittests/conversion-test.cpp
    tests/unittests/dataformats-test.cpp
    tests/unittests/dispatch-test.cpp
    tests/unittests/document-test.cpp
    tests/unittests/enumoptionproperty-test.cpp
    tests/unittests/glm-test.cpp
    tests/unittests/histogram1d-test.cpp
    tests/unittests/image-tests.cpp
    tests/unittests/indirectiterator-tests.cpp
    tests/unittests/interpolation-tests.cpp
    tests/unittests/inviwo-core-unittest-main.cpp
    tests/unittests/metadata-test.cpp
    tests/unittests/network-evaluator-test.cpp
    tests/unittests/optionproperty-test.cpp
    tests/unittests/ordinalproperty-test.cpp
    tests/unittests/permutations-test.cpp
    tests/unittests/picking-test.cpp
    tests/unittests/pickingcontroller-test.cpp
    tests/unittests/port-tests.cpp
    tests/unittests/resize-test.cpp
    tests/unittests/serialize-container-test.cpp
    tests/unittests/serializer-polymorphic-test.cpp
    tests/unittests/serializer-test.cpp
    tests/unittests/staticstring-test.cpp
    tests/unittests/stringconversion-test.cpp
    tests/unittests/tfprimitiveset-test.cpp
    tests/unittests/typedmesh-test.cpp
    tests/unittests/unitsystem-test.cpp
    tests/unittests/utilities-test.cpp
    tests/unittests/volumesequenceutils-tests.cpp
    tests/unittests/zip-test.cpp
)
ivw_add_unittest(${TEST_FILES})

set(GEN_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include/inviwo/core/common/coremodulesharedlibrary.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/common/coremodulesharedlibrary.cpp
)
source_group("Header Files\\common" FILES ${CMAKE_CURRENT_BINARY_DIR}/include/inviwo/core/common/coremodulesharedlibrary.h)
source_group("Source Files\\common" FILES ${CMAKE_CURRENT_BINARY_DIR}/src/common/coremodulesharedlibrary.cpp)

set(CMAKE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
)
source_group("CMake Files" FILES ${CMAKE_FILES})

# Create library
add_library(inviwo-core 
    ${GEN_FILES} ${CMAKE_FILES}
    $<$<BOOL:${WIN32}>:${IVW_ROOT_DIR}/tools/natvis/inviwo.natvis>
    $<$<BOOL:${WIN32}>:${IVW_ROOT_DIR}/tools/natvis/glm.natvis>
    ${IVW_ROOT_DIR}/.editorconfig
)
add_library(inviwo::core ALIAS inviwo-core)

target_sources(inviwo-core 
    PUBLIC 
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS 
        ${IVW_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include/
    FILES 
        ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)

# Define standard defintions
ivw_define_standard_definitions(inviwo-core inviwo-core)
ivw_define_standard_properties(inviwo-core)

# Special Core Defines
target_compile_definitions(inviwo-core PUBLIC -DCMAKE_BUILD_TYPE=\"$<CONFIG>\")

# Add core folders to includes, this will also be picked up by all the modules
target_include_directories(inviwo-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${IVW_INCLUDE_DIR}>
)

# Add core dependencies to this list
find_package(fmt REQUIRED)
find_package(glm REQUIRED)
find_package(tclap REQUIRED)
find_package(roaring REQUIRED)
find_package(llnl-units REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)
find_package(warn CONFIG REQUIRED)
find_package(md4c CONFIG REQUIRED)
find_package(ticpp CONFIG REQUIRED)
find_package(sml CONFIG REQUIRED)
find_package(flags CONFIG REQUIRED)
find_package(tracywrap CONFIG REQUIRED)
find_package(FastFloat CONFIG REQUIRED)
find_package(curlcpp CONFIG REQUIRED)

if(APPLE)
    find_library(CORESERVICES_LIBRARY CoreServices)
endif()

if(WIN32)
    find_package(stackwalker CONFIG REQUIRED)
endif()

target_compile_definitions(inviwo-core 
    PUBLIC
        GLM_FORCE_SIZE_T_LENGTH
        GLM_ENABLE_EXPERIMENTAL
        GLM_FORCE_UNRESTRICTED_GENTYPE
        GLM_FORCE_SILENT_WARNINGS
        BOOST_SML_CFG_DISABLE_MIN_SIZE
)

target_link_libraries(inviwo-core 
    PUBLIC
        ticpp::ticpp
        glm::glm
        warn::warn
        inviwo::tclap
        sml::sml
        inviwo::flags
        inviwo::tracywrap
        fmt::fmt
        $<$<BOOL:${IVW_ENABLE_OPENMP}>:OpenMP::OpenMP_CXX>
        $<$<BOOL:${UNIX}>:${CMAKE_DL_LIBS}>  # Required for dlopen
        $<$<BOOL:${WIN32}>:unofficial::stackwalker::StackWalker>
        $<$<BOOL:${APPLE}>:${CORESERVICES_LIBRARY}>
        llnl-units::units
        FastFloat::fast_float
        curlcpp::curlcpp
    PRIVATE
        roaring::roaring
        md4c::md4c
        utf8cpp::utf8cpp
)

# Core header files used in PCH file
# These headers where identified by the CompileScore extension in Visual Studio as the ones 
# where most time is spent on (accumulative).
set(PCH_HEADERS_CORE
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwoapplication.h
    ${IVW_INCLUDE_DIR}/inviwo/core/common/inviwomodule.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/data.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/geometry/mesh.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/image/image.h
    ${IVW_INCLUDE_DIR}/inviwo/core/datastructures/volume/volume.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/deserializer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/serializebase.h
    ${IVW_INCLUDE_DIR}/inviwo/core/io/serialization/serializer.h
    ${IVW_INCLUDE_DIR}/inviwo/core/processors/processor.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/property.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/propertyowner.h
    ${IVW_INCLUDE_DIR}/inviwo/core/properties/valuewrapper.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/document.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/exception.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glm.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmmat.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmutils.h
    ${IVW_INCLUDE_DIR}/inviwo/core/util/glmvec.h
)
# Standard headers and external header files used in PCH file
# as identified by CompileScore extension in Visual Studio.
set(PCH_HEADERS_EXTERNAL
    <cassert>
    <cmath>
    <vector>
    <string>
    <string_view>
    <ostream>

    <glm/detail/setup.hpp>
    <fmt/format.h>
)

ivw_compile_optimize_on_target(inviwo-core HEADERS ${PCH_HEADERS_EXTERNAL} ${PCH_HEADERS_CORE})

# Installer setup
install(
    FILES 
        ${IVW_ROOT_DIR}/cmake/compileoptions.cmake
        ${IVW_ROOT_DIR}/cmake/compileresources.cmake
        ${IVW_ROOT_DIR}/cmake/coredata.cmake
        ${IVW_ROOT_DIR}/cmake/createmodule.cmake
        ${IVW_ROOT_DIR}/cmake/doc.cmake
        ${IVW_ROOT_DIR}/cmake/filegeneration.cmake
        ${IVW_ROOT_DIR}/cmake/globalutils.cmake
        ${IVW_ROOT_DIR}/cmake/installutils.cmake
        ${IVW_ROOT_DIR}/cmake/licenses.cmake
        ${IVW_ROOT_DIR}/cmake/moduledata.cmake
        ${IVW_ROOT_DIR}/cmake/precompileheaders.cmake
        ${IVW_ROOT_DIR}/cmake/pybind11.cmake
        ${IVW_ROOT_DIR}/cmake/registermodules.cmake
        ${IVW_ROOT_DIR}/cmake/unittests.cmake
        ${IVW_ROOT_DIR}/cmake/vcpkghelpers.cmake
    DESTINATION ${IVW_INSTALL_PREFIX}cmake
    COMPONENT Development
)
install(DIRECTORY ${IVW_ROOT_DIR}/cmake/templates
    DESTINATION ${IVW_INSTALL_PREFIX}cmake/
    COMPONENT Development
)


set(packages fmt glm tclap roaring llnl-units utf8cpp warn ticpp md4c sml flags tracywrap FastFloat)
if(WIN32)
    list(APPEND packages stackwalker)
endif()

set(preconfig [=[
file(GLOB configs LIST_DIRECTORIES true ${IVW_SHARE_DIR}/*)
list(APPEND CMAKE_PREFIX_PATH ${configs})
]=]
)

string(CONFIGURE [=[
option(IVW_CFG_CMAKE_DEBUG "Print CMake Debug Information" OFF)
function(ivw_debug_message)
    if(IVW_CFG_CMAKE_DEBUG)
        message(${ARGV})
    endif()
endfunction()

macro(ivw_setup)
    set(IVW_VERSION         @IVW_VERSION@)
    set(IVW_ROOT_DIR        ${IVW_PREFIX})
    set(IVW_MODULE_DIR      ${IVW_PREFIX}/modules)
    set(IVW_CMAKE_TEMPLATES ${IVW_PREFIX}/cmake/templates)
    set(IVW_BINARY_DIR      ${CMAKE_CURRENT_BINARY_DIR})

    include(${IVW_PREFIX}/cmake/globalutils.cmake)
    include(${IVW_PREFIX}/cmake/coredata.cmake)
    include(${IVW_PREFIX}/cmake/moduledata.cmake)
    include(${IVW_PREFIX}/cmake/createmodule.cmake)
    include(${IVW_PREFIX}/cmake/unittests.cmake)
    include(${IVW_PREFIX}/cmake/compileoptions.cmake)
    include(${IVW_PREFIX}/cmake/precompileheaders.cmake)
    include(${IVW_PREFIX}/cmake/registermodules.cmake)
    include(${IVW_PREFIX}/cmake/filegeneration.cmake)
    include(${IVW_PREFIX}/cmake/doc.cmake)
    include(${IVW_PREFIX}/cmake/licenses.cmake)
    include(${IVW_PREFIX}/cmake/installutils.cmake)

    set(INVIWOCOREMODULE_name @INVIWOCOREMODULE_name@)
    set(INVIWOCOREMODULE_dir @INVIWOCOREMODULE_dir@)
    set(INVIWOCOREMODULE_base @INVIWOCOREMODULE_base@)
    set(INVIWOCOREMODULE_path @INVIWOCOREMODULE_path@)
    set(INVIWOCOREMODULE_group @INVIWOCOREMODULE_group@)
    set(INVIWOCOREMODULE_opt @INVIWOCOREMODULE_opt@)
    set(INVIWOCOREMODULE_target @INVIWOCOREMODULE_target@)
    set(INVIWOCOREMODULE_alias @INVIWOCOREMODULE_alias@)
    set(INVIWOCOREMODULE_class @INVIWOCOREMODULE_class@)
    set(INVIWOCOREMODULE_modName @INVIWOCOREMODULE_modName@)
    set(INVIWOCOREMODULE_version @INVIWOCOREMODULE_version@)
    set(INVIWOCOREMODULE_dependencies @INVIWOCOREMODULE_dependencies@)
    set(INVIWOCOREMODULE_udependencies @INVIWOCOREMODULE_udependencies@)
    set(INVIWOCOREMODULE_dependenciesversion @INVIWOCOREMODULE_dependenciesversion@)
    set(INVIWOCOREMODULE_protected @INVIWOCOREMODULE_protected@)
    set(INVIWOCOREMODULE_aliases @INVIWOCOREMODULE_aliases@)
endmacro()
]=]
postconfig @ONLY)

ivw_install_helper(
    TARGET inviwo-core
    NAMESPACE inviwo
    DESTINATION inviwo
    ALIAS core
    PACKAGES ${packages}
    PRECONFIG ${preconfig}
    POSTCONFIG ${postconfig}
)

ivw_make_unittest_target(core inviwo-core)

get_property(core_install_list TARGET inviwo-core PROPERTY IVW_INSTALL_LIST)
get_property(ext_install_list DIRECTORY ${IVW_ROOT_DIR}/ext PROPERTY IVW_INSTALL_LIST)
list(APPEND core_install_list ${ext_install_list})
set_property(TARGET inviwo-core PROPERTY IVW_INSTALL_LIST ${core_install_list})

# register license files
ivw_register_license_file(NAME "Inviwo" MODULE Core
    URL http://www.inviwo.org
    VERSION ${IVW_VERSION}
    TYPE "The MIT License"
    FILES ${IVW_ROOT_DIR}/LICENSE
)
get_property(core_license_list TARGET inviwo-core PROPERTY IVW_LICENSE_LIST)
get_property(ext_license_list DIRECTORY ${IVW_ROOT_DIR}/ext PROPERTY IVW_LICENSE_LIST)
get_property(meta_license_list DIRECTORY ${IVW_ROOT_DIR}/tools/meta PROPERTY IVW_LICENSE_LIST)
list(APPEND core_license_list ${ext_license_list} ${meta_license_list})
set_property(TARGET inviwo-core PROPERTY IVW_LICENSE_LIST ${core_license_list})

ivw_register_license_file(NAME "base64" VERSION 2.rc.08 MODULE Core
    URL https://github.com/ReneNyffenegger/cpp-base64
    TYPE "Copyright (C) 2004-2017, 2020, 2021 René Nyffenegger"
    FILES ${IVW_ROOT_DIR}/include/inviwo/core/algorithm/base64_license.txt
)

# Generate build info - the build info will be updated during every compilation
# make a clean help script
set(build_info_script_list
    "include(${IVW_ROOT_DIR}/cmake/globalutils.cmake)"
    "ivw_generate_build_info(${IVW_CMAKE_TEMPLATES}/inviwo_buildinfo_template.cpp"
    "    ${IVW_CMAKE_TEMPLATES}/inviwo_buildinfo_template.ini"
    "    ${CMAKE_CURRENT_BINARY_DIR}/src/util/inviwo_buildinfo.cpp"
    "    inviwo_buildinfo.ini"
    "    ${IVW_MODULE_DIR} ${IVW_EXTERNAL_MODULES})"
)
string(REPLACE ";" "\n" build_info_script "${build_info_script_list}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/build_info.cmake" ${build_info_script})

# Custom target for updating the build information (this target is always out-of-date)
set(buildinfo_output_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<BOOL:${IVW_GENERATOR_IS_MULTI_CONFIG}>:$<CONFIG>/>)
add_custom_target(inviwo-buildinfo-header ALL
    BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/src/util/inviwo_buildinfo.cpp
    COMMAND ${CMAKE_COMMAND} -DINI_DEST_PATH=\"${buildinfo_output_dir}\"
        -P "${CMAKE_CURRENT_BINARY_DIR}/build_info.cmake"
    WORKING_DIRECTORY ${OUTPUT_DIR}
    COMMENT "Generating inviwo build info (buildinfo)"
    VERBATIM
)
set_target_properties(inviwo-buildinfo-header PROPERTIES FOLDER tools)

if(IVW_CFG_EMBED_BUILDINFO)
    add_dependencies("inviwo-core" "inviwo-buildinfo-header")
    target_compile_definitions("inviwo-core" PRIVATE "IVW_CFG_EMBED_BUILDINFO")
else()
    ## ensure that the ini file containing the build info is packaged and installed to the bin folder
    install(
        FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<BOOL:${IVW_GENERATOR_IS_MULTI_CONFIG}>:$<CONFIG>/>inviwo_buildinfo.ini 
        DESTINATION ${IVW_RUNTIME_INSTALL_DIR} 
        COMPONENT Application
    )
endif()

if(IVW_TEST_BENCHMARKS)
    add_subdirectory(tests/benchmarks)
endif()
