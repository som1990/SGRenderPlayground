$input v_world, v_normal, v_view

/*
 * Copyright 2025 Soumitra Goswami. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */
#include "../common/common.sh"
#include "uniforms.sh"

#define PI 3.14159265


// Taken from Siggraph 2014: Moving Frostbite to Physically based rendering V3 course notes. Sebastien Lagarde
// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v32.pdf
vec3 F_Schlick(in vec3 f0, in float f90, in float u)
{
    return f0 + (f90 - f0) * pow(1.f - u, 5.f);
}

// Taken from Siggraph 2014: Moving Frostbite to Physically based rendering V3 course notes. Sebastien Lagarde
// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v32.pdf
float D_GGX(float _NdotH, float _alpha)
{
    float alpha2 = _alpha * _alpha;
    float f = (_NdotH * alpha2 - _NdotH)*_NdotH + 1.0;
    return alpha2 / (f*f);
}

// Taken from Siggraph 2014: Moving Frostbite to Physically based rendering V3 course notes. Sebastien Lagarde
// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v32.pdf
float G_SmithGGXCorrelated(float _NdotL, float _NdotV, float _alphaG)
{
    float alphaG2 = _alphaG * _alphaG;
    float Lambda_GGXV = _NdotL * sqrt((-_NdotV * alphaG2 + _NdotV)) * _NdotV + alphaG2;
    float Lambda_GGXL = _NdotV * sqrt((-_NdotL * alphaG2 + _NdotL)) * _NdotL + alphaG2;

    return 0.5f/ (Lambda_GGXL + Lambda_GGXV);
}


vec3 light_point_attenuated(vec3 _lightColor, float _radiusSquared, float _minRadius, float _maxRadius)
{
    //Taken from chapter 5.2 of Real Time Rendering 3rd Edition (Pg 111-113)
    
    // Dampening factor taken from unreal and Frostbyte game engines. RealTime Rendering 3rd Edition Eq: 5.14
    float dampening = max(0.0, 1.0 - ((_radiusSquared/(_maxRadius*_maxRadius))*(_radiusSquared/(_maxRadius*_maxRadius))));
    dampening = dampening * dampening;
    
    // Assuming at 1 m measured luminance of 50.0 .
    float intensity = 50.0 * 1.0/max(_minRadius*_minRadius, _radiusSquared);
    return _lightColor * intensity * dampening;
}

// Taken from Siggraph 2014: Moving Frostbite to Physically based rendering V3 course notes. Sebastien Lagarde
// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v32.pdf
float Fr_DisneyDiffuse (float _NdotV, float _NdotL, float _LdotH, float _roughness )
{
    float energyBias    = mix(0, 0.5f, _roughness);
    float energyFactor  = mix(1.0, 1.0/1.51, _roughness);
    float fd90          = energyBias + 2.0 * _LdotH * _LdotH * _roughness;
    vec3  f0            = vec3(1.0f, 1.0f, 1.0f);
    float lightScatter  = F_Schlick(f0, fd90, _NdotL).r;
    float viewScatter   = F_Schlick(f0, fd90, _NdotV).r;

    return lightScatter * viewScatter * energyFactor;
}

void main()
{
vec3 normal = normalize(v_normal);
vec3 view = -normalize(v_view);
vec3 surfaceColor = u_albedo;
vec3 lightPos = u_lightPos;
vec3 lightMinusPos = lightPos - v_world.xyz;
vec3 lightDir = normalize(lightMinusPos);

vec3 lightColor = light_point_attenuated(u_lightColor, dot(lightMinusPos, lightMinusPos), u_lightRadiusMin, u_lightRadiusMax);

float NdotV = abs(dot(normal, view)) + 1e-5f;
vec3  H     = normalize(view + lightDir);
float LdotH = clamp(dot(lightDir, H), 0.f, 1.f);
float NdotH = clamp(dot(normal, H), 0.f, 1.f);
float NdotL = clamp(dot(normal, lightDir), 0.f, 1.f);

// Specular BRDF 
vec3 f0    = mix(vec3(1.0,1.0,1.0), u_f0, u_metallic);
// f90 formula taken from Siggraph 2014: Moving Frostbite to Physically based rendering V3 course notes (pg78).
// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v32.pdf
float f90   = clamp(50.0 * dot(u_f0, vec3(0.33,0.33,0.33)),0.0, 1.0);
vec3  F     = F_Schlick(u_f0, f90, LdotH);
float G     = G_SmithGGXCorrelated(NdotV, NdotL, u_roughness*u_roughness);
float D     = D_GGX(NdotH, u_roughness * u_roughness);
vec3  Fr    = (D * G / PI) * F;

// Diffuse BRDF
float Fd    = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, u_roughness);

vec3 color =  lightColor * Fr * Fd;

gl_FragColor.xyz = pow(color, vec3(1.0,1.0,1.0)*0.44) ;
gl_FragColor.w = 1.0;
}