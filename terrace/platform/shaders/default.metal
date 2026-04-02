//
//  default.metal
//  terrace
//
//  Created by celine on 2026-03-21.
//

#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float4 position;
    float4 normal;
    float2 uv;
    float4 tangent;
    float4 color;
};

struct VertexOut {
    float4 position [[position]];
    float4 normal;
    float2 uv;
    float4 tangent;
    float4 color;
};

vertex VertexOut vertex_main(
    uint vid [[vertex_id]],
    device VertexIn* verts [[buffer(0)]],
    constant float2& viewportSize [[buffer(1)]])   // pass {W, H} from CPU
{
    VertexOut out;
    float aspect = viewportSize.x / viewportSize.y;

    float4 pos = verts[vid].position;
    pos.x /= aspect;   // compress x so world units are square on screen

    out.position = pos;
    out.color    = verts[vid].color;
    out.normal   = verts[vid].normal;
    out.uv       = verts[vid].uv;
    out.tangent  = verts[vid].tangent;
    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]]) {
    return in.color;
}
