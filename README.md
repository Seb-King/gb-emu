# gb-emu

## Building

The build process uses CMake. Look here for installation instructions https://cmake.org/install/.

```sh
cmake -B build
cmake --build  build -t emulator
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
