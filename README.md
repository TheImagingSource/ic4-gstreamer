# tcamic4src

## Usage

Ensure `GENICAM_GENTL64_PATH` is set!
`source env.sh` from build dir, for test usage.
gst-launch-1.0 -v tcamic4src serial=40210174

To activate conversion by ic4, add `device-format` to the caps.

```
gst-launch-1.0 tcamic4src serial=28710095 ! video/x-raw,format=BGRx,device-format=GRAY8,width=1920,height=1080,framerate=30/1 !  videoconvert ! waylandsink sync=false 
```

## Requirements

- ic4
- vcpkg
- gstreamer-1.0
- g++/clang
- sphinx
- tcam-properties

## Building

cmake --workflow --preset package-linux

## License

To be determined.

