##########
tcamic4src
##########

Documentation for the Imaging Control 4 GStreamer source by `The Imaging Source`.

If assistance is required at any point, please :ref:`contact our support <contact>`. 


Runtime Requirements
====================

`tcamic4src` requires the following packages to work:

- ic4 SDK
- tcam-property helper library
- At least one GenTL provider for TIS cameras

All of the packages above can be found here:  
https://www.theimagingsource.com/de-de/support/download/

Usage
=====

`tcamic4src` is a tiscamera compatible GStreamer source based on `Imaging Control 4`.

To get an overview over available gstreamer properties, execute `gst-inspect tcamic4src`.

| If you want to use tcamic4src while still using tiscamera:  
| tcamsrc automatically detects tcamic4src and integrates it.  
| To explicitely select a tcamic4src device use `<SERIAL>-ic4`.

| The tcam-property documentation is part of of tiscamera and can be found here:
| https://www.theimagingsource.com/en-us/documentation/tiscamera/tcam_property.html


Caps
----
A caps description for a camera might look like this (captured with `tcam-ctrl -c <SERIAL>`):

.. code-block::

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

| `[]` indicates a range [minimum, maximum, step_size].
| `{}` indicates a list.

   
| The existence of `device-format` in a GstCaps description indicates, that this is a conversion format.
| These formats do not come from tha camera itself but are a image format conversion applied by IC4.
| The device-format selected will be the format that is set in the camera.
   

The following IC4 properties are not available when using tcam-property, as they are part of GStreamer caps handling:

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


Overview
--------

.. toctree::
   :maxdepth: 3

   contact.rst

