# Build your own Processor
This guide will walk you through the creation of a new processor.

## Necessary files and changes
There are three ways to create a new processor in Inviwo:
1. **Using the GUI**: Just click the `Tools -> Create Sources -> Add Processor` button, navigate to `<module-dir>/src/processors` and specify your processor name in PascalCase (e.g. `MyFancyProcessor`).
2. **Using the Inviwo-meta-cli**: Check out the Inviwo-meta docs (TODO: link)
3. **Manually**:
    In order to create a new processor in Inviwo, you need to create the source files for the processor itself, register it in the according module and add it to the Cmake files.
    Inviwo-meta does all of those things for you automatically. The following describes the files that need to be created or modified:
    1. You need to create the processors source and header files in `<module-dir>/src/processors/`
    2. The processor needs to be registered in its module. To do so, open the `<module-dir>/src/<module-name>module.cpp` and include your processor's header, then add the following line to the module's constructor:
    ```
        registerProcessor<MyNewProcessor>();
    ```
    3. Add your new processor's header and source to the module's `CMakeLists.txt` (`<module-dir>/CMakeLists.txt`)

We suggest using the GUI whenever possible.


## Processor Subclassing
If you have created your new processor using the GUI or Inviwo-meta, you can see there is already a class definition in your created files. If you created the files manually, make sure your class is implemented as follows:

Your processor needs to be a subclass of `Processor` and it needs to override the `void Processor::process()` method. The `process()` method is where a processor's action is implemented.
Additionally, you can override the `const ProcessorInfo getProcessorInfo() const` method which returns meta information about the processor, like a class identifier (unique, in URI style), name, category, tags etc. Make sure to adjust these appropriately.
Further, your processor needs to define a constructor to initialize all your properties and a destructor (which will usually be the default destructor).


Having the rough otuline of the processor set up, you can then add properties and ports as private attributes in your class. Make sure that you initialize each property in the constructor. We recommend using the initializer lists for that.
In the constructor body itself you have to call `addProperty()` or `addProperties()` to add the properties to your processor. This will make sure the properties are displayed in the specified order.
Similarly, you have to add your ports as well.

**Example**:
If you want to add a `float` property and an image inport, you would add the following lines to your processor.
```
// MyProcessor.h
FloatProperty myProp;
ImageInport inport;

// MyProcessor.cpp
MyProcessor::MyProcessor()
    : Processor()
    , inport("imageInport")
    , myProp("myProp", "My Property", 0.5f, 0.0f, 1.0f) {
        addPort(inport);
        addProperty(myProp);
}
```
Note that the constructor of the `FloatProperty` takes an identifier, display name, default, min and max value as parameters. You can then use `myProp` in your code as you would use a normal `float`. The `ImageInport` only requires an identifier.


## Inports and Outports
Similar to properties, you can add ports as private members of your class and initialize them in the constructor. Ports also need to be added to the processor using `addPort()`. Note that the convention is to put optional inports to the right of the processor (added last). If your processor performs some kind of rendering, it should usually also include an optional `ImageInport`, where previously rendered parts of an image can be passed through, so that the processor's outport produces a composited image of the current and previous renderings.

## process()
The `process()` method is where the functionality of your processor is defined. Every time the processor is invalidated (either by changing inputs, events or property changes), the `process()` method is executed.
Inside `process()` you can access your inports' data by using `yourinport.getData()` and similarly you can write data to the outports using `youroutport.setData(...)`. What happens with your data in between is fully up to you.
You can also use all your defined properties here. This let's you access all your algorithm's parameters directly from the GUI with automatic updates upon change.

### Frequently used operations

#### Setting up an `ImageOutport`:
As mentioned above, most processors that output an image should also have an optional image inport for compositing of different renders. Usually by default you want to render on top of this incoming image, which basically holds an OpenGL framebuffer. That means you usually want to copy the inport data to the outport and activate it as target framebuffer for your rendering. `utilgl::activateTargetAndClearOrCopySource(outport, image_inport)` does exactly this for you. All your rendering will be automatically composited according to the framebuffer depth layer. Note that it is good practice to deactivate the target at the end of your `process()` by using `utilgl::deactivateCurrentTarget()`. You can include the function from `modules/opengl/texture/textureutils.h`.

#### Shader and Texture utilities:
The `utilgl` namespace includes more useful functions for OpenGL, like controlling various OpenGL states (see `modules/opengl/openglutils.h`), utilities for working with `Shader`s (see `modules/opengl/shader/shaderutils/h`), drawing full screen quads and more.

The `Shader` class can be used to activate shaders and bind uniforms etc. Similarly you can use the `TextureUnit` class as a wrapper for texture units. An image inport can be bound to a texture unit using
```
TextureUnit tex_unit;
utilgl::bindColorTexture(tex_inport, tex_unit.getEnum());
shader.setUniform("Uniform Name", tex_unit.getUnitNumber()); // shader is initialized in constructor
```

#### Dispatching data to the CPU:
Inviwo has different data representations, depending on where the data is stored (Hard drive, RAM, GPU RAM) and by which API it is used (e.g. OpenGL, OpenCL). You can request each of those representations using `getRepresentation<RepresentationType>()`. For example you could get a volume representation in RAM as follows:
```
auto volume_ram_repr = volumeInport.getData()->getRepresentation<VolumeRAM>();
```
However, you cannot directly use this representation to modify your data. This representation basically holds a `void*` pointer to the raw data. In order to safely cast it to a concrete type (`int`, `float`, `vec3`), so that you can use the data, you have to use `dispatch<>()`.

The `dispatch<>()` method takes a lambda as parameter and the lambda is passed the correctly typed data as parameter. That means you can only access your data directly inside `dispatch<>()`. Furthermore, `dispatch<ReturnType>()` has a type parameter as well, which is the return type of the lambda. See the following example:
```
volume_ram_repr->dispatch<  std::shared_ptr<Volume>  >([] (auto ram) {
    using ValueType = util::PrecisionValueType<decltype(ram)>;
    const size3_t dims = ram->getDimensions();

    dstRam = std::make_shared<VolumeRAMPrecision< typename ValueType> >(dims);
    const ValueType* srcVol = ram->getDataTyped(); // This is always const for data coming from inports!
    ValueType* dstData = dstRam->getDataTyped();

    for (size_t i = 0; i < (dims.x * dims.y * dims.z); ++i) {
        dstData[i] = srcData[i] * ValueType(2);
    }
    return std::make_shared<Volume>(dstRam);
});
```
This piece of code takes the volume from the inport (more specifically its `VolumeRAM` representation) and outputs another `Volume` with all values doubled. There are a few things to note here:
- This works for all possible data types of the volume, it doesn't matter whether it's `float`, `int` or anything else. `ValueType` will be the corresponding type inside `dispatch()`. In order to support all possible types, you have to define the lambda argument with `auto` as is done in the first line. Using this combination of `auto` and the `ValueType` will have your compiler produce `dispatch()` functions for every supported type. Make sure to wrap constants like the `2` inside the `for`-loop in a `ValueType` cast to ensure that its type matches your data's type.
- The template parameter of `dispatch<>()`, in this case `std::shared_ptr<Volume>`, is the return type of the lambda and `dispatch<>()`. This must be the same for all data types.
- All the data that comes from an inport, like the `srcVol` in the example, is `const`. If you want to modify the volume, you will have to create a new one as demonstrated in the example.
- If you need any other variables from outside the scope of the `dispatch<>()`'s lambda, you can pass them into the lambda like so: `dispatch<...>([&my_local_var] (auto ram) { ... })`


## Event Handling and Invalidation
In Inviwo, events are propagated upwards in the network until they are consumed by a processor. In practice this usually means the canvas processor starts sending mouse/keyboard input upwards in the network, until a trackball property consumes the event, adjusts the camera properties and thus, invalidates the processors with the camera properties (usually renderers). Of course you can also define other kinds of events which are not related to I/O. You can trigger them anywhere and you can listen to all kinds of events within your processor. In order to subscribe to events, you can use the `EventProperty`. Usually you will want to define a `function(Event*)` in your processor that defines the callback to execute, then you can just pass a lambda to the `EventProperty`'s constructor that calls your callback.

Whenever you initialize a `Property`, you can set an `InvalidationLevel`. The invalidation level is one of `Valid`, `InvalidOutput` or `InvalidResources`. Whenever the property is changed, this sets the invalidation level of its processor to one of:
- The `Valid` level means there is no re-evaluation necessary.
- The `InvalidOutput` level triggers a re-execution of `process()`
- The `InvalidResources` level triggers both `initalizeResources()` and `process()` (in that order).

The `initializeResources()` method can be overriden to perform initialization and pre-processing of resources. Inviwo will take care of calling this method whenever the processor was set to this state of having invalid resources.

Note that the re-evaluation only happens after invalidation levels of all processors are determined, so that every processor is only re-evaluated once. Running the `process()` method automatically sets the invalidation level back to `Valid`.
