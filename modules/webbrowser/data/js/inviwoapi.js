
/*
 * API/Object for communicating with Inviwo through Javascript.
 */
class InviwoAPI {
  /*
   * Subscribe to changes of a property. 
   * The supplied onChange callback will be called when the property changes.
   * @param id HTML element id of the input to update.
   * @param path dot separated path to property, e.g. processor.property_id
   */
  async subscribe(path, onChange, propertyObserver) {
	var observerName = "";
	if (typeof propertyObserver === 'function') {
		observerName = propertyObserver.name;
	}
	window.cefQuery({
		request: JSON.stringify({"command":"subscribe", "path": path, "onChange": onChange.name, "propertyObserver": observerName}),
		onSuccess: function(response) {
		},
		onFailure: function(error_code, error_message) {}
		
	});
  }
  /*
   * Set property value(s) 
   * @param path dot separated path to property, e.g. processor.property_id
   * @param parameters Value(s) to set in JSON format, e.g. {value: 4} 
   */
  async setProperty(path, parameters, onSuccess = function(response){}, 
                    onFailure = function(error_code, error_message){}) {
	window.cefQuery({
		request: JSON.stringify({"command":"property.set", "path": path, ... parameters}),
		onSuccess: function(response) {
			if(typeof onSuccess === 'function') { 
				onSuccess(response);
			}
		},
		onFailure: function(error_code, error_message) {
			if(typeof onFailure === 'function') { 
				onFailure(error_code, error_message);
			} else {
				console.log("getProperty error (" + error_code + "): " + error_message);
			}
		}
	});
  }
  /*
   * Get property value(s). Values will be sent to supplied onSuccess callback.
   * @param path dot separated path to property, e.g. processor.property_id
   * @param onSuccess Callback with a single (JSON) argument containing the property
   */
  async getProperty(path, onSuccess, 
                    onFailure = function(error_code, error_message){}) {
		window.cefQuery({
			request: JSON.stringify({"command":"property.get", "path": path}),
			onSuccess: function(response) {
				if(typeof onSuccess === 'function') { 
					onSuccess(JSON.parse(response));
				}
			},
			onFailure: function(error_code, error_message) {
				if(typeof onFailure === 'function') { 
					onFailure(error_code, error_message);
				} else {
					console.log("getProperty error (" + error_code + "): " + error_message);
				}
			}
		});
	}
	
	
	async syncRange(htmlId, prop) {
		var property = document.getElementById(htmlId);
		if (property!=null) {
			property.min = prop["minValue"];
			property.max = prop["maxValue"];
			property.step =prop["increment"];
			property.value = prop["value"];
			// Send oninput event to update element
			property.oninput();
		};
	}
	
	async syncCheckbox(htmlId, prop) {
		var property = document.getElementById(htmlId);
		if (property!=null) {
			property.checked = prop["value"];
			// Send oninput event to update element
			property.click();
		};
	}
}


