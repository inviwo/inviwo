#Inviwo Python script 
import inviwopy

import pydoc

page = pydoc.html.page(pydoc.describe(inviwopy), pydoc.html.document(inviwopy, 'inviwopy'))
path = inviwopy.getApp().getPath(inviwopy.PathType.Help , "/inviwopy.html" )
with open(path,'w') as file:
        print(page,file=file)