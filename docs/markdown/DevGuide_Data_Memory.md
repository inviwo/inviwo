# Data Structures
In Inviwo the main core data structures (`Volume`, `Image`, `Layer`, `Mesh`, `Buffer`) use a pattering of handles and representations. The Volume data structure for example has a handle class called Volume. The handle class by itself does not have any data, it only stores metadata, and a list of representations. The representations is where the actual data is stored.

## The different structures
In Inviwo, the `Volume` class wraps volumetric data, more specifically data in a structured grid. This corresponds to what you would expect from a rank 3 tensor (or rank 4 if you have a multi-channel volume).

The `Image` class is used to wrap *Frame Buffers*, which are in essence packages for multiple images with potentially different shapes and data types. The actual single images inside those frame buffers are represented by a `Layer` in Inviwo.
As an effect, most Image ports in Inviwo actually transfer multiple images, or one Frame Buffer, usually consisting of a color layer, depth layer and optionally a picking layer. In this example the `Image` would contain a `UInt8` color layer, a `Float32` depth layer and a `UInt8` picking layer. The picking layer basically encodes object instance IDs in color, so that a lookup in the picking layer gives the object ID for the pixel of interest. This is used for example to drag'n'drop objects in 3D space.
Of course you can add more layers to the images in your custom processors as necessary.

The third big data structure in Inviwo is the `Mesh` and its `Buffer`s, which directly corresponds to the OpenGL concepts.
Thus, you can similarly have buffers for vertex positions, indices, normals, etc. all wrapped in one `Mesh` object.

# Memory Representations

Any data structure can have a set of different representations, for example a `Volume` has a `VolumeRAM` representation and a `VolumeGL` representation. The representation basically determines where the data actually is. In general there are representations for `Disk`, `RAM`, `GL` and if needed also `CL`, where the latter refer to *OpenGL* and *OpenCL* representations respectively.

![The `Volume` handle has multiple `Representations`](images/DataStructures.png)

At any time at least one of the representations should be in a valid state. Whenever we want to access the data of a volume we will ask the `Volume` for a representation of some kind, and the handle is then responsible to try and provide that to us. If the requested representation is valid the handle can just return that representation. Otherwise, it will have to find a valid representation and try to either create the desired representation from that or convert it. (More on conversions below)

When asking a data handle for a representation, you can either ask for a read-only representation using `getRepresentation<>()` or for an editable representation using `getEditableRepresentation<>()`. Note that requesting an editable representation invalidates all other representations in the handle, since they will not receive possible updates and will hence be out of sync.

## Conversion between Representations

Instead of specifying all representations directly for all objects, you can define a `RepresentationConverter`, which can then automatically convert between two representations.
For example a typical use case can be that we start with a `Volume` handle with a `VolumeDisk` representation and we want to do raycasting using OpenGL. In our processor we will then ask the `Volume` for a `VolumeGL` representation. The volume will see that there are currently no such representations. It will then try and find a chain of `RepresentationConverters` to create that representation. In this case that might be a `VolumeDisk2RAMConverter` that will read in the file from `Disk` into `RAM`, and a `VolumeRAM2GLConverter` that will upload the data to the graphics card. The data handle will always try and find the shortest chain of converters. I.e. if there was a `VolumeDisk2GLConverter` that one would have been used instead.
