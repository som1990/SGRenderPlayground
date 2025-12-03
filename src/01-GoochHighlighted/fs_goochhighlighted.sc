$input v_pos, v_normal, v_view

/*
 * Copyright 2025 Soumitra Goswami. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "../common/common.sh"
#include "uniforms.sh"

vec3 gooch_highlighted(vec3 _colorCool, vec3 _colorWarm, vec3 _colorHighlight, float _kDiff, float _kSpec )
{
    // Lerp is not available in GLSL . Use mix instead
    return mix( mix(_colorWarm, _colorCool, _kDiff), _colorHighlight, _kSpec);
}

 void main()
 {
    vec3 lightDir = vec3(0.0,0.0, -1.0);
    vec3 normal = normalize(v_normal);
    vec3 view = normalize(v_view);
    vec3 colorSurface = vec3(0.6,0.6,0.6);
    vec3 colorCool = u_coolColor + 0.25 * u_surfaceColor;
    //vec3 colorCool = vec3(0.0f, 0.0f, 0.55f) + 0.25 * colorSurface;
    vec3 colorWarm = u_warmColor + 0.25 * u_surfaceColor;
    //vec3 colorWarm = vec3(0.3f,0.3f,0.0f) + 0.25 * colorSurface;
    
    //vec3 colorHighlight = vec3(1.,1.,1.);
    vec3 colorHighlight = u_highlightColor;

    float ndotl = dot(normal, u_lightDir);
    float kDiff = (ndotl + 1.0) * 0.5f;
    vec3 rVec = 2.0 * (ndotl)*normal - 1.0;
    float kSpec = clamp(100.0 * dot(rVec, view)- 97.0,0.0,1.0); // saturate not available in GLSL. Use clamp instead.
    
    // Apply Shading
    vec3 color = gooch_highlighted(colorCool, colorWarm, colorHighlight, kDiff, kSpec );
    
    
    // Apply sRGB lut "pow(color,1/2.2)"
    gl_FragColor.xyz = pow(color, vec3(1.0,1.0,1.0) * 0.44);
    gl_FragColor.w = 1.0; 
 }