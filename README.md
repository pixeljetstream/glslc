glslc
=====

Simple GLSL compilation checker (uses display driver)

(c) 2013 Christoph Kubisch: pixeljetstream@luxinia.de
http://github.com/CrazyButcher/glslc

Basic offline compiler for GLSL
Creates a dummy window and evokes the GL driver for compiling.
Requires moderately new drivers, GL_ARB_separate_shader_objects
Can dump pseudo assembly files for NVIDIA

Usage:

glslc [options] filename
Mandatory Options:
  -profile profilename
       profilename can be: vertex, fragment, geometry,
       tessevaluation, tesscontrol, compute
Other:
  -o outputfilename
       NVIDIA drivers can output pseudo assembly based on NV_program
  -DMACRO[=VALUE]
       prepends '#define MACRO VALUE' to shader
       If VALUE is not specified it defaults to 1.