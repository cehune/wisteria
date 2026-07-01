//
//  default.metal
//  wisteria
//
//  Created by celine on 2026-03-21.
//

#include <metal_stdlib>
#include "../common/Types.h"
using namespace metal;


vertex VertexOut vertex_main(
    uint vid [[vertex_id]],
    device VertexIn*            verts [[buffer(0)]],
    constant CameraUniforms&    cam   [[buffer(1)]],
    constant float4x4&           model [[buffer(2)]]) // transforms
{
    VertexOut out;
    float4 local_position = verts[vid].position;
    out.position = cam.viewProjection * model * local_position;
    out.normal  = model * verts[vid].normal;
    out.tangent = model * verts[vid].tangent;
    // temp until we have colours and materials
    out.color = {out.normal[0] * 0.5 + 0.5, out.normal[1] * 0.5 + 0.5, out.normal[2] * 0.5 + 0.5, 1.0};
    out.uv = verts[vid].uv;

    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]]) {
    return in.color;
}
