###Overview

This is the source code repository for the book *Real Time 3D Rendering with DirectX and HLSL: A Practical Guide to Graphics Programming* by Paul Varcholik.

[![Real-Time 3D Rendering with DirectX and HLSL](http://www.varcholik.org/RealTime3DRendering/BookCover.jpg)](http://www.informit.com/store/real-time-3d-rendering-with-directx-and-hlsl-a-practical-9780321962720)

There is also a companion site for the book at [http://www.varcholik.org/](http://www.varcholik.org/). There you'll find errata and a forum for discussions and questions about the text.

###Dependencies

Chapters 4 through 9 are intended for use with [NVIDIA FX Composer](https://developer.nvidia.com/fx-composer).

Chapters 10 through 22 require Visual Studio 2013 (any sku, including [Express 2013 for Windows Desktop](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx)).

There is no source for Chapters 1 through 3.

### Attention Windows 8.1 Users

The latest revision of NVIDIA FX Composer (which is used in Chapters 4-9) does not run under Windows 8.1. However, [an older version (2.51.0701.1135)](http://www.softpedia.com/progDownload/NVIDIA-FX-Composer-Download-104791.html) does work. I have tested all of the .fxcproj files included with the source code, they all load correctly and their corresponding shaders run without issue.