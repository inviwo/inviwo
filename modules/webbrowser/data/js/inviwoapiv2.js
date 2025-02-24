export function cefAPIRequest(request) {
    return new Promise((resolve, reject) => {
        let request_id = window.cefQuery({
            request: JSON.stringify(request),
            onSuccess: function (response) {
                try {
                    var data = JSON.parse(response);
                } catch (e) {
                    throw Error(`Failed to parse json response from: '${request.command}' got: '${response}'`)
                }
                resolve(data);
            },
            onFailure: function (error_code, error_message) {
                reject({ "error_code": error_code, "error_message": error_message });
            }
        });
    });
}

export function callback(callbackId, jsonArg = "") {
    return cefAPIRequest({
        "command": "callback",
        "callback": callbackId,
        "data": JSON.stringify(jsonArg)
    });
}

export class Property {
    constructor(path, identifier) {
        this.path = path;
        this.identifier = identifier;
    }

    property(propertyId) {
        return new Property(this.path + "." + this.identifier, propertyId);
    }

    set(value) {
        return cefAPIRequest({
            'command': 'property.set',
            'path': this.path + "." + this.identifier,
            'value': value
        });
    }

    get() {
        return cefAPIRequest({
            'command': 'property.get',
            'path': this.path + "." + this.identifier
        });
    }
}

export class Inport {
    constructor(processor, identifier) {
        this.processor = processor;
        this.identifier = identifier;
    }


    getData() {
        return cefAPIRequest({
            'command': 'inport.getData',
            'processor': this.processor,
            'identifier': this.identifier
        }).catch((reason) => {
            throw Error("Failed to get port data: " + JSON.stringify(reason))
        });
    }

    getFilteredIndices(target = "row") {
        return cefAPIRequest({
            'command': 'inport.getFilteredIndices',
            'processor': this.processor,
            'identifier': this.identifier,
            'target': target
        }).catch((reason) => {
            throw Error("Failed to get port indices: " + JSON.stringify(reason))
        });
    }
    getSelectedIndices(target = "row") {
        return cefAPIRequest({
            'command': 'inport.getSelectedIndices',
            'processor': this.processor,
            'identifier': this.identifier,
            'target': target
        }).catch((reason) => {
            throw Error("Failed to get port indices: " + JSON.stringify(reason))
        });
    }
    getHighlightedIndices(target = "row") {
        return cefAPIRequest({
            'command': 'inport.getHighlightedIndices',
            'processor': this.processor,
            'identifier': this.identifier,
            'target': target
        }).catch((reason) => {
            throw Error("Failed to get port indices: " + JSON.stringify(reason))
        });
    }

    filter(indices, target = "row") {
        return cefAPIRequest({
            'command': 'inport.filter',
            'processor': this.processor,
            'identifier': this.identifier,
            'target': target,
            'indices': indices
        })
    }

    select(indices, target = "row") {
        return cefAPIRequest({
            'command': 'inport.select',
            'processor': this.processor,
            'identifier': this.identifier,
            'target': target,
            'indices': indices
        })
    }

    highlight(indices, target = "row") {
        return cefAPIRequest({
            'command': 'inport.highlight',
            'processor': this.processor,
            'identifier': this.identifier,
            'target': target,
            'indices': indices
        });
    }


}

export class Outport {
    constructor(processor, identifier) {
        this.processor = processor;
        this.identifier = identifier;
    }
}

export class Processor {
    constructor(identifier) {
        this.identifier = identifier;
    }

    properties() {
        return cefAPIRequest({
            'command': 'processor.properties',
            'identifier': this.identifier
        }).then((response) => {
            return response.map((propertyId) => {
                return new Property(this.identifier, propertyId);
            });
        });
    }

    property(propertyId) {
        return new Property(this.identifier, propertyId);
    }

    inports() {
        return cefAPIRequest({
            'command': 'processor.inports',
            'identifier': this.identifier
        }).then((response) => {
            return response.map((portId) => {
                return new Inport(this.identifier, portId);
            });
        });
    }

    inport(portId) {
        return new Inport(this.identifier, portId);
    }

    outports() {
        return cefAPIRequest({
            'command': 'processor.outports',
            'identifier': this.identifier
        }).then((response) => {
            return response.map((portId) => {
                return new Outport(this.identifier, portId);
            });
        });
    }
    outport(portId) {
        return new Outport(this.identifier, portId);
    }
}


export class Network {
    processors() {
        return cefAPIRequest({
            'command': 'network.processors'
        }).then((response) => {
            return response.map((identifier) => {
                return new Processor(identifier);
            });
        });
    }

    processor(ientifier) {
        return new Processor(identifier);
    }
}

export var network = new Network();

