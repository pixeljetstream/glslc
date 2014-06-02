glslc
=====

Simple GLSL compilation checker (uses display driver)

(c) 2013-2014 Christoph Kubisch: pixeljetstream@luxinia.de
http://glslc.luxinia.de

* Basic commandline compiler for GLSL
* Creates an invisible dummy window and evokes the GL driver for compiling.
* Can dump pseudo assembly files for NVIDIA
* Basic #include handling independent of GL_ARB_shading_language_include support
* Best used with GLSL editing capabilites of the http://www.luxinia.de/index.php/Estrela or http://studio.zerobrane.com/ IDE

Examples
--------

glslc -o output.txt -profile vertex myvertex.vs

glslc -glslversion "430 core" -o output.txt -DFOO -vertex myvertex.vs -fragment myfrag.fs

glslc -glslversion "430 core" -o output.txt -DFOO -SD_VERTEX_ -vertex myprogram.glsl -SD_FRAGMENT_ -fragment myprogram.glsl

Usage
-----

glslc [options] *filename*

### Mandatory options

-*profilename*

or

-profile *profilename*

> Profilename can be: vertex, fragment, geometry, tessevaluation, tesscontrol, compute
> and must be specified prior each file name.

### Other options:

-glslversion "*string*"

> Prepends "#version *string*\n" prior macros and shader and puts // in front of #version find in shaderfile.

-separable

> Will mark the program separable (default is false) prior attaching and linking, if GL_ARB_separate_shader_objects is supported.

-o *outputfilename*

> NVIDIA drivers can output pseudo assembly file based on NV_program

-DMACRO[=VALUE]
  
> Prepends '#define MACRO VALUE' to all shaders. If VALUE is not specified it defaults to 1.

-SDMACRO[=VALUE]
  
> Prepends '#define MACRO VALUE' to next shader. If VALUE is not specified it defaults to 1. All shader defines are cleared with each shader file.

Building
--------

Currently Windows and Linux are supported. For Visual Studio 2008 a solution exists in /buildvc9. The project is just the src/glsl.cpp compiled as console application.

Thanks to Alan Chambers for Linux support.
