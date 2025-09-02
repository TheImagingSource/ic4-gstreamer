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

### ic4-gst-helper format-table

List a table showing ic4::Pixelformats names and the correlating GstCaps definition.
