# libdht

This library provides easy sampling of humidity and temperature for DHT11/22.
It is written in C++20 and uses CMake as the buildsystem generator.

## Why?

The currently most used library for interfacing with the DHT sensor is through
Adafruit's CircuitPython. CircuitPython uses libgpiod under the hood which
spinlocks on one CPU, regardless of sampling speed.

Instead of having a walltime of ~50ms per sample every 2s, the walltime is
~1.98s instead effectively hogging one entire CPU for no reason at all which is
not ideal in low power devices.

## Usage

TODO

## Building

Building through CMake:

```bash
$ mkdir bin
$ cd bin
$ cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
$ cmake --build . --target dht22
```

Use the Ninja generator `-G Ninja` in the first cmake step for parallel
compilation.

## Installing

TODO
