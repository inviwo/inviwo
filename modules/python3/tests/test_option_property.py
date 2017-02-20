#Inviwo Python script 
import inviwopy


app = inviwopy.app

app.network.clear()

pr= app.processorFactory.create('org.inviwo.Background')
app.network.addProcessor(pr)


p = inviwopy.OptionPropertyInt("test","Test")
p.addOption("a","A",1)
p.addOption("b","B",2)

pr.addProperty(p)