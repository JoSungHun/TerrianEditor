//==============================================================
//Vertex Input
//==============================================================
struct Vertex
{
    float4 position : POSITION0;
};

struct VertexColor
{
    float4 position : POSITION0;
    float4 color    : COLOR0;
};

struct VertexTexture
{
    float4 position : POSITION0;
    float2 uv       : TEXCOORD0;
};

struct VertexTNTBB
{
    float4 position     : POSITION0;
    float2 uv           : TEXCOORD0;
    float3 normal       : NORMAL0;
    float3 tangent      : TANGENT0;
    float4 blendIndices : BLENDINDICES0;
    float4 blendWeights : BLENDWEIGHTS0;
};

struct VertexTerrain
{
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float3 binormal : BINORMAL0;
    float4 color : COLOR0;
    float4 alpha : COLOR1;
};
//==============================================================
//Pixel Input
//==============================================================
struct Pixel
{
    float4 position : SV_POSITION0;
};

struct PixelColor
{
    float4 position : SV_POSITION0;
    float4 color    : COLOR0;
};

struct PixelTexture
{
    float4 position : SV_POSITION0;
    float2 uv       : TEXCOORD0;
};

struct PixelTNT
{
    float4 position : SV_POSITION0;
    float2 uv       : TEXCOORD0;
    float3 normal   : NORMAL0;
    float3 tangent  : TANGENT0;
};