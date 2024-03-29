# PoC

This is a Proof of Concept to demonstrate several implications
of incorrect memory allocation for image handling in
the `pbm`, `ppm` and `xbm` format of the *QT* framework.

*QT* is a cross-platform application development platform mostly written in C++
and therefore inherits possible memory (un)safety related bugs.
A lot of commonly found desktop applications use *QT* and the update
cycle of these applications is ususally lagging behind the upstream
available version of *QT*.

So please check your application and confirm
you are using the latest patched versions of *QT*.

We found these issues to be present in Qt `5.15.2`
and the latest *QT*6 available `6.2.1-2` at the time of discovery (2021).
Most likely older versions are affected as well but we did not confirm this.

## Read of Uninitialized Memory

The vulnerability is a heap out of bounds read, which allocates
the memory boundaries defined in the *raw* or *ascii* `xbm`, `ppm` or `pbm` file header.

For this PoC a crafted `.pbm` file is passed to the `QImage` object,
where the header indicates a larger pixel dimension than existing in
the file itself. The reader process does not stop at the end of the data
and continues to read from the uninitialized heap.

The function [`read_pbm_body`](https://github.com/qt/qtbase/blob/1a63409579ff0e9ce524c09701c1ef8bd2d99f25/src/gui/image/qppmhandler.cpp#L135)
uses the *width* `w` and *height* `h` parameter, which were constructed directly from the file header without validation of actual size.

The `h` parameter is then used inside the function to incorrectly limit (`y < h`) the [allocation loop](https://github.com/qt/qtbase/blob/c0a8cfe1677f55daec4bc8626aced41c7ebeb1c4/src/gui/image/qppmhandler.cpp#L239)
where the heap data is passed into the image.

A very simple application to reproduce and to showcase the required primitives was created. 
This was also passed to the QT maintainer for reproduction:

`main.cpp`

```c
#include <QtWidgets/QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLabel *label = new QLabel("yeeee");
    QPixmap *pixmap = new QPixmap("pbm.pbm");
    
    label->setPixmap(*pixmap);
    label->show();

    return app.exec();
}
```

A file with this header will allocate around 4GB ((2^15)*(2^15)-2 bytes) of memory:

`pbm.pbm`

```
P1
32767 32767
```

Whoever can control the content of the image file `pbm.pbm`, has the ability to cause a *Denial-of-Service* at the rendering
party and in case the image is uploaded or exposed to other parties than the executing party,
it can be classified as an *Information Leak*.

> Real world example 1: Create ephemeral `Teamspeak` channels on a server as a guest with this image set in the description. Each channel will cause around 4GB memory allocation at every user on the server.

> Real world example 2: Visit a malicous website with an older version of `qutebrowser` which uses non-default flags to enable parsing these quite old image formats. The site then proceeds to extract your heap via javascript.

The information rendered in the image are the raw heap bits shown as a monochrome
image. These can be reversed by converting the pixel data into binary format. Afterwards secrets or other information leaks can be searched.

![heap_sample_image.jpg](heap_sample_image.jpg)

Another related vulnerability, which was present at the time of investigation
was that even correct `pbm` (in ascii mode) images were parsed and rendered incorrectly, also leaking heap data.

The issues seemed to be introduced in this [commit](https://github.com/qt/qtbase/commit/1a63409579ff0e9ce524c09701c1ef8bd2d99f25/src/gui/image/qppmhandler.cpp) several years ago.
The issues were fixed in this [commit](https://github.com/qt/qtbase/commit/997c052db9e2bef47cf8217c1537a99c2f086858) cherry picked to `6.2 6.2.2 5.15 5.12 5.12.12` and this [commit](https://github.com/qt/qtbase/commit/8ce36938569841020daf9dc23e41438b06e0ee53) cherry picked to `6.2 6.2.2` and no CVE or security release was assigned by the *QT* project, which is the reason we waited a long time for public disclosure.

The responsible *QT* maintainer took swift action to fix the underlying issue but it
seems like the coordination and process accidentially dropped the public
disclosure from the *QT* side.

Multiple other CVEs and security issues were publicly disclosed in *QT*-5/6 in the meantime. We carried on with our lives and multiple
patched releases happened, so we feel it is fine to publish this issue now.

Some applications seem to have not yet upgraded to the latest *QT* versions available,
so we hope this publication will lead to attention by the developers
using outdated *QT* versions.

The issue was easy to exploit and so intuitive that we had a lot of fun
figuring out how the heap layout was constructed. By filling small or
big size heap buckets we could visually observe changes in the resulting images. We recommend others to use this in-the-wild issue
to give students or beginners without deep programming or exploitation
knowledge a visual insight into how heap is rendered and could be leaked.

Thanks @flipnut for the fun time :wave:
