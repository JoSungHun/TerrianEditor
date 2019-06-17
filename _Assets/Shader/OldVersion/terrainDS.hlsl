cbuffer CameraBuffer : register(b0)
{
    matrix View : packoffset(c0);
    matrix Proj : packoffset(c4);
};

cbuffer TransformBuffer : register(b1)
{
    matrix World : packoffset(c0);
}; 

cbuffer BrushBuffer : register(b2)
{
    float4 BrushColor : packoffset(c0);
    float3 BrushPos : packoffset(c1);
    float BrushRange : packoffset(c1.w);
};

cbuffer TerrainData : register(b3)
{
    float2 TerrainSize : packoffset(c0);
};

cbuffer LightData : register(b4)
{
    matrix lightView;
    matrix lightProj;
    float3 lightPos;
}

Texture2D BrushTexture : register(t0);
SamplerState Samp : register(s0);

struct DOMAIN_INPUT
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

struct DS_VS_OUTPUT_PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 DepthPosition : POSITION1;
    float2 Uv : UV0;
    float2 bUv : UV1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float4 BrushColor : COLOR;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};


float4 Brush(float3 Position)
{
    float brushW = 0, brushH = 0;

    BrushTexture.GetDimensions(brushW, brushH);

    int minX, maxX, minZ, maxZ;

    minX = BrushPos.x - BrushRange;
    if (minX <= 0)
        minX = 0;

    maxX = BrushPos.x + BrushRange;
    if (maxX >= TerrainSize.x)
        maxX = TerrainSize.x;

    minZ = BrushPos.z - BrushRange;
    if (minZ <= 0)
        minZ = 0;
    maxZ = BrushPos.z + BrushRange;

    if (maxZ >= TerrainSize.y)
        maxZ = TerrainSize.y;
    
    if (Position.x < minX || Position.x > maxX
        || Position.z < minZ || Position.z > maxZ)
        return float4(0, 0, 0, 0);
    
    float2 uvBrush = float2((Position.x - (BrushPos.x - BrushRange)) / (2 * BrushRange), (Position.z - (BrushPos.z - BrushRange)) / (2 * BrushRange));
    float4 color = BrushTexture.SampleLevel(Samp, uvBrush, 0);

    return float4(0, color.g, 0, 0);
}


[domain("tri")]
DS_VS_OUTPUT_PS_INPUT DS(HS_CONSTANT_DATA_OUTPUT input,
                float3 domain : SV_DomainLocation,
                const OutputPatch<DOMAIN_INPUT, 3> TrianglePatch)
{
    DS_VS_OUTPUT_PS_INPUT Out;


    Out.Position = domain.x * TrianglePatch[0].Position +
                   domain.y * TrianglePatch[1].Position +
                   domain.z * TrianglePatch[2].Position;
    Out.Position.w = 1.0f;

    Out.Uv = domain.x * TrianglePatch[0].Uv +
            domain.y * TrianglePatch[1].Uv +
            domain.z * TrianglePatch[2].Uv;
   
    Out.Normal = domain.x * TrianglePatch[0].Normal +
                domain.y * TrianglePatch[1].Normal +
                domain.z * TrianglePatch[2].Normal;
   
   
    Out.Tangent = domain.x * TrianglePatch[0].Tangent +
                 domain.y * TrianglePatch[1].Tangent +
                 domain.z * TrianglePatch[2].Tangent;
   
    Out.Binormal = domain.x * TrianglePatch[0].Binormal +
                  domain.y * TrianglePatch[1].Binormal +
                  domain.z * TrianglePatch[2].Binormal;

    Out.bUv = float2(Out.Position.x / TerrainSize[0], Out.Position.z / TerrainSize[1]);
    Out.BrushColor = Brush(Out.Position.xyz);
    
    Out.Position.w = 1.0f;
    float4 worldPosition = mul(Out.Position, World);
    Out.Position = mul(worldPosition, View);
    Out.Position = mul(Out.Position, Proj);
    Out.DepthPosition = Out.Position;
  
    Out.Normal = mul(Out.Normal, (float3x3) World);
    Out.Tangent = mul(Out.Tangent, (float3x3) World);
    Out.Binormal = mul(Out.Binormal, (float3x3) World);
    
    //���� ���� VERTEX����
    Out.lightViewPosition = mul(worldPosition, lightView);
    Out.lightViewPosition = mul(Out.lightViewPosition, lightProj);
    Out.lightPos = lightPos.xyz - worldPosition.xyz;

    Out.lightPos = normalize(Out.lightPos);
    


    return Out;
}