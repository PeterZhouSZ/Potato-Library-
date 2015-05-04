/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
// Generated by shader2cpp.
// 2015-05-04T13:44:58.265314



extern const char* frag_lines_glsl;
const char* frag_lines_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"flat in vec4 color[2];\n"
	"in vec2 texCoord;\n"
	"flat in vec3 line;\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	float r0 = line[0];\n"
	"	float r1 = line[1];\n"
	"	float len = line[2];\n"
	"	float alpha;\n"
	"    if(texCoord.x < 0.)\n"
	"		alpha = r0 - length(texCoord);\n"
	"	else if(texCoord.x > len)\n"
	"        alpha = r1 - length(texCoord - vec2(len, 0.));\n"
	"	else\n"
	"        alpha = mix(r0, r1, texCoord.x / len) - abs(texCoord.y);\n"
	"\n"
	"    if(alpha <= 0.)\n"
	"		discard;\n"
	"    alpha = clamp(alpha, 0., 1.);\n"
	"    vec4 c = mix(color[0], color[1], texCoord.x / len);\n"
	"    out_color = mix(vec4(c.rgb, 0.), c, alpha);\n"
	"    //out_color = c;\n"
	"	//out_color = vec4(vec3(fract((texCoord.x)/r0)), 1.);\n"
	"    //out_color = vec4(texCoord, 0., 1.);\n"
	"}\n"
	"";

extern const char* frag_points_glsl;
const char* frag_points_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"flat in vec4 color;\n"
	"in vec2 texCoord;\n"
	"flat in float radius;\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	float alpha = radius - length(texCoord);\n"
	"\n"
	"	if(alpha <= 0)\n"
	"		discard;\n"
	"	alpha = clamp(alpha, 0, 1);\n"
	"	out_color = mix(vec4(color.rgb, 0.), color, alpha);\n"
	"}\n"
	"";

extern const char* geom_lines_glsl;
const char* geom_lines_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"layout(lines) in;\n"
	"layout(triangle_strip, max_vertices = 4) out;\n"
	"\n"
	"uniform vec2 viewportSize;\n"
	"\n"
	"in vec4 line_position[];\n"
	"in vec4 line_color[];\n"
	"\n"
	"flat out vec4 color[2];\n"
	"out vec2 texCoord;\n"
	"flat out vec3 line;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	vec2 sp0 = gl_in[0].gl_Position.xy * viewportSize / (gl_in[0].gl_Position.w * 2.);\n"
	"	vec2 sp1 = gl_in[1].gl_Position.xy * viewportSize / (gl_in[1].gl_Position.w * 2.);\n"
	"\n"
	"	float len = length(sp1 - sp0);\n"
	"	vec2 v = (sp1 - sp0) / len;\n"
	"	vec2 n = vec2(-v.y, v.x);\n"
	"\n"
	"	// w parameter of position encode the line width\n"
	"	float r = max(line_position[0].w, line_position[1].w) / 2. + .5001;\n"
	"\n"
	"	color[0] = line_color[0];\n"
	"	color[1] = line_color[1];\n"
	"	line = vec3(line_position[0].w / 2. + .5001, line_position[1].w / 2. + .5001, len);\n"
	"\n"
	"	gl_Position = vec4(\n"
	"			(sp0 + (-v + n)*r) * gl_in[0].gl_Position.w * 2. / viewportSize,\n"
	"			gl_in[0].gl_Position.zw);\n"
	"	texCoord = vec2(-r, r);\n"
	"	EmitVertex();\n"
	"\n"
	"	gl_Position = vec4(\n"
	"			(sp0 + (-v - n)*r) * gl_in[0].gl_Position.w * 2. / viewportSize,\n"
	"			gl_in[0].gl_Position.zw);\n"
	"	texCoord = vec2(-r, -r);\n"
	"	EmitVertex();\n"
	"\n"
	"	gl_Position = vec4(\n"
	"			(sp1 + ( v + n)*r) * gl_in[1].gl_Position.w * 2. / viewportSize,\n"
	"			gl_in[1].gl_Position.zw);\n"
	"	texCoord = vec2(len + r, r);\n"
	"	EmitVertex();\n"
	"\n"
	"	gl_Position = vec4(\n"
	"			(sp1 + ( v - n)*r) * gl_in[1].gl_Position.w * 2. / viewportSize,\n"
	"			gl_in[1].gl_Position.zw);\n"
	"	texCoord = vec2(len + r, -r);\n"
	"	EmitVertex();\n"
	"}\n"
	"";

extern const char* geom_points_glsl;
const char* geom_points_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"layout(points) in;\n"
	"layout(triangle_strip, max_vertices = 4) out;\n"
	"\n"
	"uniform vec2 viewportSize;\n"
	"\n"
	"in vec4 point_color[];\n"
	"in float point_radius[];\n"
	"\n"
	"flat out vec4 color;\n"
	"out vec2 texCoord;\n"
	"flat out float radius;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	vec2 sp = gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;\n"
	"\n"
	"	float pixelRadius = point_radius[0] + .5001;\n"
	"	vec2 r = pixelRadius * 2. / viewportSize;\n"
	"\n"
	"	color = point_color[0];\n"
	"	radius = pixelRadius;\n"
	"\n"
	"	gl_Position = vec4(\n"
	"			gl_in[0].gl_Position.xy + vec2(-r.x,  r.y) * gl_in[0].gl_Position.w,\n"
	"			gl_in[0].gl_Position.zw);\n"
	"	texCoord = vec2(-pixelRadius,  pixelRadius);\n"
	"	EmitVertex();\n"
	"\n"
	"	gl_Position = vec4(\n"
	"			gl_in[0].gl_Position.xy + vec2(-r.x, -r.y) * gl_in[0].gl_Position.w,\n"
	"			gl_in[0].gl_Position.zw);\n"
	"	texCoord = vec2(-pixelRadius, -pixelRadius);\n"
	"	EmitVertex();\n"
	"\n"
	"	gl_Position = vec4(\n"
	"			gl_in[0].gl_Position.xy + vec2( r.x,  r.y) * gl_in[0].gl_Position.w,\n"
	"			gl_in[0].gl_Position.zw);\n"
	"	texCoord = vec2( pixelRadius,  pixelRadius);\n"
	"	EmitVertex();\n"
	"\n"
	"	gl_Position = vec4(\n"
	"			gl_in[0].gl_Position.xy + vec2( r.x, -r.y) * gl_in[0].gl_Position.w,\n"
	"			gl_in[0].gl_Position.zw);\n"
	"	texCoord = vec2( pixelRadius, -pixelRadius);\n"
	"	EmitVertex();\n"
	"}\n"
	"";

extern const char* vert_lines_glsl;
const char* vert_lines_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"uniform mat4 viewMatrix;\n"
	"\n"
	"in vec4 vx_position;\n"
	"in vec4 vx_color;\n"
	"\n"
	"out vec4 line_position;\n"
	"out vec4 line_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_Position = viewMatrix * vec4(vx_position.xyz, 1.);\n"
	"\n"
	"	line_position = vx_position;\n"
	"	line_color = vx_color;\n"
	"}\n"
	"";

extern const char* vert_points_glsl;
const char* vert_points_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"uniform mat4 viewMatrix;\n"
	"\n"
	"in vec4 vx_position;\n"
	"in vec4 vx_color;\n"
	"\n"
	"out vec4 point_color;\n"
	"out float point_radius;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_Position = viewMatrix * vec4(vx_position.xyz, 1.);\n"
	"\n"
	"	point_color = vx_color;\n"
	"	point_radius = vx_position.w;\n"
	"}\n"
	"";


