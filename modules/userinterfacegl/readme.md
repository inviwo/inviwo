# UserInterfaceGL Module

This module contains a selection of various processors providing on-screen interfaces for user interactions within an OpenGL canvas. For example, it includes a widget for camera manipulation (Camera Widget), widgets for volume cropping (Cropping Widget), and a presentation mode (Presentation Processor). In addition, it provides a graphical user interface based on OpenGL (glui namespace). The UI elements are rendered from textures with additional labels and regular picking is used for triggering events. The glui::Renderer is responsible for caching UI textures and managing shader states. Individual widgets can be combined in a layout.

So far, the following UI elements are supported by glui:

- [X] box layout (vertical and horizontal)
- [X] checkboxes
- [X] buttons
- [X] sliders
- [ ] range sliders

The Presentation Processor can be used, e.g. during demonstrations, as it allows to show images in a PowerPoint fashion instead of its regular image input.

The [Assimp](http://assimp.sourceforge.net/) library is used for handling the meshes used in the widgets. 
