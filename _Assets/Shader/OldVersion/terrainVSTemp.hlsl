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
}

struct VS_INPUT
{
    float4 Position : POSITION0;
    float2 Uv : UV0;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float3 Binormal : NORMAL2;
    float4 Color : COLOR0;
};

struct VS_OUT_PIXEL_INPUT
{
    float4 Position : SV_POSITION0;
    float4 depthPosition : POSITION1;
    float2 Uv : UV0;
    float2 bUv : UV1;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float3 Binormal : NORMAL2;
    float4 BrushColor : COLOR0;
};


Texture2D BrushTexture : register(t0);
SamplerState Samp : register(s0);


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

VS_OUT_PIXEL_INPUT VS(VS_INPUT input)
{
    /*
    float4 Position : SV_POSITION0;
    float4 depthPosition : POSITION1;
    float2 Uv : UV0;
    float2 bUv : UV1;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float3 Binormal : NORMAL2;
    float4 BrushColor : COLOR0;
    */
    VS_OUT_PIXEL_INPUT output;
    //output.Position = input.Position;
    //output.Uv = input.Uv;
    //output.Normal = input.Normal;
    //output.Tangent = input.Tangent;
    //output.Binormal = input.Binormal;

    input.Position.w = 1.0f;    

    output.bUv = float2(input.Position.x / TerrainSize[0], input.Position.z / TerrainSize[1]);

    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Proj);


    output.depthPosition = output.Position;
       
    output.Normal = normalize(mul(input.Normal, (float3x3) World));
    output.Binormal = normalize(mul(input.Binormal, (float3x3) World));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) World));     

    output.Uv = input.Uv;
    output.BrushColor = Brush(input.Position.xyz);

    return output;
}

