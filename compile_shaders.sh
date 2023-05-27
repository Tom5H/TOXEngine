#!/bin/sh
/usr/bin/glslc ./Engine/shaders/raytrace.rgen -o ./resources/shaders/raytrace.rgen.spv --target-env=vulkan1.2
/usr/bin/glslc ./Engine/shaders/raytrace.rchit -o ./resources/shaders/raytrace.rchit.spv --target-env=vulkan1.2
/usr/bin/glslc ./Engine/shaders/raytrace.rmiss -o ./resources/shaders/raytrace.rmiss.spv --target-env=vulkan1.2
