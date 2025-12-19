// [ 替换你的 offscreen.vert.shader ]
#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO
{
    mat4 depthMVP; // (这个是 Light's VP * BaseModel(1.0))
} ubo;

// 1. [新增] 声明 Push Constant
layout(push_constant) uniform PushConstants {
    mat4 node_matrix;
} constants;

out gl_PerVertex
{
    vec4 gl_Position;
};


void main()
{
    // 2. [修改]
    // ubo.depthMVP 是 光源的 VP 矩阵
    // constants.node_matrix 是 节点的 M 矩阵
    // 最终位置 = (VP) * (M) * pos
    gl_Position =  ubo.depthMVP * constants.node_matrix * vec4(inPos, 1.0);
}