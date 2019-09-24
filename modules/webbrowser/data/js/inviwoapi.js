
/*
 * API/Object for communicating with Inviwo through Javascript.
 * Include the API using:
 * ```
 * <script src="https://inviwo/modules/webbrowser/data/js/inviwoapi.js"></script>
 * ```
 * If you only need one-way synchronization, e.g. always changing values from the
 * webpage:
 * 1. Use inviwo.getProperty to retrieve and set the initial value in the webpage
 * 2. Use inviwo.setProperty to set the property from the webpage
 *
 * If a property may change from other sources than your webpage (e.g. the Inviwo network editor):
 * 1. Use inviwo.subscribe to recieve callbacks when the property changes in Inviwo
 * 2. Use inviwo.setProperty to set the property from the webpage
 */
class InviwoAPI {
    /*
     * Subscribe to changes of a property.
     * The supplied onChange callback will be called when the property changes.
     * The callbacks must be declared in global scope.
     * @param path dot separated path to property, e.g. processor.property_id
     * @param onChange callback that will be called when a property changes in inviwo.
     *                 Callback argument will contain JSON-encoded property parameters.
     * @param propertyObserver callback that will be called on PropertyObserver notifications in
     *     inviwo.
     *                         Exmples of notifications: onSetReadOnly, onSetDisplayName.
     *                         Callback argument will contain JSON-encoded property parameters.
     */
    async subscribe(path, onChange, propertyObserver) {
        var observerName = '';
        if (typeof propertyObserver === 'function') {
            observerName = propertyObserver.name;
        }
        window.cefQuery({
            request: JSON.stringify({
                'command': 'subscribe',
                'path': path,
                'onChange': onChange.name,
                'propertyObserver': observerName
            }),
            onSuccess: function(response) {},
            onFailure: function(error_code, error_message) {}
        });
    }
    /*
     * Set property value(s)
     * @param path dot separated path to property, e.g. processor.property_id
     * @param parameters Value(s) to set in JSON format, e.g. {value: 4}
     */
    async setProperty(
        path, parameters, onSuccess = function(response) {},
        onFailure = function(error_code, error_message) {}) {
        window.cefQuery({
            request:
                JSON.stringify({'command': 'property.set', 'path': path, 'parameters': parameters}),
            onSuccess: function(response) {
                if (typeof onSuccess === 'function') {
                    onSuccess(response);
                }
            },
            onFailure: function(error_code, error_message) {
                if (typeof onFailure === 'function') {
                    onFailure(error_code, error_message);
                } else {
                    console.log('getProperty error (' + error_code + '): ' + error_message);
                }
            }
        });
    }
    /*
     * Get property value(s). Values will be sent to supplied onSuccess callback.
     * @param path dot separated path to property, e.g. processor.property_id
     * @param onSuccess Callback with a single (JSON) argument containing the property
     */
    async getProperty(path, onSuccess, onFailure = function(error_code, error_message) {}) {
        window.cefQuery({
            request: JSON.stringify({'command': 'property.get', 'path': path}),
            onSuccess: function(response) {
                if (typeof onSuccess === 'function') {
                    onSuccess(JSON.parse(response));
                }
            },
            onFailure: function(error_code, error_message) {
                if (typeof onFailure === 'function') {
                    onFailure(error_code, error_message);
                } else {
                    console.log('getProperty error (' + error_code + '): ' + error_message);
                }
            }
        });
    }

    /*
     * Subscribe to progress changes of a processor.
     * The supplied onChange callback will be called when the property changes.
     * The callbacks must be declared in global scope.
     * @param path - Processor identifier
     * @param onProgressChange callback that will be called when progress changes in inviwo.
     *                 Callback argument will contain new progress as parameter.
     * @param onVisibleChange callback that will be called when progress bar visibility changes in inviwo.
     *                 Callback argument will contain new visibility (boolean) as parameter.
     */
    async subscribeProcessorProgress(path, onProgressChange, onVisibleChange) {
        window.cefQuery({
            request: JSON.stringify({
                'command': 'processor.progress.subscribe',
                'path': path,
                'onProgressChange': onProgressChange.name,
                'onVisibleChange': onVisibleChange.name
            }),
            onSuccess: function(response) {},
            onFailure: function(error_code, error_message) {}
        });
    }

    async syncRange(htmlId, prop) {
        var property = document.getElementById(htmlId);
        if (property != null) {
            property.min = prop['minValue'];
            property.max = prop['maxValue'];
            property.step = prop['increment'];
            property.value = prop['value'];
        };
    }

    async syncCheckbox(htmlId, prop) {
        var property = document.getElementById(htmlId);
        if (property != null) {
            property.checked = prop['value'];
        };
    }

    async syncStringInput(htmlId, prop) {
        var property = document.getElementById(htmlId);
        if (property != null) {
            property.value = prop['value'];
        };
    }
    async syncOption(htmlId, prop) {
        var property = document.getElementById(htmlId);
        var newOptions = prop['options'];
        for (var i in newOptions) {
            var option = newOptions[i];
            if (property.options.length <= i) {
                property.options.add(document.createElement('option'));
            }
            var optionElem = property.options[i];
            optionElem.id = option['id'];
            optionElem.text = option['name'];
            optionElem.value = option['value'];

            if (i == prop['selectedIndex']) {
                optionElem.selected = true;
            }
        }
        // Remove leftover properties
        for (i = newOptions.length; i < property.length; i++) {
            property.remove(i);
        }
    }
}
