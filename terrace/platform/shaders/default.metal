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
    device VertexIn* verts [[buffer(0)]]) {
    
    VertexOut out;
    out.position = verts[vid].position;
    out.color    = verts[vid].color;
    out.normal   = verts[vid].normal;
    out.uv       = verts[vid].uv;
    out.tangent  = verts[vid].tangent;
    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]]) {
    return in.color;
}
