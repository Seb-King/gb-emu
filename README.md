# gb-emu

## Building

The build process uses CMake. Look here for installation instructions https://cmake.org/install/.

To build run:

```sh
cmake -B build
cmake --build  build -t emulator
```

This will download library files and produce an executable `emulator` in the `./build` folder on MacOS or `./build/build` on Windows.
Point the executable to the path of a gameboy game to play it e.g.

```sh
./build/emulator -r your-game.gb
```

To build for the web using emscripten run

```sh
emcmake cmake . && emmake make
```

Then to serve the game over network locally using

```sh
emrun src/emulator.html
```

## Tests

To run tests first build them with cmake

```sh
cmake -B build
cmake --build  build -t tests
```

Then run the executable:

```sh
./build/tests/tests
```

## How to play

'Z' and 'X' on the keyboard are mapped to the 'A' and 'B' buttons on the gameboy, respectively. The arrow keys are mapped to the d-pad.
