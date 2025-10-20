
def structSummary(names : list[str]):
    def summary(valobj, internal_dict):
        try:
            return f"[{', '.join(str(valobj.GetChildMemberWithName(name).GetValue()) for name in names)}]"
        except Exception as e:
            return "Error Formatting: " + str(e)
    return summary

def extractQString(valobj, internal_dict):
    dataPtr = valobj.GetChildMemberWithName("d")
    if not dataPtr: raise RuntimeError("Missing dataptr 'd'")

    sizeObj = dataPtr.GetChildMemberWithName("size")
    if not sizeObj: raise RuntimeError("Missing 'size'")

    size = sizeObj.GetValueAsSigned()
    if size < 0: raise RuntimeError("Negative size")

    charPtr = dataPtr.GetChildMemberWithName("ptr")
    if not charPtr: raise RuntimeError("Missing 'ptr'")

    charArray = charPtr.GetPointeeData(0, size).uint16s
    string = ''.join(chr(c) for c in charArray)

    return [size, string]

def QStringSummary(valobj, internal_dict):
    try:
        size, string = extractQString(valobj, internal_dict)
        return f"size: {size}, str: '{string}'"
    except Exception as e:
        return f"Error Formatting Summary QString {e}"


QSizeSummary = structSummary(["wd", "ht"])
QPointSummary = structSummary(["xp", "yp"])

QSizeFSummary = structSummary(["wd", "ht"])
QPointFSummary = structSummary(["xp", "yp"])

QRectSummary = structSummary(["xp", "yp", "w", "h"])
QRectFSummary = structSummary(["xp", "yp", "w", "h"])


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand('type summary add -w qt -F qt_lldb.QPointSummary "QPoint"')
    debugger.HandleCommand('type summary add -w qt -F qt_lldb.QPointFSummary "QPointF"')

    debugger.HandleCommand('type summary add -w qt -F qt_lldb.QRectSummary "QRect"')
    debugger.HandleCommand('type summary add -w qt -F qt_lldb.QRectFSummary "QRectF"')

    debugger.HandleCommand('type summary add -w qt -F qt_lldb.QSizeSummary "QSize"')
    debugger.HandleCommand('type summary add -w qt -F qt_lldb.QSizeFSummary "QSizeF"')

    debugger.HandleCommand('type summary add -w qt -F qt_lldb.QStringSummary "QString"')









