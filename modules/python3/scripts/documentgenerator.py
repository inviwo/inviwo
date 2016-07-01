import inviwopy
import pydoc

page = pydoc.html.page(pydoc.describe(inviwopy), pydoc.html.document(inviwopy, 'inviwopy'))
path = inviwopy.app.getPath(inviwopy.PathType.Help , "/inviwopy.html" )
with open(path,'w') as file:
        print(page,file=file)



page = pydoc.html.page(pydoc.describe(inviwopy.glm), pydoc.html.document(inviwopy.glm, 'inviwopy.glm'))
path = inviwopy.app.getPath(inviwopy.PathType.Help , "/inviwopy.glm.html" )
with open(path,'w') as file:
        print(page,file=file)