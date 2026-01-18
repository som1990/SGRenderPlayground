$input a_position, a_normal
$output v_pos, v_normal, v_view

/*
 * Copyright 2025 Soumitra Goswami. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "../common/common.sh"

void main()
{
   gl_Position = mul(u_modelViewProj, vec4(a_position,1.0));
   v_pos = gl_Position.xyz;
   vec3 normal = a_normal.xyz*2.0 - 1.0;
   v_normal = mul(u_modelView, vec4(normal, 0.0)).xyz;
   v_view = mul(u_modelView, vec4(a_position,1.0)).xyz;
}