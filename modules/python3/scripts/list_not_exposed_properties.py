import inviwopy

factory = inviwopy.app.propertyFactory
keys = factory.keys
keys.sort()
for k in keys:
    prop = factory.create(k)
    if type(prop) == inviwopy.Property:
        print('Property not exposed: ' + k )
