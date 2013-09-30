var trezor = (function (exports) {

    var Trezor = function (plugin) {
        this.plugin = plugin;
    };

    Trezor.prototype.version = function () {
        return this.plugin.version;
    };

    Trezor.prototype.devices = function () {
        return this.plugin.devices;
    };

    Trezor.prototype.open = function (device, on) {
        return new Channel(device, on);
    };

    exports.Trezor = Trezor;

    /**
     * Trezor device handle.
     */

    var Channel = function (device, on) {
        this.device = device;
        this.on = on || {};
    };

    Channel.prototype.getEntropy = function (size, cb) {
        this.call('GetEntropy', { size: size }, function (t, m) {
            if (t !== 'Entropy')
                return cb(new Error('Message of unknown type'));
            return cb(null, m.entropy);
        });
    };

    Channel.prototype.getAddress = function (address_n, cb) {
        this.call('GetAddress', { address_n: address_n }, function (t, m) {
            if (t !== 'Address')
                return cb(new Error('Message of unknown type'));
            return cb(null, m.address);
        });
    };

    Channel.prototype.getMasterPublicKey = function (cb) {
        this.call('GetMasterPublicKey', {}, function (t, m) {
            if (t !== 'MasterPublicKey')
                return cb(new Error('Message of unknown type'));
            return cb(null, m.key);
        });
    };

    Channel.prototype.call = function (type, msg, cb) {
        var self = this;
        this.device.call(type, msg, function (t, m) {
            switch (t) {
            case 'Failure':
                return self.on.failure(m);
            case 'ButtonRequest':
                return self.call('ButtonAck', {}, cb);
            case 'PinMatrixRequest':
                return self.on.pin(m, function (pin) {
                    return self.call('PinMatrix', { pin: pin }, cb);
                });
            default:
                return cb(t, m);
            }
        });
    };

    return exports;

}({}));
