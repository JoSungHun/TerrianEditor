cbuffer CameraBuffer : register(b0)
{
	matrix View;
	matrix Proj;
};

cbuffer TransformBuffer : register(b1)
{
	matrix World;
};

cbuffer ReflectionBuffer
{
    matrix reflectionMatrix;
};

struct VertexInput
{
    float4 Position : POSITION;
    float2 Uv : UV0;
};

struct VertexOut
{
    float4 Position : SV_POSITION0;
    float2 Uv : UV0;
    float4 reflectionPosition : TEXCOORD1;
    float4 refractionPosition : TEXCOORD2;
};

VertexOut VS(VertexInput input)
{
    VertexOut output;

    input.Position.w = 1.0f;

    output.Position = mul(input.Position,World);
    output.Position = mul(output.Position,View);
    output.Position = mul(output.Position,Proj);

    output.Uv = input.Uv;


    matrix reflectProjectWorld;
    matrix viewProjectWorld;

    reflectProjectWorld = mul(reflectionMatrix, Proj);
    reflectProjectWorld = mul(World, reflectProjectWorld);

    output.reflectionPosition = mul(input.Position, reflectProjectWorld);

    viewProjectWorld = mul(View, Proj);
    viewProjectWorld = mul(World, viewProjectWorld);

    output.refractionPosition = mul(input.Position, viewProjectWorld);
    
    return output;
}
