var trezor = (function (exports) {

    function bin2hex(bin) {
        var chr, hex = '';
        for (var i = 0; i < bin.length; i++) {
            chr = (bin.charCodeAt(i) & 0xFF).toString(16);
            hex += chr.length < 2 ? '0' + chr : chr;
        }
        return hex;
    }

    exports.bin2hex = bin2hex;

    /**
     * Trezor plugin loader.
     */

    var TrezorLoader = {
        waiting: null,
        trezor: null,
        timeout: null
    };

    TrezorLoader.load = function(success, failure, timeout) {
        if (this.trezor) { // already loaded
            success(this.trezor);
            return;
        }
        if (this.waiting) { // in the process of loading
            failure(new Error('Plugin is already being loaded'));
            return;
        }

        // register the callbacks and load the plugin
        this.waiting = { success: success, failure: failure };
        this.inject('__trezorPluginOnLoad', timeout);
    };

    TrezorLoader.inject = function(callbackName, timeout) {
        // load installed plugins
        navigator.plugins.refresh(false);

        // inject wrapper element
        var body = document.getElementsByTagName('body')[0],
            elem = document.createElement('div');
        body.appendChild(elem);

        // inject <object> with async callback
        elem.innerHTML =
            '<object width="1" height="1" id="__trezor-plugin" '+
            'type="application/x-bitcointrezorplugin">'+
            '<param name="onload" value="'+callbackName+'"></param>'+
            '</object>';

        // register timeout function
        if (timeout) {
            this.timeout = setTimeout(function () {
                TrezorLoader.resolve(null, new Error('Plugin loading timed out'));
            }, timeout);
        }
    };

    TrezorLoader.resolve = function(plugin, error) {
        if (!this.waiting) // not being loaded, bogus call
            return;

        var success = this.waiting.success,
            failure = this.waiting.failure;

        // reset state
        if (this.timeout)
            clearTimeout(this.timeout);
        this.timeout = null;
        this.waiting = null;

        if (plugin && plugin.version !== 'undefined') { // loaded successfully
            this.trezor = new Trezor(plugin);
            if (success)
                success(this.trezor);
        }
        else if (failure) // error occured
            failure(error);
    };

    window.__trezorPluginOnLoad = function () {
        TrezorLoader.resolve(document.getElementById('__trezor-plugin'));
    };

    exports.load = function () {
        TrezorLoader.load.apply(TrezorLoader, arguments);
    };

    /**
     * Trezor plugin.
     */

    var Trezor = function (plugin, url) {
        this._plugin = plugin;
        this._configure(url || Trezor._DEFAULT_URL);
    };

    Trezor._DEFAULT_URL = 'http://localhost:8000/signer/config_signed.bin';

    Trezor.prototype._configure = function (url) {
        var req = new XMLHttpRequest(),
            time = new Date().getTime();

        req.open('get', url + '?' + time, false);
        req.send();

        if (req.status !== 200)
            throw Error('Failed to load configuration');

        this._plugin.configure(req.responseText);
    };

    // Returns the plugin version
    Trezor.prototype.version = function () {
        return this._plugin.version;
    };

    // Returns the list of connected Trezor devices.
    Trezor.prototype.devices = function () {
        return this._plugin.devices;
    };

    // Opens a given device and returns a Session object.
    Trezor.prototype.open = function (device, on) {
        return new Session(device, on);
    };

    /**
     * Trezor device session handle.
     */

    var Session = function (device, on) {
        this._device = device;
        this._on = on || {};
        this.open();
    };

    // Opens the session and acquires the HID device handle.
    Session.prototype.open = function () {
        this._device.open();
    };

    // Closes the session and the HID device.
    Session.prototype.close = function () {
        this._device.close();
    };

    Session.prototype.initialize = function (success, failure) {
        this._call('Initialize', {}, function (t, m) {
            if (t === 'Failure') {
                failure(new Error(m.message));
                return;
            }
            if (t !== 'Features') {
                failure(new Error('Response of unexpected type'));
                return;
            }

            success(m);
        });
    };

    Session.prototype.getEntropy = function (size, success, failure) {
        this._call('GetEntropy', { size: size }, function (t, m) {
            if (t === 'Failure') {
                failure(new Error(m.message));
                return;
            }
            if (t !== 'Entropy') {
                failure(new Error('Response of unexpected type'));
                return;
            }

            success(m.entropy);
        });
    };

    Session.prototype.getAddress = function (address_n, success, failure) {
        this._call('GetAddress', { address_n: address_n }, function (t, m) {
            if (t === 'Failure') {
                failure(new Error(m.message));
                return;
            }
            if (t !== 'Address') {
                failure(new Error('Response of unexpected type'));
                return;
            }

            success(m.address);
        });
    };

    Session.prototype.getMasterPublicKey = function (success, failure) {
        this._call('GetMasterPublicKey', {}, function (t, m) {
            if (t === 'Failure') {
                failure(new Error(m.message));
                return;
            }
            if (t !== 'MasterPublicKey') {
                failure(new Error('Response of unexpected type'));
                return;
            }

            success(m.key);
        });
    };

    Session.prototype.signTx = function (inputs, outputs, success, failure) {
        var self = this,
            signatures = [],
            serializedTx = '';

        this._call('SignTx', { inputs_count: inputs.length,
                               outputs_count: outputs.length }, process);

        function process (t, m) {

            if (t === 'Failure') {
                failure(new Error(m.message));
                return;
            }
            if (t !== 'TxInputRequest') {
                failure(new Error('Response of unexpected type'));
                return;
            }

            if (m.serialized_tx)
                serializedTx += m.serialized_tx;

            if (m.signature && m.signed_index >= 0)
                signatures[m.signed_index] = m.signature;

            if (m.request_index < 0) {
                success(signatures, serializedTx);
                return;
            }

            if (m.request_type == 'TXINPUT')
                self._call('TxInput', inputs[m.request_index], process);
            else
                self._call('TxOutput', outputs[m.request_index], process);
        }
    };

    Session.prototype._log = function () {
        if (!console || !console.log)
            return;
        [].unshift.call(arguments, '[trezor]');
        if (console.log.apply)
            console.log.apply(console, arguments);
        else
            console.log(arguments);
    };

    Session.prototype._call = function (type, msg, callback) {
        var self = this;

        self._log('Sending:', type, msg);

        self._device.call(type, msg, function (err, t, m) {
            if (err) {
                self._log('Received error:', err);
                if (self._on.error)
                    self._on.error(err);
                return;
            }

            self._log('Received:', t, m);

            if (t === 'ButtonRequest') {
                self._call('ButtonAck', {}, callback);
                return;
            }

            if (t === 'PinMatrixRequest') {
                if (self._on.pin)
                    self._on.pin(function (pin) {
                        if (pin)
                            self._call('PinMatrixAck', { pin: pin }, callback);
                        else
                            self._call('PinMatrixCancel', {}, callback);
                    });
                else {
                    self._log('PIN callback not configured, cancelling PIN request');
                    self._call('PinMatrixCancel', {}, callback);
                }
                return;
            }

            callback(t, m);
        });
    };

    return exports;

}({}));
