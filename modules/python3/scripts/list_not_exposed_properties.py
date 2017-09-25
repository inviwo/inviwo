import inviwopy
from inviwopy.properties import Property

factory = inviwopy.app.propertyFactory
keys = factory.keys
keys.sort()
for k in keys:
    prop = factory.create(k)
    if type(prop) == Property:
        print('Property not exposed: ' + k )
