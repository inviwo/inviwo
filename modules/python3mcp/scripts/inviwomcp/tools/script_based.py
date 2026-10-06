#----------------------------------- Script based approach ----------------------------------------------
from inviwomcp.inviwomcp import *
def register_tool(mcp, app, network, logger):
    
    @mcp.tool()
    async def generate_script(script: str) -> dict:
        """Used to generate python scripts to construct and modify pipelines by inputting a string based script. Do not use this tool unless you have already seen the api documentation.
            You are explicitly forbidden from using any kind of code to alter or delete any content on the disk or to access any other information than what is provided through the documenation.
            You have the following predefined variables to your disposal, app = inviwopy.application, network = app.network.

            The executable code needs to be captured inside a function and must be called on by the following function,
            def call_dispatch():
                try:
                    data = app.dispatch_front(YOUR FUNCTION)
                    loop.call_soon_threadsafe(future.set_result, data)
                except Exception as e:
                    loop.call_soon_threadsafe(future.set_exception, e)

            The final line in the script is the following,
            threading.Thread(target=call_dispatch).start()

            Make sure no unallowed code is used before executing the tool, strictly only allowed to construct, modify and display information about the network and processors, and respective meta data.
        """

        loop = asyncio.get_event_loop()
        future = loop.create_future()

        variables = {
            "app": app,
            "network": network,
            "loop": loop,
            "future": future,
            "threading": threading
        }

        try:
            logger.debug(f"Generated script: {script}")
            exec(script, variables)
            return await future

        except Exception as e:
            return {"error": str(e)}


    @mcp.tool()
    async def prohibited_function(text: str) -> dict:
        """DO NOT USE THIS FUNCTION"""
        #loop = asyncio.get_event_loop()
        #future = loop.create_future()
        logger.debug(f"returning generated script: {text}")
        return {"Script": text}


    @mcp.tool()
    def get_python_api_documentation() -> dict:
        """Gives the full python API documenation"""
        docs_dir = "C:/Users/oskar/Dokument/Skola/Examensarbete/Projekt/inviwopy_docs.json"
        return json.load(open(docs_dir, encoding="utf-8"))


    @mcp.tool()
    def get_python_api_classes() -> list:
        """Gives the python API classes"""
        docs_dir = "C:/Users/oskar/Dokument/Skola/Examensarbete/Projekt/inviwopy_classes_docs.json"
        return json.load(open(docs_dir, encoding="utf-8"))


    @mcp.tool()
    def get_python_api_class_by_name(className: str) ->dict:
        """Gives a python API class from class name, use get_python_api_classes() before calling this function"""
        docs_dir ="C:/Users/oskar/Dokument/Skola/Examensarbete/Projekt/inviwopy_docs.json"
        all_docs = json.load(open(docs_dir, encoding="utf-8"))
        class_doc = all_docs.get(className, {"error": f"Class '{className}' not found"})
        return class_doc


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

             
