
/*
 * API/Object for communicating with Inviwo through Javascript.
 */
class InviwoAPI {
  /*
   * Subscribe to changes of a property. 
   * The supplied input element will automatically be updated when the property changes.
   * @param id HTML element id of the input to update.
   * @param path dot separated path to property, e.g. processor.property_id
   */
  async subscribe(id, path) {
	window.cefQuery({
		request: JSON.stringify({"command":"subscribe", "id": id, "path": path}),
		onSuccess: function(response) {
		},
		onFailure: function(error_code, error_message) {}
		
	});
  }
  /*
   * Set property value(s) 
   * @param path dot separated path to property, e.g. processor.property_id
   * @param payload Value(s) to set in JSON format, e.g. {value: 4} 
   */
  async setProperty(path, payload) {
	window.cefQuery({
		request: JSON.stringify({"command":"property.set", "path": path, ... payload}),
		onSuccess: function(response) {
		},
		onFailure: function(error_code, error_message) {}
		
	});
  }
  /*
   * Get property value(s). Values will be sent to supplied callback.
   * @param path dot separated path to property, e.g. processor.property_id
   * @param cb Callback with a single (JSON) argument 
   */
  async getProperty(path, cb) {
	window.cefQuery({
		request: JSON.stringify({"command":"property.get", "path": path}),
		onSuccess: function(response) {
			cb(response);
		},
		onFailure: function(error_code, error_message) {}
	});
  }
}


