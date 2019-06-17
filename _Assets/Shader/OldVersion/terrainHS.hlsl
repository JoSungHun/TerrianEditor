
cbuffer TesselationValue: register(b0)
{
    float tess_factor : packoffset(c0);
    float3 CameraPos : packoffset(c0.y);
};


struct VertexOut
{
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 Uv : UV;
    float4 Position : POSITION;
};

struct HullOut
{
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 Uv : UV;
    float4 Position : POSITION;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT ConstantsHS(InputPatch<VertexOut, NUM_CONTROL_POINTS> p, uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT TessOut;

    TessOut.EdgeTess[0] = tess_factor;
    TessOut.EdgeTess[1] = tess_factor;
    TessOut.EdgeTess[2] = tess_factor;

    TessOut.InsideTess = tess_factor;

    return TessOut;
}

[domain("tri")]
[partitioning("fractional_odd")]// fractional_even
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHS")]
HullOut HS(InputPatch<VertexOut, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID,
                            uint patchID : SV_PrimitiveID)
{
    HullOut hullOut;

    hullOut.Position = ip[i].Position;
    hullOut.Uv = ip[i].Uv;
    hullOut.Normal = ip[i].Normal;
    hullOut.Tangent = ip[i].Tangent;
    hullOut.Binormal = ip[i].Binormal;

    return hullOut;
}
