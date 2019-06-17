
SamplerState samp : register(s0);
Texture2D reflectionTex : register(t0);
Texture2D refractionTex : register(t1);
Texture2D normalTex : register(t2);

cbuffer WaterData
{
    float waterTranslation;
    float reflectRefractScale;
};


struct PixelInput
{
    float4 Position : SV_POSITION0;
    float2 Uv : UV0;
    float4 reflectionPosition : TEXCOORD1;
    float4 refractionPosition : TEXCOORD2;
}; 

float4 PS(PixelInput input) : SV_Target
{
    float2 reflectTexCoord;
    float2 refractTexCoord;
    float4 normalMap;
    float3 normal;
    float4 reflectionColor;
    float4 refractionColor;
    float4 color;

	
	// Move the position the water normal is sampled from to simulate moving water.	
    input.Uv.y += waterTranslation;
	
	// Calculate the projected reflection texture coordinates.
    reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;
	
	// Calculate the projected refraction texture coordinates.
    refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

	// Sample the normal from the normal map texture.
    normalMap = normalTex.Sample(samp, input.Uv);

	// Expand the range of the normal from (0,1) to (-1,+1).
    normal = (normalMap.xyz * 2.0f) - 1.0f;

	// Re-position the texture coordinate sampling position by the normal map value to simulate the rippling wave effect.
    reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractScale);
    refractTexCoord = refractTexCoord + (normal.xy * reflectRefractScale);

	// Sample the texture pixels from the textures using the updated texture coordinates.
    reflectionColor = reflectionTex.Sample(samp, reflectTexCoord);
    refractionColor = refractionTex.Sample(samp, refractTexCoord);

	// Combine the reflection and refraction results for the final color.
    color = lerp(reflectionColor, refractionColor, 0.6f);
	
    return color;
}