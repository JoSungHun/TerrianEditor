#pragma once
#include "ISubsystem.h"

enum RendererOption : uint
{
	PostProcess_Bloom		= 1U << 0,
	PostProcess_FXAA		= 1U << 1,
	PostProcess_SSAO		= 1U << 2,
	PostProcess_TAA			= 1U << 3,
	PostProcess_Sharpening	= 1U <<4,
	PostProcess_SSR			= 1U << 6,
};

enum class RenderableType : uint
{
	Opaque,
	Transparent,
	Camera,
	Light,
};

enum class RendererDebugType : uint
{
	None,
	Albedo,
	Normal,
	Material,
	Velocity,
	Depth,
	SSAO,
};

class Renderer final : public ISubsystem
{
public:
	Renderer(class Context* context);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	const bool Initialize() override;

	void FlagsEnable(const RendererOption& flag) { render_flags |= flag; }
	void FlagsDisable(const RendererOption& flag) { render_flags &= ~flag; }
	auto IsOnFlag(const RendererOption& flag)-> const bool { return (render_flags & flag) > 0U; }

	auto GetFrameResource()->ID3D11ShaderResourceView*;

	auto GetMainCamera() const -> class Camera*;

	auto GetResolution() const -> const D3DXVECTOR2& { return resolution; }
	void SetResolution(const uint& width, const uint& height);
	
	auto GetCursorPos(D3DXVECTOR2& pos) -> const bool;
	void SetCursorPos(D3DXVECTOR2 pos) { mouse_pos = pos; }

	auto GetDebugType() const -> const RendererDebugType& { return debug_type; }
	void SetDebugType(const RendererDebugType& debug_type) { this->debug_type = debug_type; }

	void AcquireRenderables(class Scene* scene);
	void Clear();

	void DrawLine
	(
		const D3DXVECTOR3& from,
		const D3DXVECTOR3& to,
		const D3DXCOLOR& from_color = D3DXCOLOR(0.41f, 0.86f, 1.0f, 1.0f),
		const D3DXCOLOR& to_color = D3DXCOLOR(0.41f, 0.86f, 1.0f, 1.0f),
		const bool& bDepth = true
	);

	void Render();

private:
	void PassMain();
	void PassLightDepth();
	void PassGBuffer();
	void PassPreLight
	(
		std::shared_ptr<class RenderTexture>& in,
		std::shared_ptr<class RenderTexture>& shadow_out,
		std::shared_ptr<class RenderTexture>& ssao_out
	);
	void PassLight
	(
		std::shared_ptr<class RenderTexture>& shadow_in,
		std::shared_ptr<class RenderTexture>& ssao_in,
		std::shared_ptr<class RenderTexture>& out
	);
	void PassPostLight
	(
		std::shared_ptr<class RenderTexture>& in,
		std::shared_ptr<class RenderTexture>& out
	);
	void PassTransparent(std::shared_ptr<class RenderTexture>& out);

	void PassShadowMapping
	(
		std::shared_ptr<class RenderTexture>& out,
		class Light* light
	);

	void PassBoxBlur
	(
		std::shared_ptr<class RenderTexture>& in,
		std::shared_ptr<class RenderTexture>& out,
		const float& sigma
	);
	void PassGaussianBlur
	(
		std::shared_ptr<class RenderTexture>& in,
		std::shared_ptr<class RenderTexture>& out,
		const float& sigma,
		const float& pixel_stride = 1.0f
	);
	void PassBilateralGaussianBlur
	(
		std::shared_ptr<class RenderTexture>& in,
		std::shared_ptr<class RenderTexture>& out,
		const float& sigma,
		const float& pixel_stride = 1.0f
	);
	void PassBloom
	(
		std::shared_ptr<class RenderTexture>& in,
		std::shared_ptr<class RenderTexture>& out
	);
	void PassGammaCorrection
	(
		std::shared_ptr<class RenderTexture>& in,
		std::shared_ptr<class RenderTexture>& out
	);
	void PassSSAO(std::shared_ptr<class RenderTexture>& out);
	void PassDebug(std::shared_ptr<class RenderTexture>& out);

private:
	void CreateRenderTextures();
	void CreateTextures();
	void CreateShaders();
	void CreateConstantBuffers();
	void CreateSamplers();
	void CreateRasterizerStates();
	void CreateBlendStates();
	void CreateDepthStencilStates();

	void UpdateGlobalBuffer(const uint& width, const uint& height, const D3DXMATRIX& wvp = *D3DXMatrixIdentity(&D3DXMATRIX()));
	void UpdateShadowMapBuffer(class Light* light);
	void UpdateBlurBuffer(const D3DXVECTOR2& direction, const float& sigma);
	void UpdateLightBuffer();
	void UpdateLightCascadeBuffer(const D3DXMATRIX& wvp, const uint& index);
	void UpdateTransparentBuffer(const D3DXMATRIX& world, const D3DXCOLOR& color, const float& roughness);

	auto GetDirectionalLight() -> class Light*;
	auto GetClearDepth() const -> const float { return Settings::Get().IsReverseZ() ? 0.0f : 1.0f; }
	auto GetRasterizerState(const CullMode& cull_mode, const FillMode& fill_mode = FillMode::SOLID)-> class RasterizerState*;

private:
	//SCREEN POS
	D3DXVECTOR2 mouse_pos;

	// Camera Data
	D3DXMATRIX camera_view;
	D3DXMATRIX camera_proj;
	D3DXMATRIX camera_vp; // view * proj
	D3DXMATRIX camera_vp_inv;
	D3DXVECTOR3 camera_position;
	float camera_near;
	float camera_far;

	D3DXMATRIX postprocess_view;
	D3DXMATRIX postprocess_ortho;
	D3DXMATRIX postprocess_vp;

	uint render_flags;
	RendererDebugType debug_type;
	D3DXVECTOR2 resolution;

	float bloomIntensity;
	float blurSigma;

	D3DXVECTOR3 directional_avg_dir;

private:
	class Camera* sceneCamera;
	class Skybox* skybox;
	class Terrain* terrain;
	std::shared_ptr<class Camera> editorCamera;
	std::shared_ptr<class CommandList> commandList;
	std::unordered_map<RenderableType, std::vector<class Actor*>> renderables;

#pragma region Gizmo
	Rect gizmo_rectangle;
	std::shared_ptr<class Grid> gizmo_grid;
#pragma endregion
#pragma region Geometry
	std::shared_ptr<class VertexBuffer> screen_vertex_buffer;
	std::shared_ptr<class IndexBuffer> screen_index_buffer;
	std::shared_ptr<class VertexBuffer> line_vertex_buffer;
	std::vector<VertexColor> depth_enabled_line_vertices;
	std::vector<VertexColor> depth_disabled_line_vertices;
#pragma endregion
#pragma region Texture
private:
	std::shared_ptr<class Texture> noise_normal_texture;
	std::shared_ptr<class Texture> white_texture;
	std::shared_ptr<class Texture> black_texture;
	std::shared_ptr<class Texture> lut_ibl_texture;
	/*std::shared_ptr<class Texture> directional_gizmo_texture;
	std::shared_ptr<class Texture> point_gizmo_texture;
	std::shared_ptr<class Texture> spot_gizmo_texture;*/
#pragma endregion
#pragma region Shader
private:
	//vertex shader
	std::shared_ptr<class Shader> vs_gbuffer;
	std::shared_ptr<class Shader> vs_quad;
	std::shared_ptr<class Shader> vs_depth;

	//pixel shader
	std::shared_ptr<class Shader> ps_texture;
	std::shared_ptr<class Shader> ps_debug_normal;
	std::shared_ptr<class Shader> ps_debug_velocity;
	std::shared_ptr<class Shader> ps_debug_ssao;
	std::shared_ptr<class Shader> ps_debug_depth;

	std::shared_ptr<class Shader> ps_bloom_bright;
	std::shared_ptr<class Shader> ps_bloom_blend;

	std::shared_ptr<class Shader> ps_downsample_box;
	std::shared_ptr<class Shader> ps_upsample_box;

	std::shared_ptr<class Shader> ps_blur_box;
	std::shared_ptr<class Shader> ps_blur_gaussian;
	std::shared_ptr<class Shader> ps_blur_gaussian_bilateral;
	std::shared_ptr<class Shader> ps_gamma_correction;
	std::shared_ptr<class Shader> ps_ssao;

	std::shared_ptr<class Shader> ps_shadow_mapping_point;
	std::shared_ptr<class Shader> ps_shadow_mapping_spot;

	//vertex shader & pixel shader
	std::shared_ptr<class Shader> vps_shadow_mapping_directional;
	std::shared_ptr<class Shader> vps_light;
	std::shared_ptr<class Shader> vps_transparent;
	std::shared_ptr<class Shader> vps_color;

#pragma endregion
#pragma region ConstantBuffer
private:
	std::shared_ptr<class ConstantBuffer> global_buffer;
	std::shared_ptr<class ConstantBuffer> blur_buffer;
	std::shared_ptr<class ConstantBuffer> shadow_map_buffer;
	std::shared_ptr<class ConstantBuffer> light_buffer;
	std::shared_ptr<class ConstantBuffer> cascade_buffer;
	std::shared_ptr<class ConstantBuffer> transparent_buffer;
#pragma endregion
#pragma region RenderTexture
private:
	//GBuffer
	std::shared_ptr<class RenderTexture> gbuffer_albedo;
	std::shared_ptr<class RenderTexture> gbuffer_normal;
	std::shared_ptr<class RenderTexture> gbuffer_material;
	std::shared_ptr<class RenderTexture> gbuffer_velocity;
	std::shared_ptr<class RenderTexture> gbuffer_depth;

	//Full
	std::shared_ptr<class RenderTexture> full_hdr_target1;
	std::shared_ptr<class RenderTexture> full_hdr_target2;
	std::shared_ptr<class RenderTexture> full_spare;

	//Half
	std::shared_ptr<class RenderTexture> half_shadows;
	std::shared_ptr<class RenderTexture> half_ssao;
	std::shared_ptr<class RenderTexture> half_spare;
	std::shared_ptr<class RenderTexture> half_spare2;

	//Quarter
	std::shared_ptr<class RenderTexture> quarter_blur1;
	std::shared_ptr<class RenderTexture> quarter_blur2;
#pragma endregion
#pragma region RenderState
private:
	//Sampler State
	std::shared_ptr<class Sampler> compare_depth;
	std::shared_ptr<class Sampler> point_clamp;
	std::shared_ptr<class Sampler> bilinear_clamp;
	std::shared_ptr<class Sampler> bilinear_wrap;
	std::shared_ptr<class Sampler> trilinear_clamp;
	std::shared_ptr<class Sampler> anisotropic_wrap;

	//Rasterizer State
	std::shared_ptr<class RasterizerState> cull_none_solid;
	std::shared_ptr<class RasterizerState> cull_back_solid;
	std::shared_ptr<class RasterizerState> cull_front_solid;
	std::shared_ptr<class RasterizerState> cull_none_wireframe;
	std::shared_ptr<class RasterizerState> cull_back_wireframe;
	std::shared_ptr<class RasterizerState> cull_front_wireframe;

	//Blend State
	std::shared_ptr<class BlendState> blend_enabled;
	std::shared_ptr<class BlendState> blend_disabled;
	std::shared_ptr<class BlendState> blend_shadow_map;

	//DepthStencil State
	std::shared_ptr<class DepthStencilState> depth_stencil_enabled;
	std::shared_ptr<class DepthStencilState> depth_stencil_disabled;
#pragma endregion
};


