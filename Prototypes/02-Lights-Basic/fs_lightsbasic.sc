$input v_pos, v_normal, v_view

/*
 * Copyright 2025 Soumitra Goswami. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */
#include "../common/common.sh"

#define PI 3.14159265

vec3 brdf_ggx(vec3 _normal, vec3 _wi, vec3 _wo, vec3 _f0, vec3 _albedo, float _roughness, float _metallic)
{
    vec3 h = normalize(_wi + _wo);
    float ndotl = max(dot(_normal, _wi), 0.0);
    float ndotv = max(dot(_normal, _wo), 0.0);
    float ndoth = max(dot(_normal, h), 0.0);
    float vdoth = max(dot(_wo, h), 0.0);

    // Distribution function (GGX)
    float a = _roughness * _roughness;
    float a2 = a * a;
    float denom = (ndoth * ndoth) * (a2 - 1.0) + 1.0;
    float D = a2 / (PI * denom * denom + 1e-5);

    // Geometry function (Schlick-GGX)
    float k = (a + 1.0) * (a + 1.0) / 8.0;
    float G1o = ndotv / (ndotv * (1.0 - k) + k + 1e-5);
    float G1i = ndotl / (ndotl * (1.0 - k) + k + 1e-5);
    float G = G1o * G1i;
    
    vec3 f0 = mix(vec3(0.04, 0.04, 0.04), _f0, _metallic);

    // Fresnel function (Schlick's approximation)
    vec3 F = f0 + (vec3(1.0, 1.0, 1.0) - f0) * pow(1.0 - vdoth, 5.0);

    vec3 specular = (D * G * F) / (4.0 * ndotv * ndotl + 1e-5);

    vec3 kD = (vec3(1.0, 1.0, 1.0) - F) * (1.0 - _metallic);

    return kD * _albedo/ PI + specular;
}

vec3 light_point_attenuated(vec3 _lightColor, float _radiusSquared, float _minRadius, float _maxRadius)
{
    float dampening = max(0.0, 1.0 - ((_radiusSquared/(_maxRadius*_maxRadius))*(_radiusSquared/(_maxRadius*_maxRadius))));
    dampening = dampening * dampening;
    float intensity = 1.0/max(_minRadius*_minRadius, _radiusSquared);
    return _lightColor * intensity * dampening;
}

 void main()
 {
    vec3 normal = normalize(v_normal);
    vec3 view = -normalize(v_view);
    vec3 surfaceColor = vec3(0.6,0.6,0.6);
    vec3 f0_gold = vec3(1.02, 0.782, 0.344);
    vec3 lightPos = vec3(0.0,2.0,10.0);
    vec3 lightMinusPos = lightPos - v_pos.xyz;
    vec3 lightDir = normalize(lightMinusPos);

    vec3 lightColor = light_point_attenuated(vec3(1.0,1.0,1.0), dot(lightMinusPos, lightMinusPos), 1.0, 50.0);
    //vec3 lightColor = vec3(1.0,1.0,1.0);
    vec3 brdf = brdf_ggx(normal, lightDir, view, f0_gold, surfaceColor, 0.2, 0.5);

    vec3 color = lightColor * brdf * max(dot(normal, lightDir), 0.0);


    gl_FragColor.xyz = pow(color, vec3(1.0,1.0,1.0)*0.44);
    gl_FragColor.w = 1.0;
 }