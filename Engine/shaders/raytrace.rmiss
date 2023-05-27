#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable

#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT hitPayload payload;

void main()
{
  payload.emission = vec3(0.7, 0.6, 0.5);
  payload.done = true;
}
