# Mayo
Mayo is a basic 3D viewer inspired by FreeCad.  

Current features are :
* Multi-documents support, user can open many parts in the session
* Support of IGES/STEP/BRep formats for import/export operations
* Support of STEP/IGES assemblies (colors and tree structure)
* Support of STL format with either OpenCascade or [gmio](https://github.com/fougue/gmio) (optional)
* 3D clip planes with configurable capping
* Save image(snapshot) of the current 3D view
* Editable 3D properties of the imported items, eg. material, color, display mode, ...
* Area and volume properties for meshes and shapes

# Build instructions
Mayo requires Qt5 and OpenCascade-7.2.0.  
Although only tested with VC++/Windows it should build fine on Linux and MacOS.  
`cd .../mayo`  
`qmake "CASCADE_INC_DIR=occ_include_dir" "CASCADE_LIB_DIR=occ_library_dir"`  
`(n)make`  
To enable optional gmio library, add this option to the qmake command line:  
`"GMIO_ROOT=path_to_gmio"`

# Screencast

<img src="doc/screencast.gif"/>
