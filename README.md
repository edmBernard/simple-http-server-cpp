# simple-http-server-cpp [![C++ CI](https://github.com/edmBernard/simple-http-server-cpp/workflows/C++%20CI/badge.svg?branch=master)](https://github.com/edmBernard/simple-http-server-cpp/actions)

A simple example of a http server using [uwebsockets](https://github.com/uNetworking/uWebSockets)

## Dependencies

I use [vcpkg](https://github.com/Microsoft/vcpkg) to manage dependencies

Dependencies :
- [cxxopts](https://github.com/jarro2783/cxxopts): Command line argument parsing
- [fmt](https://fmt.dev/latest/index.html): A modern formatting library
- [spdlog](https://github.com/gabime/spdlog): Very fast, header-only/compiled, C++ logging library
- [uwebsockets](https://github.com/uNetworking/uWebSockets): Simple, secure & standards compliant web I/O

They can be installed with
```
./vcpkg install cxxopts fmt spdlog uwebsockets
```

## Compilation

```bash
mkdir build
cd build
# configure make with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake
# on Windows : cmake .. -DCMAKE_TOOLCHAIN_FILE=${env:VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```
