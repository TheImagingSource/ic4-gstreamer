# tcamic4src

## Usage

Ensure `GENICAM_GENTL64_PATH` is set!
`source env.sh` from build dir, for test usage.
gst-launch-1.0 -v tcamic4src serial=40210174

## Requirements

- ic4
- vcpkg
- gstreamer-1.0
- g++/clang

## Building

    cmake --workflow --preset package-linux

## TODO:

### Open

- documentation
- support for formats other than bayer/mono
- caps support for resolution sets (not implemented in ic4)

### Done

- packaging
- GstDevice Handling
- Basic Image retrieval
- Basic caps handling
- Basic property handling
- Device lost
- GstTcamStatistics

## License

To be determined.

