glslc
=====

Simple GLSL compilation checker (uses display driver)

(c) 2013 Christoph Kubisch: pixeljetstream@luxinia.de
http://github.com/CrazyButcher/glslc

Basic offline compiler for GLSL
Creates a dummy window and evokes the GL driver for compiling.
Can dump pseudo assembly files for NVIDIA

Usage
-----

glslc [options] *filename*

### Mandatory options

-profile *profilename*

> Profilename can be: vertex, fragment, geometry, tessevaluation, tesscontrol, compute

### Other options:

-notseparable

> Will not mark the program separable (default is false if GL_ARB_separate_shader_objects is supported) prior attaching and linking.

-o *outputfilename*

> NVIDIA drivers can output pseudo assembly file based on NV_program

-DMACRO[=VALUE]
  
> Prepends '#define MACRO VALUE' to shader. If VALUE is not specified it defaults to 1.

-IPATTERN

> Includes files with the given wildcard pattern (e.g. -I*.h -Icommon/*.h) for use with GL_ARB_shading_language_include

