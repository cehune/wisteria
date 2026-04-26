//
//  default.metal
//  terrace
//
//  Created by celine on 2026-03-21.
//

#include <metal_stdlib>
#include "Types.hpp"
using namespace metal;


vertex VertexOut vertex_main(
    uint vid [[vertex_id]],
    device VertexIn*            verts [[buffer(0)]],
    constant CameraUniforms&    cam   [[buffer(1)]])
{
    VertexOut out;
    float4 world_position = verts[vid].position;
    out.position = cam.viewProjection * world_position;
    //out.color = verts[vid].color;
    out.normal = verts[vid].normal;
    // temp until we have colours and materials
    out.color = {out.normal[0] * 0.5 + 0.5, out.normal[1] * 0.5 + 0.5, out.normal[2] * 0.5 + 0.5, 1.0};
    out.tangent = verts[vid].tangent;
    out.uv = verts[vid].uv;

    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]]) {
    return in.color;
}
