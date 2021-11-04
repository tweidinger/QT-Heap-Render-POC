# PoC

This is a proof of concept to demonstrate several implications
of incorrect memory allocation for image handling in
the `pbm` and `xbm` format.

We found this issue to be present in Qt 5.15.2
and the latest QT6 available (6.2.1-2).
Most likely other versions are affected as well.


The first vulnerability is the heap out of bounds read, which allocates
the memory read from the `xbm` or `pbm` file header.
For this poc a crafted `.pbm` file is passed to the QImage object
and then rendered in an application.
After reading the real image data, the previously allocated QImage object
is processed without discarding unused memory.

A file with this header will allocate around 4GB ((2^15)*(2^15)-2 bytes) of memory:

```
P1
32767 32767
```

The next issue is that in certain applications this image file is rendered
and possibly exposed to third parties.
The information rendered in the image are the raw heap bits shown as a monochrome
image.

![heap_sample_image.jpg](heap_sample_image.jpg)

### Issue 1, read of uninitialized memory
Specially crafted pbm/pgm/ppm (raw/ascii) images, that contain less pixel data than indicated by the pixel dimensions in the file header, lead to a valid QImage object containing uninitialized heap data. We suspect the bug to be in the following code location and should be easily fixable by a) initializing allocated data with zeros and/or b) rejecting pbm/pgm/ppm images with less pixel data than expected.
[https://github.com/qt/qtbase/blob/c0a8cfe1677f55daec4bc8626aced41c7ebeb1c4/src/gui/image/qppmhandler.cpp#L168]

### Issue 2, incorrect pbm file parsing
Even correct pbm (ascii) images are parsed and rendered incorrectly. We suspect the bug to be at the following code location.
[https://github.com/qt/qtbase/blob/c0a8cfe1677f55daec4bc8626aced41c7ebeb1c4/src/gui/image/qppmhandler.cpp#L239]
