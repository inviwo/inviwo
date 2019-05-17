# The Inviwo Interface
This part of the Getting Started Guide explains Inviwo's graphical interface and the rough concepts
of its building blocks. The goal of this guide is to give an intuitive feeling for the GUI and enable
users to build workspaces from existing processors.

## GUI Overview
The following image shows Inviwos GUI:
![The Inviwo GUI](images/UI.png)

As you can see, the main area consists of the *Network Editor*,
surrounded by the [*Processor*](#markdown-header-processors) *List* on the left, the selected processor's
[*Properties*](#markdown-header-properties) to the right and the *Debug Console* with log messages below.

You can drag processors from the *Processor List* into the *Network Editor* using drag and drop.
Processors are the basic building blocks of the network and perform one encapsulated action.
Each processor can define its required input and produced output data through [*Ports*](#markdown-header-ports),
which can be connected using the mouse. The resulting network represents an acyclic graph where
the nodes are processors and the edges are port connections.
In addition to inputs and outputs, a processor's [*Properties*](#markdown-header-properties) can be edited
on the right when the processor is selected. Those properties represent the state of the processor and
expose parameters to the user. Changing a property updates the networks results interactively.
Examples for processors include:

- *Source* processors that are mainly used to load data (e.g. Volume Source, Mesh Source, Image Source).
  They have no inports and only output the loaded data through their outports. The according file location
  is set in a property.

- Processors that perform special algorithms (e.g. Volume Raycaster, Mesh Renderer, Image Invert).
  They have inports to receive input data and outports to access the algorithm's result. Their properties
  include lighting options, camera settings, as well as more specific parameters.

- *Sink* processors which only have inports, but no outports. For example the Canvas which displays an
  incoming image and has no outputs.

<details><summary>Pro Tip!</summary>

 - You can re-initialize a processor by dragging the processor from the list on top of the already initialized processor
  in your network.

- Holding `Shift` while dropping the processor in the network will attempt to connect processors above automatically.

- Some properties are linked automatically when initialized, for example the camera. Holding `Alt` while dropping
  the processor will prevent automatic linking. (See [Property Linking](#markdown-header-linking))
</details>

## The Inviwo Network
Inviwo networks are built around a few concepts in order to keep the resulting graphs clean and deterministic.
Overall the networks are built to represent the *Visualization Pipeline*, thus there is a data-flow from the
top to the bottom of the network. There must be no cycles and horizontal connections should be avoided.
In order to keep the application interactive, events (especially Mouse Events from the Canvas) are propagated
upwards in the network until a processor consumes the event. This way the subgraphs, starting from the event
consuming processor downwards, are re-evaluated. For example a Mouse Move Event is propagated upwards until
a rendering processor is reached. This rendering processor usually has a Camera Property that is adjusted from
this event. As a result, the rendering processor is invalidated and produces a new rendered image with the
new camera state. The subsequent processors are then invalidated due to their inport changing, since the
renderer has output a new image. Compare the following figure:
![Inviwo Network Flow](images/NetworkExplained.png)
### Visualization Pipeline
As stated above, the Inviwo network closely resembles the Visualization Pipeline. The following shall clarify which parts of the pipeline are realized through which processors. The pipeline begins with some
input data. Inviwo offers several processors to load data (e.g. VolumeSource, ImageSource, MeshSource) or
create data artificially (VolumeCreator, MeshCreator, VectorFieldGenerator).

The first step in the pipeline is then to **analyze and prepare** the data. That means missing data is filled in, outliers are removed and potential errors are removed from the data (see Image Operations and Volume Operations). For more advanced pre-processing, the PythonScriptProcessor might be an easy and comfortable way to modify the data using *Python 3 and Numpy* (more advanced tutorials on this are coming).

The second step is to **filter** the data to decide which parts of the data are visualized. Inviwo offers
processors like VolumeSubset or VolumeSlice to get sub-volumes or volume slices directly. Many options for filtering are often part of bigger concepts, like *Brushing and Linking*.

Next the data is **mapped**. In InfoVis that refers to mapping the data to geometric primitives (points, lines) and features in the data are mappedto visual attributes (color, size, ...).
TODO: scivis vs infovis hier. beispiele zu beidem in inviwo.
This step has a lot of room for customization and a lot of
different techniques are applied to different kinds of data. In
### Data Flow
flows downwards
### Event Flow
flow upwards

## Processors
![A processor](images/Processor.png)
### Ports
Inport vs. Outport, Types
### Port Inspectors
![Port Inspectors](images/PortInspector.png)

## Properties
### Linking
![Property Linking](images/PropertyLinks.png)
### Semantics
![Vec3 Semantics](images/PropertySemanticsMerged.png)
### Defaults & Ranges
