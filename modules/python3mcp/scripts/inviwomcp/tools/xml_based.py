#--------------------------------------- XML based approach ------------------------------------------
from inviwomcp.inviwomcp import *
def register_tool(mcp, app, network, logger):

    @mcp.tool()
    async def xml_state() -> dict:
        """Get the current Inviwo network state as xml"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def set_serialization_mode(prop, mode):
            prop.serializationMode = mode
            if hasattr(prop, "properties"):
                for sub in prop.properties:
                    set_serialization_mode(sub, mode)

        def serialize_properties():
            for p in network.processors:
                for prop in p.properties:
                    set_serialization_mode(prop, inviwopy.properties.PropertySerializationMode.All)

        def deserialize_properties():
            for p in network.processors:
                for prop in p.properties:
                    set_serialization_mode(prop, inviwopy.properties.PropertySerializationMode.Default)

        def get_xml():
            try:
                serialize_properties()
                s = inviwopy.Serializer("state")
                network.serialize(s)
                xml_str  = s.write(format=True).splitlines()
                deserialize_properties()

                return {"success": True, "xml network state": xml_str}

            except Exception as e:
                return {"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(get_xml)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()

        return await future


    @mcp.tool()
    async def update_xml(xml_str: str) -> dict:
        """Apply XML network to Inviwo. xml_str is used to update the current network. Minimum requirement for a network is as following:
        <?xml version="1.0" encoding="UTF-8" ?>
        <InviwoWorkspace version="3">
            <ProcessorNetworkVersion content="21"/>
            <Processors/>
            <Connections/>
            <PropertyLinks/>
        </InviwoWorkspace>
        Connections and PropertyLinks are defined as singular instances with src = srcProcessor.srcPort and dst = dstProcessor.dstPort or replace port with property for link
        """
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def set_xml():
            try:
                d = inviwopy.Deserializer.createWorkspaceDeserializer(xml_str)
                network.deserialize(d)

                return {"success": True, "Network": xml_str}

            except Exception as e:
                return {"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(set_xml)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()

        return await future


    @mcp.tool()
    async def get_all_xml_processors() -> dict:
        """List all processors available in xml format"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def set_serialization_mode(prop, mode):
            prop.serializationMode = mode
            if hasattr(prop, "properties"):
                for sub in prop.properties:
                    set_serialization_mode(sub, mode)

        def get_processors():
            xml_processors = {}
            try:
                processor_factory = app.processorFactory
                for key in processor_factory.keys:
                    if not key.startswith("org.inviwo"):
                        continue

                    p = processor_factory.create(key)

                    if not key.startswith("org.inviwo") or key.endswith("Deprecated"):
                        continue

                    for prop in p.properties:
                        set_serialization_mode(prop, inviwopy.properties.PropertySerializationMode.All)

                    s = inviwopy.Serializer("processor")
                    p.serialize(s)
                    xml = s.write(format=True)
                    xml_processors[key] = xml

                return {"success": True, "processors": xml_processors}

            except Exception as e:
                return {"success": True, "error": str(e)}

    
        def call_dispatch():
            data = app.dispatch_front(get_processors)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()

        return await future

    mcp.tool()
    async def get_all_processor_categories_xml() -> dict:
        """List all processor categories available in Inviwo"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def get_categories():
            processor_factory = app.processorFactory
            categories = set()
            for key in processor_factory.keys:
                if not key.startswith("org.inviwo"):
                    continue
                try:
                    p = processor_factory.create(key)
                    info = p.getProcessorInfo()
                    if p.category == "Meta":
                        continue
                    categories.add(p.category)

                except Exception as e:
                    pass

            return {"categories": list(categories)}

        def call_dispatch():
            data = app.dispatch_front(get_categories)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def get_processors_by_category_xml(category: str) -> dict:
        """List all processors in a category available in Inviwo"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def set_serialization_mode(prop, mode):
            prop.serializationMode = mode
            if hasattr(prop, "properties"):
                for sub in prop.properties:
                    set_serialization_mode(sub, mode)

        def get_processors():
            xml_processors = {}
            processor_factory = app.processorFactory
           
            for key in processor_factory.keys:
                if not key.startswith("org.inviwo"):
                    continue
                try:
                    p = processor_factory.create(key)
                    info = p.getProcessorInfo()
                    if p.category == "Meta":
                        continue

                    if p.category == category:
                        for prop in p.properties:
                            set_serialization_mode(prop, inviwopy.properties.PropertySerializationMode.All)

                        s = inviwopy.Serializer("processor")
                        p.serialize(s)
                        xml = s.write(format=True)
                        xml_processors[key] = xml

            
                except Exception as e:
                    pass

            return {"success": True, "processors": xml_processors}

        def call_dispatch():
            data = app.dispatch_front(get_processors)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def get_canvas_image(canvasIdentifier: str, fileName: str) -> Image:
        """Get an snapshot image of the canvas processor, can only be used if an canvas processor exist in the network"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()
        
        def get_snapshot():
            try:
                canvas = network.getProcessorByIdentifier(canvasIdentifier)
                if not canvas:
                    return f"Processor '{canvasIdentifier}' not found"

                canvas.snapshot(f"{fileName}.png")
                return Image(path=f"{fileName}.png")

            except Exception as e:
                return str(e)


        def call_dispatch():
            data = app.dispatch_front(get_snapshot)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future

    #----------------------- TEST -----------------------------------------
    """@mcp.tool()
    async def test2() -> dict:
            Test Inviwo module exposure and serialization.

            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def test():
                try:
                    p = network.getProcessorByIdentifier("HeightFieldRender")
                    port = p.getOutport("image")
                    #return{"success": True, "processor info": [x for x in dir(p) if not x.startswith('_')]}
                    #info = p.getProcessorInfo()
                    #h = info.help

                    s = inviwopy.Serializer("processor")
                    #p.serialize(s)
                    port.serialize(s)
                    xml = s.write(format=True)
                

                    prop = p.getPropertyByIdentifier("heightScale")
                    #p.properties

                    #port  = p.getInport("geometry")
                    #port = p.getOutport("image")

        
                    #return{"success": True, "processor info": f"{type(info)}.{dir(info)}"}
                    #return{"success": True,
                    #    "help_type": str(type(h)), 
                    #    "help_dir": str(dir(h)), 
                    #    "help_str": str(h)}
                    #return {
                    #    "success": True,
                    #    "help_str": html_to_text(h.str())
                    #}
                    #return{"info prop": [x for x in dir(prop) if not x.startswith('_')]}
                    #return {
                    #    "success": True,
                    #    "help_str": prop.help.str(),
                    #    "getHelp_str": prop.getHelp().str(),
                    #    "getDescription_str": prop.getDescription().str()
                    #}
                    #return{"success": True, "inport dir": [x for x in dir(port) if not x.startswith('_')],
                        #"processor": [x for x in dir(p) if not x.startswith('_')]}
                        #"canConnectTo": port.canConnectTo(),
                    #return {
                    #    "success": True,
                    #    "contentInfo": extract_help_text(port.contentInfo.str())
                    #    #"identifier": port.identifier
                    #}
                
                except Exception as e:
                    return {"success": False, "error": str(e)}

                return {"processor xml": xml}

            def call_dispatch():
                data = app.dispatch_front(test)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()

            return await future """


#----------------------- Helper functions -----------------------------
def split_processors(xml_str: str) -> list[str]:
    """Extract each <Processor>...</Processor> block as a separate string"""
    pattern = re.compile(r"<Processor\b.*?</Processor>", re.DOTALL)
    return pattern.findall(xml_str)

