cbuffer CameraBuffer
{
	matrix viewMatrix;
	matrix projectionMatrix;
}

cbuffer WorldBuffer
{
	matrix worldMatrix;
}

struct VS_INPUT
{
    float4 Position : POSITION0;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
	float4 DepthPosition : POSITION0;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

	input.Position.w = 1.0f;
	
	output.Position = mul(input.Position,worldMatrix);
	output.Position = mul(output.Position,viewMatrix);
	output.Position = mul(output.Position,projectionMatrix);
	
	output.DepthPosition = output.Position;
    
    return output;
}

