# Inviwo Python script 
import inviwopy

inviwopy.app.network.Canvas.snapshot("snapshot.png") 

# this will take a snapshot of the with identifier Canvas,
# or fail with an error if there is no canvas with that
# identifier in the current workspace 
