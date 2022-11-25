#Inviwo Python script 
import inviwopy
import rich
import rich.panel
import rich.console
import rich.table

def toText(document):
    stack = [rich.console.Group()]
    def before(elem, s):
        if elem.isText():
            stack.append(elem.content)
        if elem.isNode() and elem.name == "table":
            table = rich.table.Table()
            table.show_header = False
            table.show_edge = False
            table.box = None
            stack.append(table)
            
        if elem.isNode() and elem.name == "tr":
            stack.append([])
        if elem.isNode() and (elem.name == "td" or elem.name == "th"):
            stack.append(rich.console.Group())

    def after(elem, s):
        if elem.isText():
            text = stack.pop()
            stack[-1].renderables.append(text)
        if elem.isNode() and elem.name == "table":
            table = stack.pop()
            stack[-1].renderables.append(table)
        if elem.isNode() and elem.name == "tr":
            row = stack.pop()
            stack[-1].add_row(*row)
    
        if elem.isNode() and (elem.name == "td" or elem.name == "th"):
            group = stack.pop()
            stack[-1].append(group)

    document.visit(before, after)
    return rich.panel.Panel(stack[0])

rich.print(inviwopy.app.network.VolumeSource.filename.getDescription())