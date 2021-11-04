# PoC

This is a proof of code to demonstrate several implications
of incorrect memory allocation for image handling in
the `pbm` format.
We found this issue to be present in the latest qt6 release,
as well as in previous versions. We only tested Qt 5.15.2
and the latest QT6

The vulnerability is the heap out of bounds read, which allocates
the memory read from the file header.
After reading the data, the wrongly allocated image object
is processed without discarding unused memory.

A file with this header will allocate around 4GB (2^15+2^15 bytes) of memory:

```
P1
32786 32786
```

The next issue is that in certain applications this image file is rendered
and possibly exposed to third parties.
The information rendered in the image are the raw heap bits shown as a monochrome
image.

```
P1
40 150
```

![40_150_heap_pixel.png](40_150_heap_pixel.png)
