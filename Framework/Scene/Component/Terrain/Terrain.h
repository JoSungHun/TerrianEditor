#pragma once

enum class ChannelType : uint
{
	Diffuse = 0,
	Normal
};

enum class SplatChannel : uint
{
	Default = 0,
	R,
	G,
	B,
	A
};

enum class BrushType : uint
{
	BRUSH_NONE = 0,
	BRUSH_HEIGHT,
	BRUSH_FLAT,
	BRUSH_PAINTING
};


class Terrain final : public IComponent
{

public:
#pragma region BASIC Member Func
	Terrain
	(
		class Context* context,
		class Actor* actor,
		class Transform* transform
	);
	~Terrain() = default;

	Terrain(const Terrain&) = delete;
	Terrain& operator=(const Terrain&) = delete;

	void Serialize(FileStream* stream) override;
	void Deserialize(FileStream* stream) override;

	void OnInitialize() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;
	void OnDestroy() override;
#pragma endregion

	
//==============================================================
// GETTERS & SETTERS
//==============================================================
	
// Splat Map
	auto GetSplatChannelShaderResourceView(const SplatChannel& channel, const ChannelType& type)->ID3D11ShaderResourceView*;
	auto GetSplatMapShaderResourceView()->ID3D11ShaderResourceView* { return splat_map->GetShaderResourceView(); }

	void SetChannelTexture(const SplatChannel& channel,const ChannelType& type, Texture* albedo);
	auto GetChannelTexture(const SplatChannel& channel, const ChannelType& type) -> class Texture*;

// Brush Datas
	void SetBrushTexture(class Texture* texture) { brush_texture = texture; }
	auto GetBrushTexture() -> class Texture* { return brush_texture; }
	auto GetBrushSharderResourceView() -> ID3D11ShaderResourceView* { return brush_texture ? brush_texture->GetShaderResourceView() : nullptr; }

	void SetBrushType(const BrushType& type) { brush_type = type; }
	auto GetBrushType() const -> const BrushType& { return brush_type; }
	void SetBrushRange(const float& range) { brush_range = range; }
	auto GetBrushRange() const -> const float& { return brush_range; }
	void SetBrushIntensity(const float& intensity) { brush_intensity = intensity; }
	auto GetBrushIntensity() const -> const float& { return brush_intensity; }
	void SetBrushChannel(const SplatChannel& channel) { brush_channel_type = channel; }
	auto GetBrushChannel() { return brush_channel_type; }


// Terrain Data
	auto GetTerrainBuffer() const -> class ConstantBuffer* { return gpu_terrain_buffer.get(); }
	auto GetTerrainWidth() const -> const uint& { return width; }
	auto GetTerrainHeight() const -> const uint& { return height; }

//==============================================================
// For Create & Updates Terrains
//==============================================================
public:
	void CreateFromHeightMap(const std::string& paths);
	void Create(const uint& width, const uint& height);
	void Clear();

	void UpdateTerrainBuffer();

private:
	void ReadPixels(IN const std::string& filePath, OUT std::vector<D3DXCOLOR>& pixels, OUT uint& width, OUT uint& height);
	void SetupGeometry(IN std::vector<D3DXCOLOR>& heightMap, IN uint width, IN uint height);
	void CalculateTangent();
	void UpdateNormal();


//=====================================================
//	Picking & Brush
//=====================================================
private:
	//Picking
	auto Picking(D3DXVECTOR3& position) -> bool;
	auto UpdatePickBuffer() -> bool;
	
	//Brush
	void Painting(const D3DXVECTOR3& picked_position);
	void UpdateBrushBuffer(const D3DXVECTOR3& picked_position);


//=====================================================
//	Member Variables
//=====================================================
private:
	class Graphics* graphics;
	class Renderer* renderer;
	
	//Materials
	std::shared_ptr<class Texture> splat_map;
	std::shared_ptr<class Material> material;
	std::array<class Texture*, 5> splat_channel_normal;
	std::array<class Texture*, 5> splat_channel_diffuse;
	
	//Terrain Datas
	Geometry<VertexModel> geometry;
	class VertexBuffer* vertex_buffer;
	class IndexBuffer* index_buffer;

	TerrainData terrainData;
	std::shared_ptr<class ConstantBuffer> gpu_terrain_buffer;

	//Terrain Info
	uint width;
	uint height;


	//Compute Picking 
	class ConstantBuffer* gpu_picking_buffer;
	class ComputeBuffer* picking_output_buffer;
	class ComputeBuffer* picking_input_buffer;
	class ComputeShader* compute_picking;

	//Brush
	bool bPicked;

	D3DXVECTOR3 picked_pos;
	BrushType brush_type;
	SplatChannel brush_channel_type;
	float brush_range;
	float brush_intensity;

	class Texture* brush_texture;
	class ConstantBuffer* gpu_brush_buffer;
	class ComputeBuffer* brush_output_height_buffer;
	class ComputeBuffer* brush_output_texture_buffer;
	class ComputeBuffer* brush_input_buffer;
	class ComputeShader* compute_brush;

	//Terrain Struct Datas
	struct TerrainPickData
	{
		D3DXVECTOR3 Ray_origin;
		float padding1;
		D3DXVECTOR3 Ray_direction;
		float padding2;
		uint Terrain_width;
		uint Terrain_height;
		float padding3[2];
	};

	struct TerrainBrushData
	{
		D3DXVECTOR3 brush_position;
		float brush_range;
		float brush_intensity;
		float brush_type;
		float terrain_width;
		float terrain_height;
		float channel_index;
		float padding[3];
	};

	struct PickOutputData
	{
		D3DXVECTOR3 position;
		float distance;
		int bPicked;
	};
};

