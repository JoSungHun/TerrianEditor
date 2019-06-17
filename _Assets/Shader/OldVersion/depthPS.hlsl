
struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float4 DepthPosition : POSITION;
};

float4 PS(PS_INPUT input) : SV_Target
{
    float depthValue;
	float4 color;
	
	depthValue = input.DepthPosition.z / input.DepthPosition.w;
	//depthValue = input.DepthPosition.z;

	color = float4(depthValue,depthValue,depthValue,1.0f);
	
	return color;
}

