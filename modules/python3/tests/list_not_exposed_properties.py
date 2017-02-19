import inviwopy

factory = inviwopy.app.propertyFactory
for k in factory.keys:
    prop = factory.create(k)
    if type(prop) == inviwopy.Property:
        print('Property not exposed: ' + k )
