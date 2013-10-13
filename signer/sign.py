#!/usr/bin/python
import subprocess
import os
import json
import time
import ecdsa
import binascii
from google.protobuf.descriptor_pb2 import FileDescriptorSet

TREZOR_PROTO_DIR='../trezor-emu/protobuf/'

def compile_config():
    cmd = "protoc --python_out=. -I/usr/include/ -I../ ../config.proto"
    subprocess.check_call(cmd.split())

def parse_json():
    return json.loads(open('config.json', 'r').read())


def get_compiled_proto():
    # Compile trezor.proto to binary format
    cmd = "protoc -I" + os.path.abspath(TREZOR_PROTO_DIR) + " " +\
          os.path.abspath(TREZOR_PROTO_DIR) + "/trezor.proto -otrezor.bin"

    subprocess.check_call(cmd.split())

    # Load compiled protocol description to string
    proto = open('trezor.bin', 'r').read()
    os.unlink('trezor.bin')

    # Parse it into FileDescriptorSet structure
    compiled = FileDescriptorSet()
    compiled.ParseFromString(proto)
    return compiled

def compose_message(json, proto):
    import config_pb2

    cfg = config_pb2.Configuration()
    cfg.valid_until = int(time.time()) + json['valid_days'] * 3600 * 24
    cfg.wire_protocol.MergeFrom(proto)

    for url in json['whitelist_urls']:
        cfg.whitelist_urls.append(str(url))

    for url in json['blacklist_urls']:
        cfg.blacklist_urls.append(str(url))

    for dev in json['known_devices']:
        desc = cfg.known_devices.add()
        desc.vendor_id = int(dev[0], 16)
        desc.product_id = int(dev[1], 16)

    return cfg.SerializeToString()

def sign_message(data, key_pem):
    # curve = ecdsa.curves.SECP256k1
    # x = ecdsa.keys.SigningKey.generate(curve=curve)
    key = ecdsa.keys.SigningKey.from_pem(key_pem)

    verify = key.get_verifying_key()
    print "Verifying key:", binascii.hexlify(verify.to_string())

    return key.sign_deterministic(data)

def pack_datafile(filename, signature, data):
    if len(signature) != 64:
        raise Exception("Signature must be 64 bytes long")

    fp = open(filename, 'w')
    fp.write(signature)
    fp.write(data)
    fp.close()
    
    print "Signature and data stored to", filename

if __name__ == '__main__':
    key_pem = ''
    print "Paste ECDSA private key (in PEM format) and press Enter:"
    while True:
        inp = raw_input()
        if inp == '':
            break

        key_pem += inp + "\n"

    # key_pem = open('sample.key', 'r').read()

    compile_config()
    json = parse_json()
    proto = get_compiled_proto()

    data = compose_message(json, proto)
    signature = sign_message(data, key_pem)

    pack_datafile('config_signed.bin', signature, data)
