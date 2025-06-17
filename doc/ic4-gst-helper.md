# ic4-gst-helper

`ic4-gst-helper` is a commandline tool that offers convenience function
to aid with application development.


## Functionality

### ic4-gst-helper --help/-h

List all available commands.

### ic4-gst-helper devices

List all devices ic4src offers.  
This is always identical to the ic4 device list.

### ic4-gst-helper caps <serial>

List the GStreamer caps of the device with <serial>.

For an explanation on how to read the output,  
refer to the `Caps` chapter in ic4src.md in this directory.

### ic4-gst-helper transform

Inspect the caps of a transformation element.

Available options are:
| Name          | Needed Value               | Description                                     |
|---------------|----------------------------|-------------------------------------------------|
| -e, --element | Name of element to inspect | Which element to analyze. Default: videoconvert |
| --in          | GstCaps string             | Caps that go into the element                   |
| --out         | GstCaps string             | Caps that come out of the element               |
| -h, --help    | -                          | Help text                                       |
|               |                            |                                                 |

Examples:

#### print all available transformation

The following command will print all
transformation caps (in and out) bayer2rgb has to offer:

```
ic4-gst-helper transform --element bayer2rgb
```

Shortened output:

```
Element converts INCOMING caps as follows:

video/x-bayer,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1],format=bggr
        video/x-raw,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1]

[...]

video/x-bayer,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1],format=bggr10le
        video/x-raw,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1]

[...]

Element expects input for caps COMING OUT as follows:

video/x-raw,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1],format=RGBx
        video/x-bayer,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1]

video/x-raw,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1],format=xRGB
        video/x-bayer,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1]

[...]

```

#### Query available output caps

To get a list of caps that an element can offer for given input caps, execute:

```
ic4-gst-helper transform --in
```

#### Query available input caps

To get a list of caps that an element can offer for given output caps, execute:

```
ic4-gst-helper transform --out
```
