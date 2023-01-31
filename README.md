# tcamic4src

## Usage

Ensure `GENICAM_GENTL64_PATH` is set!
`source env.sh` from build dir
gst-launch-1.0 -v tcamic4src serial=40210174

## Requirements

- ic4
- vcpkg
- gstreamer-1.0
- g++/clang

## Building

Building currently requires you to manually point to ic4.

    mkdir build
    cmake -Dic4_DIR=<path to ic4 install/build dir>  -DCMAKE_TOOLCHAIN_FILE=<path to>/vcpkg.cmake ..
    make -j

## TODO:

### Open

- GstTcamStatistics
- documentation
- ic4 lookup
- packaging
- support for formats other than bayer/mono
- caps support for resolution sets (not implemented in ic4)

### Done

- GstDevice Handling
- Basic Image retrieval
- Basic caps handling
- Basic property handling
- Device lost

## License

To be determined.
