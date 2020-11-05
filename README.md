<p align="center">
  <img src="images/appicon_256.png" width="200px" align="center" />
</p>

[![Build status](https://ci.appveyor.com/api/projects/status/6d1w0d6gw28npxpf?svg=true)](https://ci.appveyor.com/project/HuguesDelorme/mayo)
[![License](https://img.shields.io/badge/license-BSD%202--clause-blue.svg)](https://github.com/fougue/mayo/blob/develop/LICENSE.txt)

# What is Mayo
Mayo is a 3D viewer and converter inspired by FreeCad

# Overview
* View and convert 3D files in different formats
* Explore assembly trees and view properties
* Cross platform: runs on Windows and Linux
* Underlying toolkits: OpenCascade and Qt

# Current features
* Support of multi-documents, user can open many files in the session
* Support of STEP/IGES assemblies(product structure and colors)
* Area and volume properties for meshes and shapes
* Editable name of STEP/IGES entities
* Editable 3D properties of the imported items, eg. material, color, display mode, ...
* 3D clip planes with configurable capping
* 3D view cube providing intuitive camera manipulation
* Perspective/orthographic 3D view projection
* Save image(snapshot) of the current 3D view

3D viewer operations :
* Rotate : mouse left + move
* Pan : mouse right + move
* Zoom : mouse wheel(scroll)
* Window zoom : mouse wheel + move
* Instant zoom : space bar

# Supported formats
  Formats                 |  Import   |  Export  | Notes
--------------------------|-----------|----------|------------------------------
STEP                      |  &#10004; | &#10004; | AP203, 214, 242(some parts)
IGES                      |  &#10004; | &#10004; | v5.3
OpenCascade BREP          |  &#10004; | &#10004; |
OBJ                       |  &#10004; | &#10060; | Requires OpenCascade &#8805; v7.4.0
glTF                      |  &#10004; | &#10060; | Requires OpenCascade &#8805; v7.4.0 (supports 1.0, 2.0 and GLB)
VRML                      |  &#10060; | &#10004; | v2.0 UTF8
STL                       |  &#10004; | &#10004; | ASCII/binary

# Build instructions
Mayo requires Qt5 and OpenCascade &#8805; 7.3.0  
* [Qt installer](https://www.qt.io/download-qt-installer)
* [OpenCascade Download Center](https://old.opencascade.com/content/latest-release)

It uses the `CSF_OCCTIncludePath` and `CSF_OCCTLibPath` environment variables to locate
OpenCascade include and library paths. On Windows these two variables are set by the `env.bat`
script which can be found within OpenCascade's base folder, it is recommended to run this script
and then build Mayo :  
```bash
cd .../mayo
qmake
(n)make
```
In case you don't want to run OpenCascade `env.bat` you can use the `CASCADE_INC_DIR`and `CASCADE_LIB_DIR`
qmake variables instead :  
```bash
qmake "CASCADE_INC_DIR=occ_include_dir" "CASCADE_LIB_DIR=occ_library_dir"
```

# Screenshots

<img src="doc/screenshot_1.png"/>
