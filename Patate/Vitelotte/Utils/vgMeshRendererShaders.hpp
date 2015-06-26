/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
// Generated by shader2cpp.


#ifndef _VITELOTTE_UTILS_VG_MESH_RENDERER_SHADERS_
#define _VITELOTTE_UTILS_VG_MESH_RENDERER_SHADERS_


namespace Vitelotte
{
namespace VGMeshRendererShaders
{

static const char* vert_common_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"uniform mat4 viewMatrix;\n"
	"uniform mat3 normalMatrix;\n"
	"\n"
	"in vec4 vx_position;\n"
	"in vec3 vx_normal;\n"
	"\n"
	"out vec4 ctrl_position_obj;\n"
	"out vec3 ctrl_normal_obj;\n"
	"out vec3 ctrl_normal_view;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = viewMatrix * vx_position;\n"
	"    ctrl_position_obj = vx_position;\n"
	"    ctrl_normal_obj = vx_normal;\n"
	"    ctrl_normal_view = normalMatrix * vx_normal;\n"
	"}\n"
	"";

static const char* ctrl_common_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"layout(vertices = 9) out;\n"
	"\n"
	"uniform vec2 viewportSize;\n"
	"uniform float smoothness;\n"
	"\n"
	"in vec4 ctrl_position_obj[];\n"
	"in vec3 ctrl_normal_obj[];\n"
	"in vec3 ctrl_normal_view[];\n"
	"\n"
	"out vec4 eval_position_obj[];\n"
	"out vec3 eval_normal_obj[];\n"
	"out vec3 eval_normal_view[];\n"
	"out ScreenSpaceBlock {\n"
	"    flat vec3 heights_scr;\n"
	"    flat vec3 vertices_obj[3];\n"
	"    flat vec3 normEdges_obj[3];\n"
	"} eval_ss[];\n"
	"\n"
	"bool isLinear(int i) {\n"
	"    vec4 p0 = eval_position_obj[3*i + 0];\n"
	"    vec4 p1 = eval_position_obj[3*i + 1];\n"
	"    vec4 p2 = eval_position_obj[3*i + 2];\n"
	"    vec4 p3 = eval_position_obj[(3*i + 3)%9];\n"
	"\n"
	"    vec4 p1off = p1 - (2*p0 + p3) / 3;\n"
	"    vec4 p2off = p2 - (p0 + 2*p3) / 3;\n"
	"\n"
	"    return dot(p1off, p1off) < 0.0001\n"
	"        && dot(p2off, p2off) < 0.0001;\n"
	"}\n"
	"\n"
	"float curviness(int i) {\n"
	"    vec4 p0 = eval_position_obj[3*i + 0];\n"
	"    vec4 p1 = eval_position_obj[3*i + 1];\n"
	"    vec4 p2 = eval_position_obj[3*i + 2];\n"
	"    vec4 p3 = eval_position_obj[(3*i + 3)%9];\n"
	"\n"
	"    vec4 p1off = p1 - (2*p0 + p3) / 3;\n"
	"    vec4 p2off = p2 - (p0 + 2*p3) / 3;\n"
	"\n"
	"    return max(length(p1off), length(p2off)) / distance(p0, p3);\n"
	"}\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
	"    eval_position_obj[gl_InvocationID]  = ctrl_position_obj[gl_InvocationID];\n"
	"    eval_normal_obj[gl_InvocationID]    = ctrl_normal_obj[gl_InvocationID];\n"
	"    eval_normal_view[gl_InvocationID]   = ctrl_normal_view[gl_InvocationID];\n"
	"\n"
	"    vec2 position_scr[3];\n"
	"    for(int i=0; i<3; ++i)\n"
	"    {\n"
	"        position_scr[i] = (viewportSize * gl_in[i*3].gl_Position.xy)\n"
	"                        / (2.0 * gl_in[i*3].gl_Position.z);\n"
	"    }\n"
	"    float area = abs(cross(vec3(position_scr[1] - position_scr[0], 0.0),\n"
	"                           vec3(position_scr[2] - position_scr[0], 0.0)).z);\n"
	"\n"
	"    for(int i=0; i<3; ++i)\n"
	"    {\n"
	"        eval_ss[gl_InvocationID].heights_scr[i]\n"
	"                = area / length(position_scr[(i+2)%3] - position_scr[(i+1)%3]);\n"
	"        eval_ss[gl_InvocationID].vertices_obj[i]\n"
	"                = ctrl_position_obj[i*3].xyz;\n"
	"        eval_ss[gl_InvocationID].normEdges_obj[i]\n"
	"                = normalize(ctrl_position_obj[((i+2)%3)*3].xyz - ctrl_position_obj[((i+1)%3)*3].xyz);\n"
	"    }\n"
	"\n"
	"    gl_TessLevelOuter = float[4](\n"
	"                isLinear(1)? 1: smoothness,\n"
	"                isLinear(2)? 1: smoothness,\n"
	"                isLinear(0)? 1: smoothness,\n"
	"                1);\n"
	"    // Uncomment this for adaptive smoothing (using a very rough heristic).\n"
	"//    gl_TessLevelOuter = float[4](\n"
	"//                max(curviness(1) * smoothness, 1),\n"
	"//                max(curviness(2) * smoothness, 1),\n"
	"//                max(curviness(0) * smoothness, 1),\n"
	"//                1);\n"
	"    gl_TessLevelInner = float[2](1, 1);\n"
	"}\n"
	"";

static const char* eval_common_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"layout(triangles) in;\n"
	"\n"
	"uniform mat4 viewMatrix;\n"
	"\n"
	"uniform samplerBuffer nodes;\n"
	"uniform int baseNodeIndex;\n"
	"uniform bool singularTriangles;\n"
	"\n"
	"in vec4 eval_position_obj[];\n"
	"in vec3 eval_normal_obj[];\n"
	"in vec3 eval_normal_view[];\n"
	"in ScreenSpaceBlock {\n"
	"    flat vec3 heights_scr;\n"
	"    flat vec3 vertices_obj[3];\n"
	"    flat vec3 normEdges_obj[3];\n"
	"} eval_ss[];\n"
	"\n"
	"flat out int frag_index;\n"
	"out vec3 frag_linearBasis;\n"
	"out vec4 frag_position_obj;\n"
	"out vec3 frag_normal_obj;\n"
	"out vec3 frag_normal_view;\n"
	"out vec3 frag_edgeDist_scr;\n"
	"flat out vec3 frag_vertices_obj[3];\n"
	"flat out vec3 frag_normEdges_obj[3];\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"    vec3 c1 = gl_TessCoord;\n"
	"    vec3 c2 = c1 * c1;\n"
	"    vec3 c3 = c2 * c1;\n"
	"    vec4 mid = vec4(0);\n"
	"    for(int i = 0; i < 9; ++i) mid += eval_position_obj[i];\n"
	"    mid /= 9;\n"
	"    frag_position_obj = eval_position_obj[0] * c3[0]\n"
	"                      + eval_position_obj[1] * c2[0] * c1[1] * 3\n"
	"                      + eval_position_obj[2] * c1[0] * c2[1] * 3\n"
	"                      + eval_position_obj[3] * c3[1]\n"
	"                      + eval_position_obj[4] * c2[1] * c1[2] * 3\n"
	"                      + eval_position_obj[5] * c1[1] * c2[2] * 3\n"
	"                      + eval_position_obj[6] * c3[2]\n"
	"                      + eval_position_obj[7] * c2[2] * c1[0] * 3\n"
	"                      + eval_position_obj[8] * c1[2] * c2[0] * 3\n"
	"                      + mid                  * c1[0] * c1[1] * c1[2] * 6;\n"
	"\n"
	"    gl_Position = viewMatrix * frag_position_obj;\n"
	"    frag_index         = gl_PrimitiveID;\n"
	"    frag_linearBasis   = gl_TessCoord;\n"
	"    frag_normal_obj    = mat3(eval_normal_obj[0],\n"
	"                              eval_normal_obj[3],\n"
	"                              eval_normal_obj[6]) * gl_TessCoord;\n"
	"    frag_normal_view   = mat3(eval_normal_view[0],\n"
	"                              eval_normal_view[3],\n"
	"                              eval_normal_view[6]) * gl_TessCoord;\n"
	"    frag_edgeDist_scr  = eval_ss[0].heights_scr * gl_TessCoord;\n"
	"    frag_vertices_obj  = eval_ss[0].vertices_obj;\n"
	"    frag_normEdges_obj = eval_ss[0].normEdges_obj;\n"
	"}\n"
	"";

static const char* geom_common_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"\n"
	"uniform vec2 viewportSize;\n"
	"\n"
	"in vec4 geom_position_obj[];\n"
	"in vec3 geom_normal_obj[];\n"
	"in vec3 geom_normal_view[];\n"
	"\n"
	"flat out int frag_index;\n"
	"out vec3 frag_linearBasis;\n"
	"out vec3 frag_position_obj;\n"
	"out vec3 frag_normal_obj;\n"
	"out vec3 frag_normal_view;\n"
	"out vec3 frag_edgeDist_scr;\n"
	"flat out vec3 frag_vertices_obj[3];\n"
	"flat out vec3 frag_normEdges_obj[3];\n"
	"\n"
	"const vec3 basis[3] = vec3[3](\n"
	"    vec3(1, 0, 0),\n"
	"    vec3(0, 1, 0),\n"
	"    vec3(0, 0, 1)\n"
	");\n"
	"\n"
	"void main()\n"
	"{\n"
	"    vec2 position_scr[3];\n"
	"    for(int i=0; i<3; ++i)\n"
	"    {\n"
	"        position_scr[i] = (viewportSize * gl_in[i].gl_Position.xy)\n"
	"                        / (2.0 * gl_in[i].gl_Position.z);\n"
	"    }\n"
	"    float area = abs(cross(vec3(position_scr[1] - position_scr[0], 0.0),\n"
	"                           vec3(position_scr[2] - position_scr[0], 0.0)).z);\n"
	"    for(int i=0; i<3; ++i)\n"
	"    {\n"
	"        gl_Position = gl_in[i].gl_Position;\n"
	"        frag_index = gl_PrimitiveIDIn;\n"
	"        frag_linearBasis = basis[i];\n"
	"        frag_position_obj = geom_position_obj[i].xyz;\n"
	"        frag_normal_obj = geom_normal_obj[i];\n"
	"        frag_normal_view = geom_normal_view[i];\n"
	"        frag_edgeDist_scr = vec3(0.0);\n"
	"        frag_edgeDist_scr[i] = area / length(position_scr[(i+2)%3] - position_scr[(i+1)%3]);\n"
	"        for(int j=0; j<3; ++j)\n"
	"        {\n"
	"            frag_vertices_obj[j] = geom_position_obj[j].xyz;//gl_in[j].gl_Position.xy;\n"
	"            frag_normEdges_obj[j] = normalize(geom_position_obj[(j+2)%3].xyz\n"
	"                                              - geom_position_obj[(j+1)%3].xyz);\n"
	"        }\n"
	"        EmitVertex();\n"
	"    }\n"
	"}\n"
	"";

static const char* frag_common_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"uniform float zoom;\n"
	"uniform float pointRadius;\n"
	"uniform float halfLineWidth;\n"
	"uniform bool showWireframe;\n"
	"uniform vec4 wireframeColor;\n"
	"uniform vec4 pointColor;\n"
	"\n"
	"in vec3 frag_linearBasis;\n"
	"in vec2 position;\n"
	"flat in vec2 vertices[3];\n"
	"flat in vec2 normEdges[3];\n"
	"\n"
	"\n"
	"float irlerp(in vec3 vx, in vec3 v1, in vec3 v2)\n"
	"{\n"
	"    float alpha = acos(clamp(dot(v1, vx), -1., 1.));\n"
	"    float beta = acos(clamp(dot(v1, v2), -1., 1.));\n"
	"    return alpha / beta;\n"
	"}\n"
	"\n"
	"\n"
	"vec4 quadraticInterp(in vec4 colors[6])\n"
	"{\n"
	"    return\n"
	"        colors[0] * frag_linearBasis.x * (2. * frag_linearBasis.x - 1.) +\n"
	"        colors[1] * frag_linearBasis.y * (2. * frag_linearBasis.y - 1.) +\n"
	"        colors[2] * frag_linearBasis.z * (2. * frag_linearBasis.z - 1.) +\n"
	"        colors[3] * 4. * frag_linearBasis.y * frag_linearBasis.z +\n"
	"        colors[4] * 4. * frag_linearBasis.z * frag_linearBasis.x +\n"
	"        colors[5] * 4. * frag_linearBasis.x * frag_linearBasis.y;\n"
	"}\n"
	"\n"
	"float diffuse(in vec3 n, in vec3 l)\n"
	"{\n"
	"    return clamp(dot(n, l), 0., 1.);\n"
	"}\n"
	"\n"
	"\n"
	"///////////////////////////////////////////////////////////////////////////////\n"
	"// Color space conversion\n"
	"\n"
	"const int COLOR_NONE       = 0;\n"
	"const int COLOR_SRGB       = 1;\n"
	"const int COLOR_LINEAR_RGB = 2;\n"
	"const int COLOR_CIE_XYZ    = 3;\n"
	"const int COLOR_CIE_LAB    = 4;\n"
	"const int COLOR_CIE_LUV    = 5;\n"
	"\n"
	"\n"
	"// SRGB <-> linear rgbToxyz\n"
	"\n"
	"vec3 srgbFromLinearRGB(const in vec3 linear)\n"
	"{\n"
	"    vec3 srgb = linear;\n"
	"    srgb[0] = (linear[0] > 0.0031308)?\n"
	"              1.055 * pow(linear[0], 1./2.4) - .055f:\n"
	"              12.92 * linear[0];\n"
	"    srgb[1] = (linear[1] > 0.0031308)?\n"
	"              1.055 * pow(linear[1], 1./2.4) - .055f:\n"
	"              12.92 * linear[1];\n"
	"    srgb[2] = (linear[2] > 0.0031308)?\n"
	"              1.055 * pow(linear[2], 1./2.4) - .055f:\n"
	"              12.92 * linear[2];\n"
	"    return srgb;\n"
	"}\n"
	"\n"
	"vec3 linearRGBFromSrgb(const in vec3 srgb)\n"
	"{\n"
	"    vec3 linear = srgb;\n"
	"    linear[0] = (linear[0] > 0.04045)?\n"
	"                pow((linear[0]+0.055) / 1.055, 2.4):\n"
	"                linear[0] / 12.92;\n"
	"    linear[1] = (linear[1] > 0.04045)?\n"
	"                pow((linear[1]+0.055) / 1.055, 2.4):\n"
	"                linear[1] / 12.92;\n"
	"    linear[2] = (linear[2] > 0.04045)?\n"
	"                pow((linear[2]+0.055) / 1.055, 2.4):\n"
	"                linear[2] / 12.92;\n"
	"    return linear;\n"
	"}\n"
	"\n"
	"\n"
	"// Linear RGB <-> Cie XYZ\n"
	"\n"
	"const mat3 xyzToRgb = mat3(\n"
	"     3.2406, -1.5372, -0.4986,\n"
	"    -0.9689,  1.8758,  0.0415,\n"
	"     0.0557, -0.2040,  1.0570\n"
	");\n"
	"\n"
	"vec3 linearRGBFromCieXYZ(const in vec3 cieXYZ) {\n"
	"    return transpose(xyzToRgb) * cieXYZ;\n"
	"}\n"
	"\n"
	"mat3 rgbToxyz = mat3(\n"
	"     0.4124, 0.3576, 0.1805,\n"
	"     0.2126, 0.7152, 0.0722,\n"
	"     0.0193, 0.1192, 0.9505\n"
	");\n"
	"\n"
	"vec3 cieXYZFromLinearRGB(const in vec3 lrgb) {\n"
	"    return transpose(rgbToxyz) * lrgb;\n"
	"}\n"
	"\n"
	"\n"
	"// Cie XYZ <-> Cie Lab\n"
	"\n"
	"float cieLabF(const in float v) {\n"
	"    return (v > 0.008856452)?\n"
	"                pow(v, 1./3.):\n"
	"                1. / (3. * 0.042806183) * v + (4. / 29.);\n"
	"}\n"
	"\n"
	"float cieLabFInv(const in float v) {\n"
	"    return (v > 0.206896552)?\n"
	"                pow(v, 3.):\n"
	"                3. * 0.042806183 * (v - (4. / 29.));\n"
	"}\n"
	"\n"
	"const vec3 cieLabWhite = vec3(0.95047, 1, 1.08883);\n"
	"\n"
	"vec3 cieLabFromCieXYZ(const in vec3 cieXYZ)\n"
	"{\n"
	"    float fy = cieLabF(cieXYZ[1] / cieLabWhite[1]);\n"
	"    return vec3(\n"
	"            1.16 * fy - 0.16,\n"
	"            5.00 * (cieLabF(cieXYZ[0] / cieLabWhite[0]) - fy),\n"
	"            2.00 * (fy - cieLabF(cieXYZ[2] / cieLabWhite[2])));\n"
	"}\n"
	"\n"
	"\n"
	"vec3 cieXYZFromCieLab(const in vec3 cielab)\n"
	"{\n"
	"    float lf = (cielab[0] + 0.16) / 1.16;\n"
	"    return vec3(\n"
	"            cieLabWhite[0] * cieLabFInv(lf + cielab[1] / 5.00),\n"
	"            cieLabWhite[1] * cieLabFInv(lf),\n"
	"            cieLabWhite[2] * cieLabFInv(lf - cielab[2] / 2.00));\n"
	"}\n"
	"\n"
	"\n"
	"// Cie XYZ <-> Cie Luv\n"
	"\n"
	"vec3 cieLuvFromCieXYZ(const in vec3 cieXYZ)\n"
	"{\n"
	"    const float wu = 0.197839825f;\n"
	"    const float wv = 0.468336303f;\n"
	"\n"
	"    float l = (cieXYZ[1] <= 0.008856452f)?\n"
	"                    9.03296296296f * cieXYZ[1]:\n"
	"                    1.16f * pow(cieXYZ[1], 1.f/3.f) - .16f;\n"
	"    float d = cieXYZ[0] + 15.f * cieXYZ[1] + 3.f * cieXYZ[2];\n"
	"\n"
	"    return vec3(\n"
	"                l,\n"
	"                (d > .001f)? 13.f * l * (4.f*cieXYZ[0] / d - wu): 0.f,\n"
	"                (d > .001f)? 13.f * l * (9.f*cieXYZ[1] / d - wv): 0.f);\n"
	"}\n"
	"\n"
	"vec3 cieXYZFromCieLuv(const in vec3 cieluv)\n"
	"{\n"
	"    const float wu = 0.197839825f;\n"
	"    const float wv = 0.468336303f;\n"
	"\n"
	"    float up_13l = cieluv[1] + wu * (13.f * cieluv[0]);\n"
	"    float vp_13l = cieluv[2] + wv * (13.f * cieluv[0]);\n"
	"\n"
	"    float y = (cieluv[0] <= .08f)?\n"
	"                    cieluv[0] * 0.1107056f:\n"
	"                    pow((cieluv[0]+.16f) / 1.16f, 3.f);\n"
	"    return vec3(\n"
	"                (vp_13l > .001f)? 2.25f * y * up_13l / vp_13l: 0.f,\n"
	"                y,\n"
	"                (vp_13l > .001f)? y * (156.f*cieluv[0] - 3.f*up_13l - 20.f*vp_13l) / (4.f * vp_13l): 0.f);\n"
	"}\n"
	"\n"
	"\n"
	"// General color conversion\n"
	"\n"
	"vec3 convertColor(in vec3 fromColor, in int from, in int to)\n"
	"{\n"
	"    if(from == COLOR_NONE || to == COLOR_NONE || from == to)\n"
	"        return fromColor;\n"
	"\n"
	"    vec3 color = fromColor;\n"
	"\n"
	"    // To XYZ\n"
	"    if(from == COLOR_SRGB) {\n"
	"        color = linearRGBFromSrgb(color);\n"
	"        from = COLOR_LINEAR_RGB;\n"
	"        if(to == COLOR_LINEAR_RGB) return color;\n"
	"    }\n"
	"    if(from == COLOR_LINEAR_RGB) {\n"
	"        color = cieXYZFromLinearRGB(color);\n"
	"    }\n"
	"    if(from == COLOR_CIE_LAB) {\n"
	"        color = cieXYZFromCieLab(color);\n"
	"    }\n"
	"    if(from == COLOR_CIE_LUV) {\n"
	"        color = cieXYZFromCieLuv(color);\n"
	"    }\n"
	"\n"
	"    // From XYZ\n"
	"    if(to < COLOR_CIE_XYZ) {\n"
	"        color = linearRGBFromCieXYZ(color);\n"
	"        if(to == COLOR_SRGB) {\n"
	"            color = srgbFromLinearRGB(color);\n"
	"        }\n"
	"    } else if(to == COLOR_CIE_LAB) {\n"
	"        color = cieLabFromCieXYZ(color);\n"
	"    } else if(to == COLOR_CIE_LUV) {\n"
	"        color = cieLuvFromCieXYZ(color);\n"
	"    }\n"
	"\n"
	"    return color;\n"
	"}\n"
	"\n"
	"\n"
	"";

static const char* frag_linear_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"uniform samplerBuffer nodes;\n"
	"uniform int baseNodeIndex;\n"
	"uniform bool singularTriangles;\n"
	"uniform bool enableShading;\n"
	"uniform int meshColorSpace;\n"
	"uniform int screenColorSpace;\n"
	"\n"
	"flat in int frag_index;\n"
	"in vec3 frag_linearBasis;\n"
	"in vec4 frag_position_obj;\n"
	"in vec3 frag_normal_obj;\n"
	"in vec3 frag_normal_view;\n"
	"flat in vec3 frag_vertices_obj[3];\n"
	"flat in vec3 frag_normEdges_obj[3];\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"float irlerp(in vec3 vx, in vec3 v1, in vec3 v2);\n"
	"vec4 quadraticInterp(in vec4 colors[6]);\n"
	"float diffuse(in vec3 n, in vec3 l);\n"
	"vec3 convertColor(in vec3 fromColor, in int from, in int to);\n"
	"\n"
	"int baseVxIndex = baseNodeIndex + frag_index * (3 + int(singularTriangles));\n"
	"\n"
	"vec4 linearInterp(in vec4 colors[3])\n"
	"{\n"
	"    return\n"
	"        colors[0] * frag_linearBasis.x +\n"
	"        colors[1] * frag_linearBasis.y +\n"
	"        colors[2] * frag_linearBasis.z;\n"
	"}\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"    vec4 colorNodes[] = vec4[3](\n"
	"        texelFetch(nodes, baseVxIndex + 0),\n"
	"        texelFetch(nodes, baseVxIndex + 1),\n"
	"        texelFetch(nodes, baseVxIndex + 2)\n"
	"    );\n"
	"\n"
	"    if(singularTriangles)\n"
	"    {\n"
	"        colorNodes[0] = mix(colorNodes[0],\n"
	"                            texelFetch(nodes, baseVxIndex + 3),\n"
	"                            irlerp(normalize(frag_position_obj.xyz - frag_vertices_obj[0]),\n"
	"                                   frag_normEdges_obj[2], -frag_normEdges_obj[1]));\n"
	"    }\n"
	"\n"
	"    out_color = linearInterp(colorNodes);\n"
	"\n"
	"    if(enableShading) {\n"
	"        // Shading is done in linear RGB\n"
	"        out_color.rgb = convertColor(out_color.rgb, meshColorSpace, 2);\n"
	"\n"
	"        vec3 n = normalize(frag_normal_view);\n"
	"        vec3 light = vec3(0.);\n"
	"        light = diffuse(n, normalize(vec3(-.2, 0, -1.))) * vec3(1., .9, .8) * .8\n"
	"              + diffuse(n, normalize(vec3( 1, .2,  .2))) * vec3(.8, .9, 1.) * .6;\n"
	"\n"
	"        out_color.rgb = convertColor(light * out_color.rgb,\n"
	"                                     1, screenColorSpace);\n"
	"    } else {\n"
	"        out_color.rgb = convertColor(out_color.rgb, meshColorSpace, screenColorSpace);\n"
	"    }\n"
	"}\n"
	"";

static const char* frag_quadratic_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"uniform samplerBuffer nodes;\n"
	"uniform int baseNodeIndex;\n"
	"uniform bool singularTriangles;\n"
	"uniform bool enableShading;\n"
	"uniform int meshColorSpace;\n"
	"uniform int screenColorSpace;\n"
	"\n"
	"flat in int frag_index;\n"
	"in vec3 frag_linearBasis;\n"
	"in vec4 frag_position_obj;\n"
	"in vec3 frag_normal_obj;\n"
	"in vec3 frag_normal_view;\n"
	"flat in vec3 frag_vertices_obj[3];\n"
	"flat in vec3 frag_normEdges_obj[3];\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"float irlerp(in vec3 vx, in vec3 v1, in vec3 v2);\n"
	"vec4 quadraticInterp(in vec4 colors[6]);\n"
	"float diffuse(in vec3 n, in vec3 l);\n"
	"vec3 convertColor(in vec3 fromColor, in int from, in int to);\n"
	"\n"
	"int baseVxIndex = baseNodeIndex + frag_index * (6 + int(singularTriangles));\n"
	"int baseEdgeIndex = baseVxIndex + 3;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"    vec4 colorNodes[] = vec4[6](\n"
	"        texelFetch(nodes, baseVxIndex + 0),\n"
	"        texelFetch(nodes, baseVxIndex + 1),\n"
	"        texelFetch(nodes, baseVxIndex + 2),\n"
	"        texelFetch(nodes, baseEdgeIndex + 0),\n"
	"        texelFetch(nodes, baseEdgeIndex + 1),\n"
	"        texelFetch(nodes, baseEdgeIndex + 2)\n"
	"    );\n"
	"\n"
	"    if(singularTriangles)\n"
	"    {\n"
	"        colorNodes[0] = mix(\n"
	"            colorNodes[0],\n"
	"            texelFetch(nodes, baseVxIndex + 6),\n"
	"            irlerp(normalize(frag_position_obj.xyz - frag_vertices_obj[0]),\n"
	"                   frag_normEdges_obj[2], -frag_normEdges_obj[1]));\n"
	"    }\n"
	"\n"
	"    // Interpolation is done in srgb\n"
	"    out_color = quadraticInterp(colorNodes);\n"
	"\n"
	"    if(enableShading) {\n"
	"        // Shading is done in linear RGB\n"
	"        out_color.rgb = convertColor(out_color.rgb, meshColorSpace, 2);\n"
	"\n"
	"        vec3 n = normalize(frag_normal_view);\n"
	"        vec3 light = vec3(0.);\n"
	"        light = diffuse(n, normalize(vec3(-.2, 0, -1.))) * vec3(1., .9, .8) * .8\n"
	"              + diffuse(n, normalize(vec3( 1, .2,  .2))) * vec3(.8, .9, 1.) * .6;\n"
	"\n"
	"        out_color.rgb = convertColor(light * out_color.rgb,\n"
	"                                     1, screenColorSpace);\n"
	"    } else {\n"
	"        out_color.rgb = convertColor(out_color.rgb, meshColorSpace, screenColorSpace);\n"
	"    }\n"
	"}\n"
	"";

static const char* frag_wireframe_glsl =
	"/*\n"
	" This Source Code Form is subject to the terms of the Mozilla Public\n"
	" License, v. 2.0. If a copy of the MPL was not distributed with this\n"
	" file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
	"*/\n"
	"\n"
	"#version 410 core\n"
	"\n"
	"uniform samplerBuffer nodes;\n"
	"uniform float lineWidth;\n"
	"uniform vec4 wireframeColor;\n"
	"\n"
	"in vec3 frag_edgeDist_scr;\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"vec3 computeEdgeDist();\n"
	"int minIndex(in vec3 dist);\n"
	"float interpFactor(float dist, float radius);\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"    float alpha = smoothstep(\n"
	"        -0.5, 0.5,\n"
	"        lineWidth / 2.0 - min(frag_edgeDist_scr.x,\n"
	"                              min(frag_edgeDist_scr.y,\n"
	"                                  frag_edgeDist_scr.z)));\n"
	"    if(alpha < 0.001)\n"
	"        discard;\n"
	"\n"
	"    out_color = vec4(wireframeColor.rgb, wireframeColor.a * alpha);\n"
	"}\n"
	"";

}
}

#endif
