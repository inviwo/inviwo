Web browser support based on the Chromium Embedded Framework (CEF https://bitbucket.org/chromiumembedded).
Note: No need to retrieve CEF manually since it will be downloaded and setup automatically.

Usage:
In Inviwo:
Insert a WebBrowser processor in your network and point it to your html-page.
In the html-page:
Include the Inviwo API for communication using:
```
<script src="https://inviwo/modules/webbrowser/data/js/inviwoapi.js"></script>
```
Initialize Inviwo API so that we can use it to synchronize properties
```
<script language="JavaScript">
var inviwo = new InviwoAPI();
</script>
```
Properties can now be set using:
```
inviwo.setProperty('Path.To.Property', {value: 2.0});
```
Properties can be retrieved using:
```
inviwo.getProperty('Path.To.Property', function(prop) {
    for (var key in prop){
        var attrName = key;
        var attrValue = prop[key];
        console.log("Hello " + attrName + ": " + attrValue);
    }
});
```
An example can be seen in data/workspaces/web_property_sync.html
