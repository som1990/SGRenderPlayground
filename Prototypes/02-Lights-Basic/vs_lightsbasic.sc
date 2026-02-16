$input a_position, a_normal
$output v_world, v_normal, v_view

/*
 * Copyright 2025 Soumitra Goswami. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "../common/common.sh"

void main()
{
   vec4 world = mul(u_model[0], vec4(a_position, 1.0));
   gl_Position = mul(u_viewProj, world);
   v_world = world.xyz;
   vec3 normal = a_normal.xyz*2.0 - 1.0;
   v_normal = mul(u_model[0], vec4(normal, 0.0)).xyz;
   v_view = mul(u_view, world).xyz;
}