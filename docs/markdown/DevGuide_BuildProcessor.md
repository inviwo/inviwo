# Build your own Processor
This guide will walk you through the creation of a new processor.

## Necessary files and changes
The easiest way to start creating a new processor in Inviwo is by using Inviwo-meta (TODO: link to meta docs).
In order to create a new processor in Inviwo, you need to create the source files for the processor itself, register it in the according module and add it to the Cmake files.
Inviwo-meta does all of those things for you automatically. The following describes the files that need to be created or modified:
1. You need to create the processors source and header files in `<module-dir>/src/processors/`
2. The processor needs to be registered in its module. To do so, open the `<module-dir>/src/<module-name>module.cpp` and include your processor's header, then add the following line to the module's constructor:
`registerProcessor<MyNewProcessor>();`
3. Add your new processor's header and source to the module's `CMakeLists.txt` (`<module-dir>/CMakeLists.txt`)


## Processor Subclassing
If you have created your new processor using Inviwo-meta, you can see there is already a class definition in your created files.
Your processor needs to be a subclass of `Processor` and it needs to override the `void Processor::process()` method. The `process()` method is where a processors action is implemented.
Additionally, you can override the `const ProcessorInfo getProcessorInfo() const` method which returns meta information about the processor, like a class identifier, name, category, tags etc.
Further your processor needs to define a constructor to initialize all your properties and a destructor (which will usually be the default destructor).

You can then add properties as private attributes in your class. Make sure that you initialize each property in the constructor. We recommend using initializer lists for that.
In the constructor body itself you have to call `addProperty()` or `addProperties()` to add the properties to your processor. This will make sure the properties are displayed in the specified order.

## Inports and Outports
Similar to properties, you can add ports as private members of your class and initialize them in the constructor. Ports also need to be added to the processor using `addPort()`. Note that the convention is to put optional inports to the right of the processor (added last). If your processor performs some kind of rendering, it should usually also include an optional `ImageInport`, where previously rendered parts of an image can be passed through, so that the processor's outport produces a composited image of the current and previous renderings.

## process()
The `process()` function is where the functionality of your processor is defined. Every time the processor is invalidated (either by changing inputs, events or property changes), the `process()` function is executed.
Inside the `process()` you can access your inports' data by using `yourinport.getData()` and similarly you can write data to the outports using `youroutport.setData(...)`. What happens with your data in between is fully up to you.
You can also use all your defined properties here. This let's you access all your algorithm's parameters directly from the GUI with automatic updates upon change.

### Frequently used operations
As mentioned above, most processors that output an image, should also have an optional image inport for compositing of different renders. Usually by default you want to render on top of this incoming image, which basically holds an OpenGL framebuffer. That means you usually want to copy the inport data to the outport and activate it as target framebuffer for your rendering. `utilgl::activateTargetAndClearOrCopySource(outport, image_inport)` does exactly this for you. All your rendering will be automatically composited according to the framebuffer depth layer. Note that it is good practice to deactivate the target at the end of your `process()` by using `utilgl::deactivateCurrentTarget()`.

The `utilgl` namespace includes more useful functions for OpenGL, like controlling various states (depth test, culling, ...), utilities for working with `Shader`s, drawing full screen quads and more.
The `Shader` class can be used to activate shaders and bind uniforms etc. Similarly you can use the `TextureUnit` class as a wrapper for texture units. An image inport can be bound to a texture unit using
```
TextureUnit tex_unit;
utilgl::bindColorTexture(tex_inport, tex_unit.getEnum());
shader.setUniform("Uniform Name", tex_unit.getUnitNumber()); # shader is initialized in constructor
```


## Event Handling and Invalidation
In Inviwo events are propagated upwards in the network until they are consumed by a processor. In practice this usually means the canvas processor starts sending mouse/keyboard input upwards in the network, until a trackball property consumes the event, adjusts the camera properties and thus, invalidates the processors with the camera properties (usually renderers). Of course you can also define other kinds of events which are not related to I/O. You can trigger them anywhere and you can listen to all kinds of events within your processor. In order to subscribe to events, you can use the `EventProperty`. Usually you will want to define a `function(Event*)` in your processor that defines the callback to execute, then you can just pass a lambda to the `EventProperty`'s constructor that calls your callback.

Whenever you initialize a `Property`, you can set an `InvalidationLevel`. The invalidation level is one of `Valid`, `InvalidOutput` or `InvalidResources`. Whenever the property is changed, this sets the invalidation level of its processor to one of:
- The `Valid` level means there is no re-evaluation necessary.
- The `InvalidOutput` level triggers a re-execution of `process()`
- The `InvalidResources` level triggers both `initalizeResources()` and `process()`.

The `initializeResources()` method can be overriden to perform initialization and pre-processing of resources. Inviwo will take care of calling this method whenever the processor was set to this state of having invalid resources.

Note that the re-evaluation only happens after invalidation levels of all processors are determined, so that every processor is only re-evaluated once. Running the `process()` method automatically sets the invalidation level back to `Valid`.
