# UserInterfaceGL Module

This module contains a selection of various processors providing on-screen interfaces for user interactions within an OpenGL canvas. For example, it includes a widget for camera manipulation (Camera Widget), widgets for volume cropping (Cropping Widget), and a presentation mode (Presentation Processor). 
The Presentation Processor can be used, e.g. during demonstrations, as it allows to show images in a PowerPoint fashion instead of its regular image input. 
The [Assimp](http://assimp.sourceforge.net/) library is used for handling the meshes used in the widgets. 

In addition, it provides a graphical user interface based on OpenGL (glui namespace). The UI elements are rendered from textures with additional labels and regular picking is used for triggering events. The glui::Renderer is responsible for caching UI textures and managing shader states. Individual widgets can be combined in layouts. Both touch and mouse interactions are supported.

So far, the following UI elements are supported by glui:

- [X] box layout (vertical and horizontal)
- [X] checkboxes
- [X] buttons and toolbuttons, which feature only an icon
- [X] sliders (vertical and horizontal)
- [X] range sliders (vertical and horizontal)

For all UI elements, matching property widgets exist, i.e. `glui::BoolPropertyWidget`, `glui::ButtonPropertyWidget`, `glui::IntPropertyWidget`, `glui::FloatPropertyWidget`, `glui::IntMinMaxPropertyWidget`, and `glui::FloatMinMaxPropertyWidget`. This allows to link the UI elements with the respective properties.

For examples and usage see below and check out `modules/UserInterfaceGL/processors/GLUITestProcessor.h`. 

### Creating a Plain glui Widget

Creating a glui widget requires a reference to the processor (for invalidation when the UI changes) and a glui::Renderer.
```c++
auto slider = std::make_unique<glui::Slider>("Label of the Slider", 50, 0, 100, 
                                             *this /* refers to a processor */, uiRenderer_, ivec2(100, 24));
// set callback when the slider has changed
slider->setAction(
    [&, p = slider.get() ]() { LogInfo("UI slider changed: " << p->get()); });

// rendering the widget
ivec2 lowerLeft(50, 50);
rangeslider->render(lowerLeft, canvasDims);
```

### Creating a glui Property Widget

Linking regular properties with glui elements is also possible.
```c++
// a regular BoolProperty
auto boolProperty = BoolProperty("boolProperty", "Bool Property", true)  
// matching glui property widget
glui::BoolPropertyWidget boolPropertyUI(boolProperty, *this, uiRenderer_);

// creating a layout
glui::BoxLayout layout(glui::BoxLayout::LayoutDirection::Vertical);
layout.addElement(boolPropertyUI);

// rendering layout in upper right corner of the canvas
const ivec2 extent(layout.getExtent());
const ivec2 outputDim(outport_.getDimensions());
ivec2 origin(ivec2(outputDim.x - extent.x, outputDim.y));

layout.render(origin, outputDim);
```
