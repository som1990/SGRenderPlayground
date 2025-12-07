uniform vec4 u_params[4];


#define u_time				u_params[0].x
#define u_warmColor			u_params[0].yzw
#define u_coolColor			u_params[1].xyz
#define u_highlightColor	u_params[2].xyz
#define u_surfaceColor		u_params[3].xyz
#define u_lightDir			vec3(u_params[1].w, u_params[2].w, u_params[3].w)	