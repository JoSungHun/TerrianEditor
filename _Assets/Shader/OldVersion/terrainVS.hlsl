

struct VS_INPUT
{
    float4 Position : POSITION0;
    float2 Uv : UV0;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float3 Binormal : NORMAL2;
    float4 Color : COLOR0;
};

struct VS_OUTPUT_HS_INPUT
{
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 Uv : UV;
    float4 Position : POSITION;
};

VS_OUTPUT_HS_INPUT VS(VS_INPUT input)
{
    VS_OUTPUT_HS_INPUT output;

    output.Position = input.Position;
    output.Uv = input.Uv;
    output.Binormal = input.Binormal;
    output.Tangent = input.Tangent;
    output.Normal = input.Normal;

    
    
    return output;
}

