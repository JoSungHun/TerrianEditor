cbuffer CameraBuffer : register(b0)
{
	matrix View;
	matrix Proj;
};

cbuffer TransformBuffer : register(b1)
{
	matrix World;
};

cbuffer LightBuffer : register(b2)
{
    matrix lightView;
    matrix lightProj;
    float3 lightPos;
};

struct VertexInput
{
	float4 Position : POSITION0;
	float4 Color : COLOR0;
};

struct VertexOut
{
	float4 Position : SV_POSITION0;
    float4 Color : COLOR0;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

VertexOut VS(VertexInput input)
{
    VertexOut output;
	output.Position = mul(input.Position, World);
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Proj);

    output.Color = input.Color;

    //광원기준 VERTEX 위치 계산
    output.lightViewPosition = mul(input.Position, World);
    output.lightViewPosition = mul(output.lightViewPosition, lightView);
    output.lightViewPosition = mul(output.lightViewPosition, lightProj);

    float4 worldPosition = mul(input.Position, World);

    output.lightPos = lightPos.xyz - worldPosition.xyz;

    output.lightPos = normalize(output.lightPos);

    return output;
}
