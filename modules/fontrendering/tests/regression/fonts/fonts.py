import inviwopy
import inviwopy.glm as glm

app = inviwopy.app
network = app.network

app.network.clear()
app.network.lock()

bg = app.processorFactory.create("org.inviwo.Background", glm.ivec2(0, -100))
bg.backgroundStyle.selectedDisplayName = "Uniform color"
bg.bgColor1.value = glm.vec4(1, 1, 1, 1)

canvas = app.processorFactory.create("org.inviwo.CanvasGL", glm.ivec2(0, 0))
canvas.inputSize.dimensions.value = glm.size2_t(256, 600)

network.addProcessor(bg)
network.addProcessor(canvas)

proc = app.processorFactory.create("org.inviwo.TextOverlayGL")

fontNames = proc.font.fontFace.displayNames
fontIdentifiers = proc.font.fontFace.identifiers

prev = bg

fontSize = 16
processorSpacing = 50
# vertical spacing of text
vertTextSpacing = 1.0 / len(fontNames)

## create one text overlay processor for each font face
for i, (name, id) in enumerate(zip(fontNames, fontIdentifiers)):
    p = app.processorFactory.create("org.inviwo.TextOverlayGL", 
                                    glm.ivec2(300, processorSpacing * i))    
    p.displayName = name
    p.text.value = name
    p.color.value = glm.vec4(0, 0, 0, 1)
    p.font.fontFace.selectedIdentifier = id
    p.font.fontSize.value = fontSize
    p.font.anchor.value = glm.vec2(-1, 1)
    p.getPropertyByIdentifier("position").value = glm.vec2(0.01, 1.0 - i * vertTextSpacing)

    network.addProcessor(p)

    if i > 0:  
        # link font size
        network.addLink(prev.font.fontSize, p.font.fontSize)
        network.addLink(p.font.fontSize, prev.font.fontSize)

    network.addConnection(prev.outports[0], p.inports[0])
    prev = p

canvas.position = glm.ivec2(0, processorSpacing * len(fontNames) + 25)

## connect last processor to canvas
network.addConnection(prev.outports[0], canvas.inports[0])

app.network.unlock()
