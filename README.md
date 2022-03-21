<p align="center">
  <img src="images/appicon_256.png" width="200px" align="center" />
</p>

[![Build status](https://ci.appveyor.com/api/projects/status/6d1w0d6gw28npxpf?svg=true)](https://ci.appveyor.com/project/HuguesDelorme/mayo)
[![Build Status](https://img.shields.io/travis/fougue/mayo/develop.svg?logo=travis)](https://app.travis-ci.com/fougue/mayo)
[![Downloads](https://img.shields.io/github/downloads/fougue/mayo/total.svg)](https://github.com/fougue/mayo/releases)
[![License](https://img.shields.io/badge/license-BSD%202--clause-blue.svg)](https://github.com/fougue/mayo/blob/develop/LICENSE.txt)


# What is Mayo
Mayo is an opensource 3D CAD viewer and converter

# Overview
* View and convert 3D files in different formats
* Explore assembly trees(product structure) and view properties
* Cross platform: runs on Windows, Linux and macOS
* Underlying toolkits: OpenCascade and Qt

# Current features
* 3D exploding of the model tree, allowing better exploration of complex designs
* 3D clip planes with configurable capping
* 3D view cube providing intuitive camera manipulation
* Save image(snapshot) of the current 3D view
* Quick access to the CAD files recently open thanks to thumbnails in the Home page
* Toggle visibility of any item from the Model tree(use checkbox)
* Customizable precision of the meshes computed from BRep shapes, affecting visualization quality and conversion into mesh formats
* Convert files to multiple CAD formats from command-line interface(CLI)

3D viewer operations :
* Rotate : mouse left + move
* Pan : mouse right + move
* Zoom : mouse wheel(scroll)
* Window zoom : mouse wheel + move
* Instant zoom : space bar
* Select Object: mouse left click
* Select Objects: SHIFT + mouse left clicks

# Supported formats
  Formats                 |  Import   |  Export  | Notes
--------------------------|-----------|----------|------------------------------
STEP                      |  &#10004; | &#10004; | AP203, 214, 242(some parts)
IGES                      |  &#10004; | &#10004; | v5.3
OpenCascade BREP          |  &#10004; | &#10004; |
DXF                       |  &#10004; | &#10060; |
OBJ                       |  &#10004; | &#10004; | Import requires OpenCascade &#8805; v7.4.0<br>Export requires OpenCascade &#8805; v7.6.0
glTF                      |  &#10004; | &#10004; | Import requires OpenCascade &#8805; v7.4.0<br>Export requires OpenCascade &#8805; v7.5.0<br>Supports 1.0, 2.0 and GLB
VRML                      |  &#10060; | &#10004; | v2.0 UTF8
STL                       |  &#10004; | &#10004; | ASCII/binary
AMF                       |  &#10060; | &#10004; | v1.2 Text/ZIP<br>Requires [gmio](https://github.com/fougue/gmio) &#8805; v0.4.0
Image                     |  &#10060; | &#10004; | PNG, JPEG, BMP, GIF, ...<br>See export [options](https://github.com/fougue/mayo/wiki/Export-parameters-by-CAD-format#image)

Mayo provides precise control over [import](https://github.com/fougue/mayo/wiki/Import-parameters-by-CAD-format) and [export](https://github.com/fougue/mayo/wiki/Export-parameters-by-CAD-format) with many parameters per format.

# Gallery

<img src="doc/screencast_1.gif"/>

<img src="doc/screencast_cli.gif"/>

<img src="doc/screenshot_2.png"/>

<img src="doc/screenshot_3.png"/>

<img src="doc/screenshot_4.png"/>

<img src="doc/screenshot_5.png"/>

# How to build Mayo
[Instructions for Windows MSVC](https://github.com/fougue/mayo/wiki/Build-instructions-for-Windows-MSVC)  
[Instructions for Debian](https://github.com/fougue/mayo/wiki/Build-instructions-for-Debian)
