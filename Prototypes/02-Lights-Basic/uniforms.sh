uniform vec4 u_params[4];

#define u_albedo			u_params[0].xyz
#define u_roughness			u_params[0].w
#define u_f0				u_params[1].xyz
#define u_metallic			u_params[1].w
#define u_lightPos			u_params[2].xyz
#define u_lightRadiusMin	u_params[2].w
#define u_lightColor		u_params[3].xyz
#define u_lightRadiusMax	u_params[3].w