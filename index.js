var ffi = require('ffi');
var ref = require('ref')
var ArrayType = require('ref-array')

var int = ref.types.int
var IntArray = ArrayType(int)
var send = ffi.Library('./send',{'send':['void',[IntArray, int, int, int, int, int]]})

var switches = [12345, 23456]
var iterations = 3

var pin = 17
var pulseLength = 190
var bitLength = 24

send.send(switches, switches.length, iterations, pin, pulseLength, bitLength)
