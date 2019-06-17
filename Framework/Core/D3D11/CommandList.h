#pragma once

enum class CommandType : uint
{
	Unknown,
	Begin,
	End,
	Draw,
	DrawIndexed,
	SetVertexBuffer,
	SetIndexBuffer,
	SetPrimitiveTopology,
	SetInputLayout,
	SetVertexShader,
	SetPixelShader,
	SetConstantBuffers,
	SetShaderResources,
	SetSamplerStates,
	SetRasterizerState,
	SetBlendState,
	SetDepthStencilState,
	SetViewport,
	SetRenderTargets,
	ClearRenderTarget,
	ClearDepthStencil,
};

struct Command final
{
	Command();
	void Clear();

	CommandType command_type;
	std::string pass_name;

	ID3D11Buffer* vertex_buffer;
	uint vertex_count;
	uint vertex_offset;
	uint vertex_stride;

	ID3D11Buffer* index_buffer;
	uint index_count;
	uint index_offset;

	D3D11_PRIMITIVE_TOPOLOGY primitive_topology;
	ID3D11InputLayout* input_layout;
	ID3D11VertexShader* vertex_shader;
	ID3D11PixelShader* pixel_shader;

	std::vector<ID3D11Buffer*> cbuffers;
	ShaderStage cbuffer_shader_type;
	uint cbuffer_count;
	uint cbuffer_slot;

	std::vector<ID3D11ShaderResourceView*> srvs;
	ShaderStage srv_shader_type;
	uint srv_count;
	uint srv_slot;

	std::vector<ID3D11SamplerState*> samplers;
	ShaderStage sampler_shader_type;
	uint sampler_count;
	uint sampler_slot;

	ID3D11RasterizerState* rasterizer_state;
	ID3D11BlendState* blend_state;
	ID3D11DepthStencilState* depth_stencil_state;

	D3D11_VIEWPORT viewport;

	std::vector<ID3D11RenderTargetView*> render_targets;
	ID3D11DepthStencilView* depth_stencil_target;
	uint render_target_count;

	ID3D11RenderTargetView* clear_render_target;
	D3DXCOLOR clear_color;

	ID3D11DepthStencilView* clear_depth_stencil_target;
	float clear_depth;
	uint clear_stencil;
	uint clear_flags;
};

class CommandList final
{
public:
	CommandList(class Context* context);
	~CommandList();

	void Begin(const std::string& pass_name);
	void End();

	void Draw(const uint& vertex_count);
	void DrawIndexed(const uint& index_count, const uint& index_offset, const uint& vertex_offset);

	void SetVertexBuffer(VertexBuffer* buffer);
	void SetIndexBuffer(IndexBuffer* buffer);
	void SetPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY& primitive_topology);
	void SetInputLayout(InputLayout* input_layout);
	void SetVertexShader(VertexShader* shader);
	void SetPixelShader(PixelShader* shader);

	void SetConstantBuffer(const uint& slot, const ShaderStage& type, ConstantBuffer* buffer);
	void SetConstantBuffer(const uint& slot, const ShaderStage& type, ID3D11Buffer* buffer);
	void SetConstantBuffers(const uint& slot, const ShaderStage& type, const std::vector<ID3D11Buffer*>& buffers);

	void SetShaderResource(const uint& slot, const ShaderStage& type, Texture* texture);
	void SetShaderResource(const uint& slot, const ShaderStage& type, RenderTexture* texture);
	void SetShaderResource(const uint& slot, const ShaderStage& type, ID3D11ShaderResourceView* srv);
	void SetShaderResources(const uint& slot, const ShaderStage& type, const std::vector<ID3D11ShaderResourceView*>& srvs);
	void ClearShaderResources(const ShaderStage & type);

	void SetSamplerState(const uint& slot, const ShaderStage& type, Sampler* sampler);
	void SetSamplerState(const uint& slot, const ShaderStage& type, ID3D11SamplerState* sampler);
	void SetSamplerStates(const uint& slot, const ShaderStage& type, const std::vector<ID3D11SamplerState*>& samplers);

	void SetViewport(const D3D11_VIEWPORT& viewport);

	void SetRasterizerState(RasterizerState* rasterizer_state);
	void SetBlendState(BlendState* blend_state);
	void SetDepthStencilState(DepthStencilState* depth_stencil_state);

	void SetRenderTarget(ID3D11RenderTargetView* render_target, ID3D11DepthStencilView* depth_stencil_target = nullptr);
	void SetRenderTarget(RenderTexture* render_target, RenderTexture* depth_stencil_target = nullptr);
	void SetRenderTargets(const std::vector<ID3D11RenderTargetView*>& render_targets, ID3D11DepthStencilView* depth_stencil_target = nullptr);

	void ClearRenderTarget(ID3D11RenderTargetView* render_target, const D3DXCOLOR& clear_color = D3DXCOLOR(0, 0, 0, 0));
	void ClearRenderTarget(RenderTexture* render_target, const D3DXCOLOR& clear_color = D3DXCOLOR(0, 0, 0, 0));
	void ClearRenderTargets(const std::vector<ID3D11RenderTargetView*>& render_targets, const D3DXCOLOR& clear_color = D3DXCOLOR(0, 0, 0, 0));

	void ClearDepthStencilTarget(RenderTexture* render_target, const uint& flags, const float& clear_depth, const uint& clear_stencil = 0U);

	auto Submit() -> const bool;

private:
	auto GetCommand()->Command&;
	void Clear();

private:
	Graphics* graphics;
	std::vector<Command> commands;
	uint command_capacity;
	uint command_count;
};