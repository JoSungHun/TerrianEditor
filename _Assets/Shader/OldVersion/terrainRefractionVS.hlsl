cbuffer CameraBuffer : register(b0)
{
    matrix View : packoffset(c0);
    matrix Proj : packoffset(c4);
};

cbuffer TransformBuffer : register(b1)
{
    matrix World : packoffset(c0);
}; 
 
cbuffer TerrainData : register(b2)
{
    float2 TerrainSize : packoffset(c0);
};

cbuffer ClipPlaneBuffer : register(b3)
{
    float4 clipPlane;
};

struct VS_INPUT
{
    float4 Position : POSITION0;
    float2 Uv : UV0;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float3 Binormal : NORMAL2;
    float4 Color : COLOR0;
};

struct PixelInput
{
    float4 Position : SV_POSITION; 
    float2 Uv : UV0; 
    float2 bUv : UV1;
    float3 Normal : NORMAL;
    float clip : SV_ClipDistance0;
};



PixelInput VS(VS_INPUT input)
{
    PixelInput output;
    
    input.Position.w = 1.0f;

    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Proj);
    
    output.bUv = float2(output.Position.x / TerrainSize[0], output.Position.z / TerrainSize[1]);
    output.Uv = input.Uv;

    output.Normal = mul(input.Normal, (float3x3) World);

    output.Normal = normalize(output.Normal);

    output.clip = dot(mul(input.Position, World), clipPlane);
    
    return output;
}