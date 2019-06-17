//BRDF - Bidirectional Reflectance Distribution Function
//-반사율 분포를 출력하는 함수여야함
//-양방향성 광원을 가져야함
//-광원과 관찰자를 입력 매개변수로 받음
//-함수여야함 - 같은 매개변수에서는 항상 같은값이 나와야함

//-Diffuse BRDF
//Lambert's Cosine Law
//-> L dot N = cos theta;
//-> L dot N / pi

//-Specular BRDF
//Shell's Law
//Fresnel Equation -> Shilck's approximation

//Fresnel at Degree 0 ---> F0

//============================================================================
//F Function : Schlick's approximation
//============================================================================
float3 F_Schlick(float Hdv, float3 f0)
{
    return f0 + (1.0f - f0) * pow(1.0f - Hdv, 5.0f);
}

//============================================================================
//G Function : Gemetric shadowing
//============================================================================
float G_GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float G_GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2 = G_GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = G_GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

//============================================================================
//D Function : Normal distribution
//============================================================================
float D_GGX(float NdotH, float a)
{
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return num / denom;
}

//============================================================================
//BRDF - Diffuse
//============================================================================
float3 Diffuse_OrenNayar(float3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH)
{
    float a = Roughness * Roughness;
    float s = a;
    float s2 = s * s;
    float VoL = 2 * VoH * VoH - 1;
    float Cosri = VoL - NoV * NoL;
    float C1 = 1 - 0.5 * s2 / (s2 + 0.33);
    float C2 = 0.45 * s2 / (s2 + 0.09) * Cosri * (Cosri >= 0 ? rcp(max(NoL, NoV + 0.0001f)) : 1);
    return DiffuseColor / PI * (C1 + C2) * (1 + Roughness * 0.5);
}

float3 BRDF_Diffuse(Material material, float n_dot_v, float n_dot_l, float v_dot_h)
{
    return Diffuse_OrenNayar(material.albedo, material.roughness, n_dot_v, n_dot_l, v_dot_h);
}

//============================================================================
//BRDF - Specular
//============================================================================
float3 BRDF_Specular(Material material, float n_dot_v, float n_dot_l, float n_dot_h, float v_dot_h, out float3 F)
{
    F = F_Schlick(v_dot_h, material.F0);
    float G = G_GeometrySmith(n_dot_v, n_dot_l, material.roughness);
    float D = D_GGX(n_dot_h, material.roughness_alpha);
    float3 nominator = F * G * D;
    float denominator = 4.0f * n_dot_l * n_dot_v;
    return nominator / max(0.00001f, denominator);
}

//============================================================================
//BRDF
//============================================================================
float3 BRDF(Material material, Light light, float3 normal, float3 camera_to_pixel)
{
    float3 h = normalize(light.direction - camera_to_pixel);
    float n_dot_v = saturate(dot(normal, -camera_to_pixel));
    float n_dot_l = saturate(dot(normal, light.direction));
    float n_dot_h = saturate(dot(normal, h));
    float v_dot_h = saturate(dot(-camera_to_pixel, h));
	
	// BRDF 요소들
    float3 cDiffuse = BRDF_Diffuse(material, n_dot_v, n_dot_l, v_dot_h);
    float3 f = 0.0f;
    float3 cSpecular = BRDF_Specular(material, n_dot_v, n_dot_l, n_dot_h, v_dot_h, f);
	
    float3 kS = f; //반사되는 빛의 에너지
    float3 kD = 1.0f - kS; // 남은 에너지, 굴절되는 빛
    kD *= 1.0f - material.metallic;
	
    float3 radiance = light.color * light.intensity;
    return (kD * cDiffuse + cSpecular) * radiance * n_dot_l;
}
