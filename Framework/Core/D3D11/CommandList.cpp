#include "Framework.h"
#include "CommandList.h"

Command::Command()
{
	render_targets.reserve(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	cbuffers.reserve(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
	srvs.reserve(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
	samplers.reserve(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
	Clear();
}

void Command::Clear()
{
	command_type = CommandType::Unknown;
	pass_name = "N/A";

	vertex_buffer = nullptr;
	vertex_count = 0;
	vertex_offset = 0;
	vertex_stride = 0;

	index_buffer = nullptr;
	index_count = 0;
	index_offset = 0;

	primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	input_layout = nullptr;
	vertex_shader = nullptr;
	pixel_shader = nullptr;

	cbuffers.clear();
	cbuffer_shader_type = ShaderStage::Unknown;
	cbuffer_count = 0;
	cbuffer_slot = 0;

	srvs.clear();
	srv_shader_type = ShaderStage::Unknown;
	srv_count = 0;
	srv_slot = 0;

	samplers.clear();
	sampler_shader_type = ShaderStage::Unknown;
	sampler_count = 0;
	sampler_slot = 0;

	rasterizer_state = nullptr;
	blend_state = nullptr;
	depth_stencil_state = nullptr;

	render_targets.clear();
	depth_stencil_target = nullptr;
	render_target_count = 0;

	clear_render_target = nullptr;
	clear_color = D3DXCOLOR(0xff555566);

	clear_depth_stencil_target = nullptr;
	clear_depth = 0.0f;
	clear_stencil = 0;
	clear_flags = 0;
}

CommandList::CommandList(Context * context)
	: command_capacity(2500)
	, command_count(0)
{
	commands.reserve(command_capacity);
	commands.resize(command_capacity);

	graphics = context->GetSubsystem<Graphics>();
}

CommandList::~CommandList()
{
	Clear();
}

void CommandList::Begin(const std::string & pass_name)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::Begin;
	cmd.pass_name = pass_name;
}

void CommandList::End()
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::End;
}

void CommandList::Draw(const uint & vertex_count)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::Draw;
	cmd.vertex_count = vertex_count;
}

void CommandList::DrawIndexed(const uint & index_count, const uint & index_offset, const uint & vertex_offset)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::DrawIndexed;
	cmd.index_count = index_count;
	cmd.index_offset = index_offset;
	cmd.vertex_offset = vertex_offset;
}

void CommandList::SetVertexBuffer(VertexBuffer * buffer)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetVertexBuffer;
	cmd.vertex_buffer = buffer->GetBuffer();
	cmd.vertex_count = buffer->GetVertexCount();
	cmd.vertex_offset = buffer->GetOffset();
	cmd.vertex_stride = buffer->GetStride();
}

void CommandList::SetIndexBuffer(IndexBuffer * buffer)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetIndexBuffer;
	cmd.index_buffer = buffer->GetBuffer();
	cmd.index_count = buffer->GetIndexCount();
	cmd.index_offset = buffer->GetOffset();
}

void CommandList::SetPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY & primitive_topology)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetPrimitiveTopology;
	cmd.primitive_topology = primitive_topology;
}

void CommandList::SetInputLayout(InputLayout * input_layout)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetInputLayout;
	cmd.input_layout = input_layout->GetLayout();
}

void CommandList::SetVertexShader(VertexShader * shader)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetVertexShader;
	cmd.vertex_shader = shader->GetShader();
}

void CommandList::SetPixelShader(PixelShader * shader)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetPixelShader;
	cmd.pixel_shader = shader ? shader->GetShader() : nullptr;
}

void CommandList::SetConstantBuffer(const uint & slot, const ShaderStage & type, ConstantBuffer * buffer)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetConstantBuffers;
	cmd.cbuffer_slot = slot;
	cmd.cbuffer_shader_type = type;
	cmd.cbuffers.emplace_back(buffer->GetBuffer());
	cmd.cbuffer_count = cmd.cbuffers.size();
}

void CommandList::SetConstantBuffer(const uint & slot, const ShaderStage & type, ID3D11Buffer * buffer)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetConstantBuffers;
	cmd.cbuffer_slot = slot;
	cmd.cbuffer_shader_type = type;
	cmd.cbuffers.emplace_back(buffer);
	cmd.cbuffer_count = cmd.cbuffers.size();
}

void CommandList::SetConstantBuffers(const uint & slot, const ShaderStage & type, const std::vector<ID3D11Buffer*>& buffers)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetConstantBuffers;
	cmd.cbuffer_slot = slot;
	cmd.cbuffer_shader_type = type;
	cmd.cbuffers = buffers;
	cmd.cbuffer_count = cmd.cbuffers.size();
}

void CommandList::SetShaderResource(const uint & slot, const ShaderStage & type, Texture * texture)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetShaderResources;
	cmd.srv_slot = slot;
	cmd.srv_shader_type = type;
	cmd.srvs.emplace_back(texture->GetShaderResourceView());
	cmd.srv_count = cmd.srvs.size();
}

void CommandList::SetShaderResource(const uint & slot, const ShaderStage & type, RenderTexture * texture)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetShaderResources;
	cmd.srv_slot = slot;
	cmd.srv_shader_type = type;
	cmd.srvs.emplace_back(texture->GetShaderResourceView());
	cmd.srv_count = cmd.srvs.size();
}

void CommandList::SetShaderResource(const uint & slot, const ShaderStage & type, ID3D11ShaderResourceView * srv)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetShaderResources;
	cmd.srv_slot = slot;
	cmd.srv_shader_type = type;
	cmd.srvs.emplace_back(srv);
	cmd.srv_count = cmd.srvs.size();
}

void CommandList::SetShaderResources(const uint & slot, const ShaderStage & type, const std::vector<ID3D11ShaderResourceView*>& srvs)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetShaderResources;
	cmd.srv_slot = slot;
	cmd.srv_shader_type = type;
	cmd.srvs = srvs;
	cmd.srv_count = cmd.srvs.size();
}

void CommandList::ClearShaderResources(const ShaderStage & type)
{
	auto empty_textures = std::vector<ID3D11ShaderResourceView*>(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
	SetShaderResources(0, type, empty_textures);
}

void CommandList::SetSamplerState(const uint & slot, const ShaderStage & type, Sampler * sampler)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetSamplerStates;
	cmd.sampler_slot = slot;
	cmd.sampler_shader_type = type;
	cmd.sampler_count = 1;
	cmd.samplers.emplace_back(sampler->GetState());
}

void CommandList::SetSamplerState(const uint & slot, const ShaderStage & type, ID3D11SamplerState * sampler)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetSamplerStates;
	cmd.sampler_slot = slot;
	cmd.sampler_shader_type = type;
	cmd.sampler_count = 1;
	cmd.samplers.emplace_back(sampler);
}

void CommandList::SetSamplerStates(const uint & slot, const ShaderStage & type, const std::vector<ID3D11SamplerState*>& samplers)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetSamplerStates;
	cmd.sampler_slot = slot;
	cmd.sampler_shader_type = type;
	cmd.sampler_count = samplers.size();
	cmd.samplers = samplers;
}

void CommandList::SetViewport(const D3D11_VIEWPORT & viewport)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetViewport;
	cmd.viewport = viewport;
}

void CommandList::SetRasterizerState(RasterizerState * rasterizer_state)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetRasterizerState;
	cmd.rasterizer_state = rasterizer_state->GetState();
}

void CommandList::SetBlendState(BlendState * blend_state)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetBlendState;
	cmd.blend_state = blend_state->GetState();
}

void CommandList::SetDepthStencilState(DepthStencilState * depth_stencil_state)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetDepthStencilState;
	cmd.depth_stencil_state = depth_stencil_state->GetState();
}

void CommandList::SetRenderTarget(ID3D11RenderTargetView * render_target, ID3D11DepthStencilView * depth_stencil_target)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetRenderTargets;
	cmd.depth_stencil_target = depth_stencil_target;
	cmd.render_targets.emplace_back(render_target);
	cmd.render_target_count = cmd.render_targets.size();
}

void CommandList::SetRenderTarget(RenderTexture * render_target, RenderTexture * depth_stencil_target)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetRenderTargets;
	cmd.depth_stencil_target = depth_stencil_target ? depth_stencil_target->GetDepthStencilView() : nullptr;
	cmd.render_targets.emplace_back(render_target ? render_target->GetRenderTargetView() : nullptr);
	cmd.render_target_count = cmd.render_targets.size();
}

void CommandList::SetRenderTargets(const std::vector<ID3D11RenderTargetView*>& render_targets, ID3D11DepthStencilView * depth_stencil_target)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::SetRenderTargets;
	cmd.depth_stencil_target = depth_stencil_target;
	cmd.render_targets = render_targets;
	cmd.render_target_count = render_targets.size();
}

void CommandList::ClearRenderTarget(ID3D11RenderTargetView * render_target, const D3DXCOLOR & clear_color)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::ClearRenderTarget;
	cmd.clear_render_target = render_target;
	cmd.clear_color = clear_color;
}

void CommandList::ClearRenderTarget(RenderTexture * render_target, const D3DXCOLOR & clear_color)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::ClearRenderTarget;
	cmd.clear_render_target = render_target->GetRenderTargetView();
	cmd.clear_color = clear_color;
}

void CommandList::ClearRenderTargets(const std::vector<ID3D11RenderTargetView*>& render_targets, const D3DXCOLOR & clear_color)
{
	for (const auto& render_target : render_targets)
		ClearRenderTarget(render_target, clear_color);
}

void CommandList::ClearDepthStencilTarget(RenderTexture * render_target, const uint & flags, const float & clear_depth, const uint & clear_stencil)
{
	auto& cmd = GetCommand();
	cmd.command_type = CommandType::ClearDepthStencil;
	cmd.clear_depth_stencil_target = render_target->GetDepthStencilView();
	cmd.clear_flags = flags;
	cmd.clear_depth = clear_depth;
	cmd.clear_stencil = clear_stencil;
}

auto CommandList::Submit() -> const bool
{
	auto device_context = graphics->GetDeviceContext();

	for (uint i = 0; i < command_count; i++)
	{
		auto& cmd = commands[i];
		switch (cmd.command_type)
		{
		case CommandType::Begin:
			break;
		case CommandType::End:
			break;
		case CommandType::Draw:
			device_context->Draw(cmd.vertex_count, 0);
			break;
		case CommandType::DrawIndexed:
			device_context->DrawIndexed(cmd.index_count, cmd.index_offset, cmd.vertex_offset);
			break;
		case CommandType::SetVertexBuffer:
			device_context->IASetVertexBuffers(0, 1, &cmd.vertex_buffer, &cmd.vertex_stride, &cmd.vertex_offset);
			break;
		case CommandType::SetIndexBuffer:
			device_context->IASetIndexBuffer(cmd.index_buffer, DXGI_FORMAT_R32_UINT, cmd.index_offset);
			break;
		case CommandType::SetPrimitiveTopology:
			device_context->IASetPrimitiveTopology(cmd.primitive_topology);
			break;
		case CommandType::SetInputLayout:
			device_context->IASetInputLayout(cmd.input_layout);
			break;
		case CommandType::SetVertexShader:
			device_context->VSSetShader(cmd.vertex_shader, nullptr, 0);
			break;
		case CommandType::SetPixelShader:
			device_context->PSSetShader(cmd.pixel_shader, nullptr, 0);
			break;
		case CommandType::SetConstantBuffers:
		{
			switch (cmd.cbuffer_shader_type)
			{
			case ShaderStage::Global:
			case ShaderStage::VS: device_context->VSSetConstantBuffers(cmd.cbuffer_slot, cmd.cbuffer_count, cmd.cbuffers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::HS: device_context->HSSetConstantBuffers(cmd.cbuffer_slot, cmd.cbuffer_count, cmd.cbuffers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::DS: device_context->DSSetConstantBuffers(cmd.cbuffer_slot, cmd.cbuffer_count, cmd.cbuffers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::GS: device_context->GSSetConstantBuffers(cmd.cbuffer_slot, cmd.cbuffer_count, cmd.cbuffers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
			case ShaderStage::PS: device_context->PSSetConstantBuffers(cmd.cbuffer_slot, cmd.cbuffer_count, cmd.cbuffers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::CS: device_context->CSSetConstantBuffers(cmd.cbuffer_slot, cmd.cbuffer_count, cmd.cbuffers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
			}
			break;
		}
		case CommandType::SetShaderResources:
		{
			switch (cmd.srv_shader_type)
			{
			case ShaderStage::Global:
			case ShaderStage::VS: device_context->VSSetShaderResources(cmd.srv_slot, cmd.srv_count, cmd.srvs.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::HS: device_context->HSSetShaderResources(cmd.srv_slot, cmd.srv_count, cmd.srvs.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::DS: device_context->DSSetShaderResources(cmd.srv_slot, cmd.srv_count, cmd.srvs.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::GS: device_context->GSSetShaderResources(cmd.srv_slot, cmd.srv_count, cmd.srvs.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
			case ShaderStage::PS: device_context->PSSetShaderResources(cmd.srv_slot, cmd.srv_count, cmd.srvs.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::CS: device_context->CSSetShaderResources(cmd.srv_slot, cmd.srv_count, cmd.srvs.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
			}
			break;
		}
		case CommandType::SetSamplerStates:
		{
			switch (cmd.sampler_shader_type)
			{
			case ShaderStage::Global:
			case ShaderStage::VS: device_context->VSSetSamplers(cmd.sampler_slot, cmd.sampler_count, cmd.samplers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::HS: device_context->HSSetSamplers(cmd.sampler_slot, cmd.sampler_count, cmd.samplers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::DS: device_context->DSSetSamplers(cmd.sampler_slot, cmd.sampler_count, cmd.samplers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::GS: device_context->GSSetSamplers(cmd.sampler_slot, cmd.sampler_count, cmd.samplers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
			case ShaderStage::PS: device_context->PSSetSamplers(cmd.sampler_slot, cmd.sampler_count, cmd.samplers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
				//case ShaderStage::CS: device_context->CSSetSamplers(cmd.sampler_slot, cmd.sampler_count, cmd.samplers.data()); if (cmd.cbuffer_shader_type != ShaderStage::Global) break;
			}
			break;
		}
		case CommandType::SetRasterizerState:
			device_context->RSSetState(cmd.rasterizer_state);
			break;
		case CommandType::SetBlendState:
		{
			float blend_factor[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
			device_context->OMSetBlendState(cmd.blend_state, blend_factor, 0xffffffff);
			break;
		}
		case CommandType::SetViewport:
			device_context->RSSetViewports(1, &cmd.viewport);
			break;
		case CommandType::SetRenderTargets:
			device_context->OMSetRenderTargets(cmd.render_target_count, cmd.render_targets.data(), cmd.depth_stencil_target);
			break;
		case CommandType::SetDepthStencilState:
			device_context->OMSetDepthStencilState(cmd.depth_stencil_state, 1);
			break;
		case CommandType::ClearRenderTarget:
			device_context->ClearRenderTargetView(cmd.clear_render_target, cmd.clear_color);
			break;
		case CommandType::ClearDepthStencil:
			device_context->ClearDepthStencilView(cmd.clear_depth_stencil_target, cmd.clear_flags, cmd.clear_depth, cmd.clear_stencil);
			break;
		}
	}

	Clear();
	return true;
}

auto CommandList::GetCommand() -> Command &
{
	if (command_count >= commands.size())
	{
		uint new_size = command_count + 100;
		commands.reserve(new_size);
		commands.resize(new_size);

		LOG_WARNING_F
		(
			"Command list has grown to fit %d command. Consider making the capacity larger to avoid re-allocations.",
			command_count + 1
		);
	}

	return commands[command_count++];
}

void CommandList::Clear()
{
	for (uint i = 0; i < command_count; i++)
		commands[i].Clear();

	command_count = 0;
}
