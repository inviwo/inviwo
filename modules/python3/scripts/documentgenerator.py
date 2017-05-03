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


page = pydoc.html.page(pydoc.describe(inviwopy.qt), pydoc.html.document(inviwopy.glm, 'inviwopy.qt'))
path = inviwopy.app.getPath(inviwopy.PathType.Help , "/inviwopy.qt.html" )
with open(path,'w') as file:
        print(page,file=file)


page = pydoc.html.page(pydoc.describe(inviwopy.data), pydoc.html.document(inviwopy.data, 'inviwopy.data'))
path = inviwopy.app.getPath(inviwopy.PathType.Help , "/inviwopy.data.html" )
with open(path,'w') as file:
        print(page,file=file)

page = pydoc.html.page(pydoc.describe(inviwopy.data.formats), pydoc.html.document(inviwopy.data.formats, 'inviwopy.data.formats'))
path = inviwopy.app.getPath(inviwopy.PathType.Help , "/inviwopy.data.formats.html" )
with open(path,'w') as file:
        print(page,file=file)

        page = pydoc.html.page(pydoc.describe(inviwopy.properties), pydoc.html.document(inviwopy.properties, 'inviwopy.properties'))
path = inviwopy.app.getPath(inviwopy.PathType.Help , "/inviwopy.properties.html" )
with open(path,'w') as file:
        print(page,file=file)
