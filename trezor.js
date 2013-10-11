var trezor = (function (exports) {

    /**
     * Trezor plugin.
     */

    var Trezor = function () {
        this._plugin = Trezor._inject();
    };

    Trezor._inject = function () {
        var body = document.getElementsByTagName('body')[0],
            elem = document.createElement('object');

        elem.type = "application/x-bitcointrezorplugin";
        elem.width = 0;
        elem.height = 0;
        body.appendChild(elem);

        return elem;
    };

    Trezor.prototype.version = function () {
        return this._plugin.version;
    };

    Trezor.prototype.devices = function () {
        return this._plugin.devices;
    };

    Trezor.prototype.open = function (device, on) {
        return new Session(device, on);
    };

    exports.Trezor = Trezor;

    /**
     * Trezor device session handle.
     */

    var Session = function (device, on) {
        this._device = device;
        this._on = on || {};
        this.open();
    };

    Session.prototype.open = function () {
        this._device.open();
    };

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
        [].unshift.call(arguments, '[trezor]');
        console.log.apply(console, arguments);
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
