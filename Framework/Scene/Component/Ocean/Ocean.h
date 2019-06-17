#pragma once

class Ocean final : public IComponent
{
public:
#pragma region BASIC Member Func
	Ocean(Context* context, class Actor* actor, class Transform* transform);
	~Ocean() = default;

	Ocean(const Ocean&) = delete;
	Ocean& operator=(const Ocean&) = delete;


		// IComponent을(를) 통해 상속됨
	virtual void Serialize(FileStream * stream) override;
	virtual void Deserialize(FileStream * stream) override;
	virtual void OnInitialize() override;
	virtual void OnStart() override;
	virtual void OnUpdate() override;
	virtual void OnStop() override;
	virtual void OnDestroy() override;
#pragma endregion

	void CreateOcean(uint width, uint height);
	void EditOceanSize(uint width, uint height);

	void UpdateOceanBuffer();

	//Getter^ & Setter
	void SetOceanHeight(const float& height);
	auto GetOceanHeight() const -> const float& { return ocean_height; }

	auto GetMaterial() -> class Material* { return material.get(); }
	

	
private:
	class Graphics* graphics;
	class Renderer* renderer;

	//Materials
	std::shared_ptr<class Material> material;

	//Ocean Data
	int width, height;
	float ocean_height;
	Geometry<VertexModel> geometry;
	VertexBuffer* vertex_buffer;
	IndexBuffer* index_buffer;



	std::shared_ptr<class ConstantBuffer> gpu_ocean_buffer;
	float ocean_translation;
	
	


};


#pragma region OldVersion
//class Ocean
//{
//public:
//	Ocean(Context* context);
//	~Ocean();
//
//	void ShaderReload();
//
//	void CreateOcean(uint width,uint height);
//	
//	const float& GetPosY() const { return posY; }
//	void SetPosY(const float& pos) { posY = pos; }
//	void SetReflectionTexture( RenderTexture* renderTexture) {	reflectionTexture = renderTexture ;}
//	void SetRefractionTexture(RenderTexture* renderTexture) { refractionTexture = renderTexture; }
//
//	void Update();
//	void Render(const GlobalData& cameraData, const D3DXMATRIX& reflectionMat);
//
//
//private:
//	Context* context;
//	Graphics* graphics;
//
//	uint width, height;
//	float posY;
//	Geometry<VertexTexture> geometry;
//	VertexBuffer* vertexBuffer;
//	IndexBuffer* indexBuffer;
//
//	Pipeline* pipeline;
//	RenderTexture* reflectionTexture;
//	RenderTexture* refractionTexture;
//
//	D3DXMATRIX world;
//
//	float waterTranslation;
//
//	ID3D11ShaderResourceView* normalMap;
//};
#pragma endregion