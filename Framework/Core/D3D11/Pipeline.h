#pragma once

class Pipeline
{
public:
	Pipeline(class Context* context);
	~Pipeline();

protected:
	struct CBuffer
	{
		ConstantBuffer* buffer;
		int type;
	};

	

	
//Setup Pipeline
public:
	void SetVertexBuffer(VertexBuffer* vertexBuffer) { this->vertexBuffer = vertexBuffer; }
	void SetInstanceBuffer(InstanceBuffer* instanceBuffer) { this->instanceBuffer = instanceBuffer; }
	void SetindexBuffer(IndexBuffer* indexBuffer) { this->indexBuffer = indexBuffer; }
	void SetInputLayout(D3D11_INPUT_ELEMENT_DESC* descs,const uint& count);
	void SetTopology(const D3D11_PRIMITIVE_TOPOLOGY& topology) { this->topology = topology; }

	void SetShader(const ShaderType& type, const std::string& shaderFileName);
	void SetSRVSlot(const ShaderType& type, const uint& slotNum);
	void SetSRVSlot(const uint& vsSlot, const uint& psSlot, const uint& hsSlot = 0, const uint& dsSlot = 0, const uint& gsSlot = 0);
	void SetShaderResourceView(const ShaderType& type, const uint& slotNum, ID3D11ShaderResourceView* srv);
	void SetShaderResourceView(const ShaderType& type, const uint& slotNum, Texture* texture);
	void SetShaderResourceView(const ShaderType& type, const uint& slotNum, const std::string& fileName);

	template<typename T >
	void SetConstantBuffer(const ShaderType& type);

	template<typename T>
	void SetConstantBufferData(const ShaderType & type, const T& data);

	void SetRSCullMode(const D3D11_CULL_MODE& cullmode);
	void SetRSFillMode(const D3D11_FILL_MODE& fillMode);

	void Bind();
	void UnBind();
public:
	

private: 
	class Context* context;
	class Graphics* graphics;

	//IAStage
	D3D11_PRIMITIVE_TOPOLOGY topology;
	InputLayout* inputLayout;
	VertexBuffer* vertexBuffer;
	InstanceBuffer* instanceBuffer;
	IndexBuffer* indexBuffer;

	//VS Stage
	VertexShader* vertexShader;
	std::vector<CBuffer*> vsConstantBuffers;
	std::vector<ID3D11ShaderResourceView*> vsShaderResourceViews;

	//HS Stage
	HullShader* hullShader;
	std::vector<CBuffer*> hsConstantBuffers;
	std::vector<ID3D11ShaderResourceView*> hsShaderResourceViews;

	//DS Stage
	DomainShader* domainShader;
	std::vector<CBuffer*> dsConstantBuffers;
	std::vector<ID3D11ShaderResourceView*> dsShaderResourceViews;

	//GS Stage
	//GeometryShader* geometryShader;
	
	//PS Stage
	PixelShader* pixelShader;
	std::vector<CBuffer*> psConstantBuffers;
	std::vector<ID3D11ShaderResourceView*> psShaderResourceViews;


	RasterizerState* rsState;



};



template<typename T>
inline void Pipeline::SetConstantBuffer(const ShaderType & type)
{
	CBuffer* constantBuffer;
	
	constantBuffer = new CBuffer;
	constantBuffer->buffer = new ConstantBuffer(context);
	constantBuffer->buffer->Create<T>();
	constantBuffer->type = typeid(T).hash_code();

	switch (type)
	{
	case ShaderType::VS:
		vsConstantBuffers.push_back(constantBuffer);
		break;
	case ShaderType::HS:
		hsConstantBuffers.push_back(constantBuffer);
		break;
	case ShaderType::DS:
		dsConstantBuffers.push_back(constantBuffer);
		break;
	case ShaderType::GS:
		break;
	case ShaderType::PS:
		psConstantBuffers.push_back(constantBuffer);
		break;
	default:
		break;
	}
}

template<typename T>
inline  void Pipeline::SetConstantBufferData(const ShaderType & type, const T& data)
{
	std::vector<CBuffer*>* cbuffers;

	switch (type)
	{
	case ShaderType::VS:
		cbuffers = &vsConstantBuffers;
		break;
	case ShaderType::HS:
		cbuffers = &hsConstantBuffers;
		break;
	case ShaderType::DS:
		cbuffers = &dsConstantBuffers;
		break;
	case ShaderType::GS:
		break;
	case ShaderType::PS:
		cbuffers = &psConstantBuffers;
		break;
	default:
		break;
	}

	for (auto buffer : *cbuffers)
	{
		if (buffer->type != typeid(T).hash_code())
			continue;

		T* mappedData = static_cast<T*>(buffer->buffer->Map());
		*mappedData = data;
		buffer->buffer->Unmap(); 

	} 
}
