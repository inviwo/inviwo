#-------------------------------------------- Functions defined as tools, mainly for command based interaction ----------------------------------------------
from inviwomcp.inviwomcp import *
APPROACH = "DEFAULT"
def register_tool(mcp, app, network, logger):
    if APPROACH == "DEFAULT":
        @mcp.tool()
        async def network_state() -> dict:
            """Get the network state, list the processors in the network with identifiers and connections and property links"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()
        
            def get_state():
                try:
                    network_dict = {}
                    processors = network.processors

                    for p in processors:
                        processor_connections = []
                        connections = network.connections

                        for c in connections:
                            if c.outport.processor == p or c.inport.processor == p:
                                processor_connections.append({
                                    "from": f"{c.outport.processor.identifier}.{c.outport.identifier}",
                                    "to": f"{c.inport.processor.identifier}.{c.inport.identifier}"})

                        processor_links = []
                        for other in processors:
                            if other == p:
                                continue
                            found = network.getLinksBetweenProcessors(p, other)
                            if found:
                                l = found[0]  # take first, ignore bidirectional duplicate
                                processor_links.append({
                                    "from": f"{p.identifier}.{l.source.identifier}",
                                    "to": f"{other.identifier}.{l.destination.identifier}"
                                })

                        network_dict[p.identifier] = {
                            "classIdentifier": p.classIdentifier,
                            "identifier": p.identifier,
                            "displayName": p.displayName,
                            "connections": processor_connections,
                            "links": processor_links
                        }

                    return {"sucess": True, "network state" : network_dict}

                except Exception as e:
                    return{"success": False, "error": str(e)}


            def call_dispatch():
                data = app.dispatch_front(get_state)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def list_all_processors() -> dict:
            """Lists all processors available in Inviwo, with information including identifiers, descriptions and categories"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def get_processors():
                processor_factory = app.processorFactory
                processor_dict = {}

                for key in processor_factory.keys:
                    if not key.startswith("org.inviwo") or key.endswith("Deprecated"):
                        continue
              
                    try:
                        p = processor_factory.create(key)
                        info = p.getProcessorInfo()
                        if p.category == "Meta":
                            continue

                        processor_dict[p.classIdentifier] = {
                            "classIdentifier": p.classIdentifier,
                            "defaultIdentifier": p.identifier,
                            "displayName": p.displayName,
                            "description": html_to_text(p.getProcessorInfo().help.str()),
                            "category": p.category,
                            "tags": p.tags.getString(),
                            "codeState": p.codeState.name
                        }
                    except Exception as e:
                        processor_dict[p.classIdentifier] = {"classIdentifier": p.classIdentifier, "error": str(e)}

                return {"processors": processor_dict}

            def call_dispatch():
                data = app.dispatch_front(get_processors)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def get_all_processor_categories_command() -> dict:
            """List all processor categories available in Inviwo"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def get_categories():
                processor_factory = app.processorFactory
                categories = set()
                for key in processor_factory.keys:
                    if not key.startswith("org.inviwo") or key.endswith("Deprecated"):
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
        async def get_processors_by_category_command(category: str) -> dict:
            """List all processors from a given category available in Inviwo, can only be used after get_all_processor_categories() has been called"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def get_processors():
                processor_factory = app.processorFactory
                processor_dict = {}
                for key in processor_factory.keys:
                    if not key.startswith("org.inviwo") or key.endswith("Deprecated"):
                        continue
                    try:
                        p = processor_factory.create(key)
                        info = p.getProcessorInfo()
                        if p.category == "Meta":
                            continue

                        if p.category == category:
                            processor_dict[p.classIdentifier] = {
                                "classIdentifier": p.classIdentifier,
                                "defaultIdentifier": p.identifier,
                                "displayName": p.displayName,
                                "description": html_to_text(p.getProcessorInfo().help.str()),
                                "category": p.category,
                                "tags": p.tags.getString(),
                                "codeState": p.codeState.name
                            }
                    except Exception as e:
                        pass

                return {"success": True, "processors": processor_dict}

            def call_dispatch():
                data = app.dispatch_front(get_processors)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future

        @mcp.tool()
        async def get_processor_properties_and_ports_info(classIdentifier: str) -> dict:
            """Get elementary information about properties and ports for a processor that is not currently in the network by classIdentifier.
               Can only be called once get_processors_by_category(category: str) or list_all_processors() has been called"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def get_properties(processor):
                def get_sub_properties(prop):
                    info = {
                        "classIdentifier": prop.classIdentifier,
                        "defaultIdentifier": prop.identifier,
                        "displayName": prop.displayName,
                        "description": html_to_text(prop.help.str()),
                    }

                    if hasattr(prop, "properties") and len(prop.properties) > 0:
                        sub_properties = {}

                        for sub in prop.properties:
                            sub_properties[sub.classIdentifier] = get_sub_properties(sub)

                        info["properties"] = sub_properties

                    return info
                
                properties = {}
                for prop in processor.properties:
                    properties[prop.classIdentifier] = get_sub_properties(prop)

                return properties


            def get_ports(processor):
                inports = {}
                for port in processor.inports:
                    inports[port.classIdentifier] = {
                        "classIdentifier": port.classIdentifier,
                        "defaultIdentifier": port.identifier,
                        "description": extract_help_text(port.contentInfo.str()),
                        "optional": port.optional
                    }

                outports = {}
                for port in processor.outports:
                    outports[port.classIdentifier] = {
                        "classIdentifier": port.classIdentifier,
                        "defaultIdentifier": port.identifier,
                        "description": extract_help_text(port.contentInfo.str())
                    }

                port_info = {
                    "inports": inports,
                    "outports": outports
                }

                return port_info


            def get_info():
                try:
                    p = app.processorFactory.create(classIdentifier)
                    if p is None:
                        return {"success": False, "error": f"Processor '{clasIdentifier}' not found"}

                    processor_dict = {
                    "Properties": get_properties(p),
                    "ports": get_ports(p)
                    }

                    return {"success": True, f"Info for {classIdentifier}": processor_dict}

                except Exception as e:
                    return {"success": False, "error": str(e)}


            def call_dispatch():
                data = app.dispatch_front(get_info)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def suggest_processor_to_connect(classIdentifier: str) -> dict:
            """Suggestions on what processors can be connected to the submitted processors ports"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def get_compatible_processors():
                compatible_inports = {}
                compatible_outports = {}
                processor_factory = app.processorFactory
                sp = processor_factory.create(classIdentifier)

                if sp is None:
                    return {"success": False, "error": f"Processor '{classIdentifier}' not found"}

                for key in processor_factory.keys:
                    if not key.startswith("org.inviwo") or key.endswith("Deprecated"):
                        continue

                    try:
                        cd = processor_factory.create(key)
                        if cd.category == "Meta":
                            continue

                        for selectedInport in sp.inports:
                            if selectedInport.classIdentifier not in compatible_inports:
                                compatible_inports[selectedInport.classIdentifier] = []

                            for candidateOutport in cd.outports:
                                if selectedInport.canConnectTo(candidateOutport):
                                    compatible_inports[selectedInport.classIdentifier].append({
                                        "processor": {"classIdentifier": cd.classIdentifier,
                                        "defaultIdentifier": cd.identifier},
                                        "port": { "classIdentifier": candidateOutport.classIdentifier,
                                            "defaultIdentifier": candidateOutport.identifier}
                                    })

                        for selectedOutport in sp.outports:
                            if selectedOutport.identifier not in compatible_outports:
                                compatible_outports[selectedOutport.identifier] = []

                            for candidateInport in cd.inports:
                                if candidateInport.canConnectTo(selectedOutport):
                                    compatible_outports[selectedOutport.identifier].append({
                                        "processor": cd.classIdentifier,
                                        "port": candidateInport.identifier
                                    })
                                
                    except Exception as e:
                        return {"success": False, "error": str(e)}

                return {
                    "success": True,
                    "processor": sp.classIdentifier,
                    "inports": compatible_inports,
                    "outports": compatible_outports
                }


            def call_dispatch():
                data = app.dispatch_front(get_compatible_processors)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future

        @mcp.tool()
        async def add_processor(classIdentifier: str, identifier: str = "", x_pos: int = 0, y_pos: int = 0) -> dict:
            """Add a processor to the Inviwo network by its classIdentifier, custom identifier can be assigned if seen as fit"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def create_processor():
                try:
                    p = app.processorFactory.create(classIdentifier)
                    if identifier:
                        p.identifier = identifier

                    p.meta.position = inviwopy.glm.ivec2(x_pos, y_pos)
                    app.network.addProcessor(p)
                    return {"success": True, "classIdentifier": classIdentifier, "identifier": p.identifier }

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(create_processor)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def remove_processor(identifier: str) -> dict:
            """Remove a processor based on identifier"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def find_processor():
                try:
                    p = network.getProcessorByIdentifier(identifier)
                    if p is None:
                        return {"success": False, "error": f"Processor '{identifier}' not found"}

                    network.removeProcessor(p)
                    return {"success": True, "Processor removed: ": identifier}

                except Exception as e:
                    return {"success": False, "error: ": str(e)}

            def call_dispatch():
                data = app.dispatch_front(find_processor)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future

        @mcp.tool()
        async def change_position_processor(identifier: str, x_pos: int, y_pos: int) -> dict:
            """Change the position of a processor in the network canvas"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def change_position():
                try:
                    p = network.getProcessorByIdentifier(identifier)
                    if p is None:
                        return {"success": False, "error": f"Processor {identifier} not found"}

                    p.meta.position = inviwopy.glm.ivec2(x_pos, y_pos)
                    return {"success": True, "Processor position changed: ": identifier}

                except Exception as e:
                    return {"success": False, "error: ": str(e)}

            def call_dispatch():
                data = app.dispatch_front(change_position)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def add_connection(sourceIdentifier: str, sourcePort: str, destIdentifier: str, destPort: str) -> dict:
            """Connect an outport of one processor to an inport of another"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def connect():
                try:
                    sourceProcessor = network.getProcessorByIdentifier(sourceIdentifier)
                    if not sourceProcessor:
                        return {"success": False, "error": f"Processor {sourceIdentifier} not found"}
                    destProcessor = network.getProcessorByIdentifier(destIdentifier)
                    if not destProcessor:
                        return {"success": False, "error": f"Processor {destIdentifier} not found"}
                    outport = sourceProcessor.getOutport(sourcePort)
                    if not outport:
                        return {"success": False, "error": f"Outport {sourcePort} not found on {sourceIdentifier}"}
                    inport = destProcessor.getInport(destPort)
                    if not inport:
                        return {"success": False, "error": f"Inport '{destPort}' not found on {destIdentifier}"}

                    network.addConnection(outport, inport)

                    connected = (inport in outport.getConnectedInports()) and (outport in inport.getConnectedOutports())
                    if connected:
                        return {
                            "success": True,
                            "connection": {
                                "from": f"{sourceIdentifier}.{sourcePort}",
                                "to": f"{destIdentifier}.{destPort}"
                            }
                        }
                    else:
                        return {"success": False, "error": f"Could not connect {sourceIdentifier}.{sourcePort} to {destIdentifier}.{destPort}"}

                    #return {"success": False, "error": f"unable to connect {sourceIdentifier}.{sourcePort} and {destIdentifier}.{destPort}"}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(connect)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def remove_connection(sourceIdentifier: str, sourcePort: str, destIdentifier: str, destPort: str) -> dict:
            """Remove a connection between two processors"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def disconnect():
                try:
                    sourceProcessor = network.getProcessorByIdentifier(sourceIdentifier)
                    if not sourceProcessor:
                        return {"success": False, "error": f"Processor {sourceIdentifier} not found"}
                    destProcessor = network.getProcessorByIdentifier(destIdentifier)
                    if not destProcessor:
                        return {"success": False, "error": f"Processor {destIdentifier} not found"}
                    outport = sourceProcessor.getOutport(sourcePort)
                    if not outport:
                        return {"success": False, "error": f"Outport {sourcePort} not found on {sourceIdentifier}"}
                    inport = destProcessor.getInport(destPort)
                    if not inport:
                        return {"success": False, "error": f"Inport '{destPort}' not found on {destIdentifier}"}

                    network.removeConnection(outport, inport)

                    connected = (inport in outport.getConnectedInports()) and (outport in inport.getConnectedOutports())
                    if not connected:
                        return {
                            "success": True,
                            "connection removed": {
                                "from": f"{sourceIdentifier}.{sourcePort}",
                                "to": f"{destIdentifier}.{destPort}"
                            }
                        }
                    else:
                        return {"success": False, "error": f"Could not disconnect {sourceIdentifier}.{sourcePort} to {destIdentifier}.{destPort}"}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(disconnect)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def add_link(processorIdentifier0: str, propertyIdentifier0: str, processorIdentifier1: str, propertyIdentifier1: str) -> dict:
            """Add a bidirectional link between two properties of different processors by their identifiers"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def link():
                try:
                    processor0 = network.getProcessorByIdentifier(processorIdentifier0)
                    if not processor0:
                        return {"success": False, "error": f"Processor {processorIdentifier0} not found"}

                    processor1 = network.getProcessorByIdentifier(processorIdentifier1)
                    if not processor1:
                        return {"success": False, "error": f"Processor {processorIdentifier1} not found"}

                    property0 = processor0.getPropertyByIdentifier(propertyIdentifier0)
                    if not property0:
                        return {"success": False, "error": f"Property {propertyIdentifier0} not found on {processorIdentifier0}"}

                    property1 = processor1.getPropertyByIdentifier(propertyIdentifier1)
                    if not property1:
                        return {"success": False, "error": f"Property {propertyIdentifier1} not found on {processorIdentifier1}"}

                    network.addLink(property0, property1)
                    network.addLink(property1, property0)

                    linked = network.isLinkedBidirectional(property0, property1)

                    if linked:
                        return {
                            "success": True,
                            "link": {
                                "from": f"{processorIdentifier0}.{propertyIdentifier0}",
                                "to": f"{processorIdentifier1}.{propertyIdentifier1}"
                            }
                        }
                    else:
                        return {"success": False, "error": f"Could not link {processorIdentifier0}.{propertyIdentifier0} to {processorIdentifier1}.{propertyIdentifier1}"}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(link)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def remove_link(processorIdentifier0: str, propertyIdentifier0: str, processorIdentifier1: str, propertyIdentifier1: str) -> dict:
            """Remove a bidirectional link between two properties of different processors by their identifiers"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def unlink():
                try:
                    processor0 = network.getProcessorByIdentifier(processorIdentifier0)
                    if not processor0:
                        return {"success": False, "error": f"Processor {processorIdentifier0} not found"}

                    processor1 = network.getProcessorByIdentifier(processorIdentifier1)
                    if not processor1:
                        return {"success": False, "error": f"Processor {processorIdentifier1} not found"}

                    prop0 = processor0.getPropertyByIdentifier(propertyIdentifier0)
                    if not prop0:
                        return {"success": False, "error": f"Property {propertyIdentifier0} not found on {processorIdentifier0}"}

                    prop1 = processor1.getPropertyByIdentifier(propertyIdentifier1)
                    if not prop1:
                        return {"success": False, "error": f"Property {propertyIdentifier1} not found on {processorIdentifier1}"}

                    network.removeLink(prop0, prop1)
                    network.removeLink(prop1, prop0)

                    linked = network.isLinkedBidirectional(prop0, prop1)

                    if not linked:
                        return {
                            "success": True,
                            "link removed": {
                                "from": f"{processorIdentifier0}.{propertyIdentifier0}",
                                "to": f"{processorIdentifier1}.{propertyIdentifier1}"
                            }
                        }
                    else:
                        return {"success": False, "error": f"Could not remove link between {processorIdentifier0}.{propertyIdentifier0} and {processorIdentifier1}.{propertyIdentifier1}"}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(unlink)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future

    
        @mcp.tool()
        async def get_network_processor_ports_info(identifier: str) -> dict:
            """Get a processors port information such as identifiers of inports, outports, whether they are connected"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def get_info():
                try:
                    p = network.getProcessorByIdentifier(identifier)
                    if p is None:
                        return {"success": False, "error": f"Processor '{identifier}' not found"}

                    inports = {}
                    for port in p.inports:
                        connected_outports = []

                        for co in port.getConnectedOutports():
                            connected_outports.append({
                                "processor": co.processor.identifier,
                                "port": co.identifier})

                        inports[port.identifier] = {
                            "identifier": port.identifier,
                            "classIdentifier": port.classIdentifier,
                            "description": extract_help_text(port.contentInfo.str()),
                            "connected": port.isConnected(),
                            "optional": port.optional,
                            "connections": connected_outports
                        }

                    outports = {}
                    for port in p.outports:
                        connected_inports = []

                        for ci in port.getConnectedInports():
                            connected_inports.append({
                                "processor": ci.processor.identifier,
                                "port": ci.identifier})

                        outports[port.identifier] = {
                            "identifier": port.identifier,
                            "classIdentifier": port.classIdentifier,
                            "description": extract_help_text(port.contentInfo.str()),
                            "connected": port.isConnected(),
                            "connections": connected_inports
                        }

                    port_info = {
                        "inports": inports,
                        "outports": outports
                    }

                    return {"success": True, f"Port info for {p.identifier}": port_info}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(get_info)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def get_network_processor_properties(identifier: str) -> dict:
            """Get all properties of a processor"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def get_properties(prop):
                info = {
                    "identifier": prop.identifier,
                    "classIdentifier": prop.classIdentifier,
                    "displayName": prop.displayName,
                    "description": html_to_text(prop.help.str()), 
                }

                try:
                    info["configurable"] = inviwopy.json.toJson(prop)
                except Exception as e:
                    #info["json_error"] = str(e)
                    pass

                if hasattr(prop, "properties") and len(prop.properties) > 0:
                    sub_properties = {}

                    for sub in prop.properties:
                        sub_properties[sub.identifier] = get_properties(sub)

                    info["properties"] = sub_properties

                return info

            def get_info():
                try:
                    p = network.getProcessorByIdentifier(identifier)
                    if p is None:
                        return {"success": False, "error": f"Processor '{identifier}' not found"}

                    properties = {}

                    for prop in p.properties:
                        properties[prop.identifier] = get_properties(prop)

                    processor_info = {
                        "identifier": p.identifier,
                        "classIdentifier": p.classIdentifier,
                        "displayName": p.displayName,
                        "properties": properties
                    }

                    return {"success": True, "processor": processor_info}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(get_info)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def set_processor_property(processorIdentifier: str, propertyIdentifier: str, value: str) -> dict:
            """Set a property value on a processor, if property is a drowdown menu selection, value is the index of the wanted property selection. If property is transfer function or isovalues, use TF tools"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def set_info():
                try:
                    prop = network.getProperty(f"{processorIdentifier}.{propertyIdentifier}")
                    #prop = network.getPropertyByIdentifier(propertyIdentifier)
                    if prop is None:
                        return {"success": False, "error": f"Property {propertyIdentifier} not found"}

                    parsed = json.loads(value)

                    if hasattr(prop, "selectedIndex"):
                        prop.selectedIndex = int(parsed)

                    elif isinstance(parsed, list):
                        prop.value = type(prop.value)(*parsed)

                    else:
                        prop.value = type(prop.value)(parsed)

                    return {"success": True, f"{propertyIdentifier}": f"value changed to {value}"}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(set_info)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def get_tf_points(processorIdentifier: str, propertyIdentifier: str, target: str = "tf") -> dict:
            """Get transfer function or isovalues control points. target: 'tf' or 'isovalues'"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def get_tf():
                try:
                    prop = network.getProperty(f"{processorIdentifier}.{propertyIdentifier}")
                    if not prop:
                        return {"success": False, "error": f"Property {propertyIdentifier} not found on {processorIdentifier}"}

                    if hasattr(prop, target):
                        prop = getattr(prop, target)

                    points = []
                    for i, pt in enumerate(prop.getValues()):
                        points.append({
                            "index": i,
                            "pos": pt.pos,
                            "r": pt.color.x,
                            "g": pt.color.y,
                            "b": pt.color.z,
                            "a": pt.color.w
                        })

                    return {"success": True, "points": points}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(get_tf)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def add_tf_point(processorIdentifier: str, propertyIdentifier: str, pos: float, r: float, g: float, b: float, a: float, target: str = "tf") -> dict:
            """Add a control point to a transfer function or isovalues property. target: 'tf' or 'isovalues'"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def add_point():
                try:
                    prop = network.getProperty(f"{processorIdentifier}.{propertyIdentifier}")
                    if not prop:
                        return {"success": False, "error": f"Property {propertyIdentifier} not found on {processorIdentifier}"}

                    if hasattr(prop, target):
                        prop = getattr(prop, target)

                    prop.add(pos, inviwopy.glm.vec4(r, g, b, a))
                    return {"success": True, "added": {"pos": pos, "r": r, "g": g, "b": b, "a": a}}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(add_point)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def remove_tf_point(processorIdentifier: str, propertyIdentifier: str, index: int, target: str = "tf") -> dict:
            """Remove a transfer function or isovalues control point by index. target: 'tf' or 'isovalues'"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def remove_point():
                try:
                    prop = network.getProperty(f"{processorIdentifier}.{propertyIdentifier}")
                    if not prop:
                        return {"success": False, "error": f"Property {propertyIdentifier} not found on {processorIdentifier}"}

                    if hasattr(prop, target):
                        prop = getattr(prop, target)

                    points = prop.getValues()
                    if index < 0 or index >= len(points):
                        return {"success": False, "error": f"Index {index} out of range, have {len(points)} points"}

                    remaining = [p for i, p in enumerate(points) if i != index]
                    prop.setValues(remaining)
                    return {"success": True, "removed_index": index}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(remove_point)
                loop.call_soon_threadsafe(future.set_result, data)

            threading.Thread(target=call_dispatch).start()
            return await future


        @mcp.tool()
        async def update_tf_point(processorIdentifier: str, propertyIdentifier: str, index: int, pos: float, r: float, g: float, b: float, a: float, target: str = "tf") -> dict:
            """Update an existing transfer function or isovalues control point by index. target: 'tf' or 'isovalues'"""
            loop = asyncio.get_event_loop()
            future = loop.create_future()

            def update_point():
                try:
                    prop = network.getProperty(f"{processorIdentifier}.{propertyIdentifier}")
                    if not prop:
                        return {"success": False, "error": f"Property {propertyIdentifier} not found on {processorIdentifier}"}

                    if hasattr(prop, target):
                        prop = getattr(prop, target)

                    points = prop.getValues()
                    if index < 0 or index >= len(points):
                        return {"success": False, "error": f"Index {index} out of range, have {len(points)} points"}

                    # rebuild list replacing the point at index with new values
                    updated = [
                        inviwopy.data.TFPrimitiveData(pos, inviwopy.glm.vec4(r, g, b, a)) if i == index else p
                        for i, p in enumerate(points)
                    ]
                    prop.setValues(updated)
                    return {"success": True, "updated_index": index, "pos": pos, "r": r, "g": g, "b": b, "a": a}

                except Exception as e:
                    return {"success": False, "error": str(e)}

            def call_dispatch():
                data = app.dispatch_front(update_point)
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


        #@mcp.tool()
        #async def test_isotf_dir() -> dict:
        #   """Test iso and transfer function dir"""
        #    loop = asyncio.get_event_loop()
        #    future = loop.create_future()

        #    def test():
        #        try:
        #            p = network.getProcessorByIdentifier("StandardVolumeRaycaster")
                    #prop1 = network.getProperty("StandardVolumeRaycaster.isotf.tf") EMPTY
                    #prop = network.getProperty("StandardVolumeRaycaster.isotf.isovalues")


                    #return {
                    #    "success": True,
                    #    "dir tf": [x for x in dir(prop1) if not x.startswith('_')],
                    #    "dir isovalues": [x for x in dir(prop2) if not x.startswith('_')]
                    #}
                
                    #prop = network.getProperty("StandardVolumeRaycaster.isotf")
                    #return {
                    #    "tf_type": str(type(prop.tf)),
                    #    "tf_dir": [x for x in dir(prop.tf) if not x.startswith('_')],
                    #    "isovalues_type": str(type(prop.isovalues))
                    #}

                    #prop = network.getProperty("StandardVolumeRaycaster.isotf")
                    #vals = prop.tf.getValues()
                    #if vals:
                        #pt = vals[0]
                        #return {
                        #    "type": str(type(pt)),
                        #    "dir": [x for x in dir(pt) if not x.startswith('_')],
                        #    "repr": str(pt)
                        #}
                    #return {"count": 0}

                #except Exception as e:
                #    return {"success": False, "error": str(e)}

            #def call_dispatch():
            #    data = app.dispatch_front(test)
            #    loop.call_soon_threadsafe(future.set_result, data)

            #threading.Thread(target=call_dispatch).start()

            #return await future
            

        #@mcp.tool()
        #async def test2() -> dict:
        #    """Test Inviwo module exposure and serialization."""

        #    loop = asyncio.get_event_loop()
        #    future = loop.create_future()

        #    def test():
        #        try:
        #            p = network.getProcessorByIdentifier("HeightFieldRender")
                    #return{"success": True, "processor info": [x for x in dir(p) if not x.startswith('_')]}
                    #info = p.getProcessorInfo()
                    #h = info.help

                    #prop = p.getPropertyByIdentifier("heightScale")
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
                    #    "processor": [x for x in dir(p) if not x.startswith('_')]}
                        #"canConnectTo": port.canConnectTo(),
                    #return {
                    #    "success": True,
                    #    "contentInfo": extract_help_text(port.contentInfo.str())
                    #    #"identifier": port.identifier
                    #}
                #except Exception as e:
                    #return {"success": False, "error": str(e)}

            #def call_dispatch():
             #   data = app.dispatch_front(test)
              #  loop.call_soon_threadsafe(future.set_result, data)

            #threading.Thread(target=call_dispatch).start()

            #return await future



    elif APPROACH == "ASSISTED":
        @mcp.tool()
        async def test2() -> dict:
            """Test Inviwo module exposure and serialization."""
            return{"success": True}


    # To be made later, alternatively done outside of the agent
    #@mcp.tool()
    #async def validate_pipeline() -> dict:
       # """Used for evalution of the generated or modified pipeline. May only be called once when the pipeline is deemed finished by the agent"""

    
#---------------------------- Helper Functions ------------------------------------------
def html_to_text(html):
        return re.sub(r'<[^>]+>', '', html).strip()

def extract_help_text(html):
    # Extract content from the help div  where info is stored
    match = re.search(r"<div class='help'><div><p>(.*?)</p></div></div>", html, re.DOTALL)
    if match:
        return match.group(1).strip()
    # Strip all tags
    return re.sub(r'<[^>]+>', '', html).strip()
