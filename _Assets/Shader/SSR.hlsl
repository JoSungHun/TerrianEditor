static const int global_step                        = 16;
static const int global_binary_search_step          = 20;
static const float global_binary_search_threshold  = 0.01f;
static const float2 global_failed                   = float2(-1.0f, -1.0f);

float2 SSR_BinarySearch(float3 ray_dir, inout float3 ray_pos, Texture2D depth_texture, SamplerState point_clamp)
{
    for (int i = 0; i < global_binary_search_step; i++)
    {
        float2 ray_uv       = Project(ray_pos, global_proj);
        float depth         = GetLinearDepth(depth_texture, point_clamp, ray_uv);
        float depth_delta   = ray_pos.z - depth;

        if(depth_delta <= 0.0f)
            ray_pos += ray_dir;

        ray_dir *= 0.5f;
        ray_pos -= ray_dir;
    }

    float2 ray_uv = Project(ray_pos, global_proj);
    float depth = GetLinearDepth(depth_texture, point_clamp, ray_uv);
    float depth_delta = ray_pos.z - depth;

    return abs(depth_delta) < global_binary_search_threshold ? Project(ray_pos, global_proj) : global_failed;
}

float2 SSR_RayMarch(float3 ray_pos, float3 ray_dir, Texture2D depth_texture, SamplerState point_clamp)
{
    for (int i = 0; i < global_step; i++)
    {
        ray_pos += ray_dir;
        float2 ray_uv = Project(ray_pos, global_proj);

        float depth_current = ray_pos.z;
        float depth_sample = GetLinearDepth(depth_texture, point_clamp, ray_uv);
        float depth_delta = ray_pos.z - depth_sample;

        [branch]
        if(depth_delta > 0.0f)
            return SSR_BinarySearch(ray_dir, ray_pos, depth_texture, point_clamp);
    }

    return global_failed;
}

float4 SSR(float3 position, float3 normal, Texture2D color_texture, Texture2D depth_texture, SamplerState point_clamp)
{
    float3 view_position    = mul(float4(position, 1.0f), global_view).xyz;
    float3 view_normal      = mul(float4(normal, 0.0f), global_view).xyz;
    float3 view_ray_dir     = normalize(reflect(view_position, view_normal));

    float3 ray_pos          = view_position;
    float2 reflection_uv    = SSR_RayMarch(ray_pos, view_ray_dir, depth_texture, point_clamp);
    float2 edge_factor      = float2(1, 1) - pow(saturate(abs(reflection_uv - float2(0.5f, 0.5f)) * 2.0f), 8);
    float screen_edge       = saturate(min(edge_factor.x, edge_factor.y));

    float alpha = 1.0f;
    if(reflection_uv.x + reflection_uv.y == -2.0f)
        alpha = 0.0f;

    return float4(color_texture.Sample(point_clamp, reflection_uv).rgb * screen_edge, alpha);
}