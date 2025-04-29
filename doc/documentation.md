# TcamIC4Src

The tcamic4src is a GStreamer source for effortless usage of IC4 in GStreamer.

If you want to use tcamic4src while still using tiscamera:  
tcamsrc automatically detects tcamic4src and integrates it.  
To explicitely select a tcamic4src device use `<SERIAL>-ic4`.


## Dependencies

The GStreamer element requires:

- GStreamer
  Best installed via package manager
- tcam-property
  Downloadable on theimagingsource.com or self built.
  Sources can be found here: https://github.com/TheImagingSource/tiscamera-tcamprop
  
## Gst Properties

To get a full overview over the available element properties, use `gst-inspect`.
The following is a list of custom properties. Standard Gstreamer properties are omitted.

| Name        | Description                                                     | Default | Note      |
|-------------|-----------------------------------------------------------------|---------|-----------|
| num-buffers | Number of buffers to output before sending EOS (-1 = unlimited) | -1      |           |
| serial      | Serial of the camera to be used. Empty opens first camera       | empty   |           |
| type        | backend type. Used only for tiscamera compatibility.            | ic4     | read-only |
|             |                                                                 |         |           |

## Signals

tcamic4src offers the following signals:

```

  "device-open" :  void user_function (GstElement * object,
                                       gpointer user_data);

  "device-close" :  void user_function (GstElement * object,
                                        gpointer user_data);
```

## Usage

tcamic4src is compatible to the IC4 Linux predecessor `tiscamera`.
For examples on how to use the source elements you can look at the examples provided in the tiscamera repository.
https://github.com/TheImagingSource/tiscamera/examples

### Device Listing

tcamic4src implements the GstDevice API.
If you are unfamiliar with the API a good starting point is
https://gstreamer.freedesktop.org/documentation/gstreamer/gstdevicemonitor.html?gi-language=c .

### Caps

#### Listing caps

To see the available GStreamer caps

- Set `serial` to the camera that shall be opened.
- Set ic4src to `GST_STATE_READY`.

This will open the device and allow querying the src pad for available caps.

A caps description for a camera might look like this (captured with `tcam-ctrl -c <SERIAL>`):

```

   video/x-raw,format=GRAY8,framerate=[1/1,130963/1000],width=[256,3072,16],height=[4,2048,4];
   video/x-raw,format=GRAY8,framerate=[1/1,130963/1000],binning=2x2,width=[256,1536,16],height=[4,1024,4];
   video/x-bayer,format=rggb,framerate=[1/1,130963/1000],width=[256,3072,16],height=[4,2048,4];
   video/x-bayer,format=rggb,framerate=[1/1,130963/1000],binning=2x2,width=[256,1536,16],height=[4,1024,4];
   video/x-bayer,format=rggb12p,framerate=[1/1,130963/1000],width=[256,3072,16],height=[4,2048,4];
   video/x-bayer,format=rggb12p,framerate=[1/1,130963/1000],binning=2x2,width=[256,1536,16],height=[4,1024,4];
   video/x-bayer,format=rggb16,framerate=[1/1,130963/1000],width=[256,3072,16],height=[4,2048,4];
   video/x-bayer,format=rggb16,framerate=[1/1,130963/1000],binning=2x2,width=[256,1536,16],height=[4,1024,4];
   video/x-raw,format=BGR,framerate=[1/1,130963/1000],width=[256,3072,16],height=[4,2048,4];
   video/x-raw,format=BGR,framerate=[1/1,130963/1000],binning=2x2,width=[256,1536,16],height=[4,1024,4];
   video/x-raw,format=BGRx,device-format={GRAY8,rggb,rggb12p,rggb16,BGR},framerate=[1/1,130963/1000],width=[256,3072,16],height=[4,2048,4];
   video/x-raw,format=BGRx,device-format={GRAY8,rggb,rggb12p,rggb16,BGR},framerate=[1/1,130963/1000],binning=2x2,width=[256,1536,16],height=[4,1024,4];
   video/x-raw,format=BGRA16_LE,device-format={GRAY8,rggb,rggb12p,rggb16,BGR},framerate=[1/1,130963/1000],width=[256,3072,16],height=[4,2048,4];
   video/x-raw,format=BGRA16_LE,device-format={GRAY8,rggb,rggb12p,rggb16,BGR},framerate=[1/1,130963/1000],binning=2x2,width=[256,1536,16],height=[4,1024,4]

```

`[]` indicates a range [minimum, maximum, step_size].
`{}` indicates a list.

The existence of `device-format` in a GstCaps description indicates, that this is a conversion format.  
These formats do not come from tha camera itself but are a image format conversion applied by IC4.  
The device-format selected will be the format that is set in the camera.

#### Setting caps

Simply set the wanted caps via the `set_caps` function.

To use the IC4 format transformation, define caps as follows:

```
 video/x-raw,format=BGRx,device-format=GRAY8,width=1920,height=1080,framerate=30/1
```

This will:

- set the device `PixelFormat` to `Mono8`
- set the outgoing GStreamer format to BGRa8.

### Properties

tcamic4src implemented the tcam-property interface.
This is the tiscamera GObject property interface.

The tcam-property documentation is part of of tiscamera and can be found here:
https://www.theimagingsource.com/en-us/documentation/tiscamera/tcam_property.html

tcamic4src passes all available properties from IC4 to this interface.
The exceptions are properties that are relevant to the GStreamer caps.

The following properties will not be passed through:

- Width
- Height
- BinningHorizontal
- BinningVertical
- PixelFormat
- AcquisitionFrameRate
- PayloadSize
- AcquisitionStart
- AcquisitionStop
- AcquisitionMode
- TLParamsLocked

### Meta Data

Each image buffer the tcamic4src sends has associated meta data
that contains multiple information concerning the buffer.

The meta object contains a GstStructure which contains all information.
This is to ensure extensibility without interfering with user applications.

The following fields are available:

| fieldname       | type   | description                                |
|-----------------|--------|--------------------------------------------|
| frame_count     | uint64 | number of frames delivered.                |
|                 |        | Starts at 0 with every stream start.       |
| frames_dropped  | uint64 | number of frames dropped by backend        |
| capture_time_ns | uint64 | Timestamp in Nanoseconds                   |
|                 |        | when the backend received the image        |
| camera_time_ns  | uint64 | When the device itself captured the image. |
| is_damaged      | bool   | Not used. tiscamera legacy                 |
|                 |        |                                            |

The point of reference for timestamps is camera dependent.
Please refer to your camera/driver documentation.

Please be aware that not all GStreamer elements correctly pass
GstMeta information through.  
