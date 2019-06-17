#pragma once

enum BrushType
{
	BRUSH_NONE = 0,
	BRUSH_HEIGHT,
	BRUSH_FLAT,
	BRUSH_TEXTURE,
};



class TerrainBrush
{
public:
	TerrainBrush(class Context* context);
	~TerrainBrush();

public:
	const float& GetBrushRange() const { return  paintData.Range; }
	const float& GetBrushIntensity() const { return  paintData.Intensity; }

	//Active
	void SetBrushActive(const bool& active) { this->active = active; }

//SetBrushData
	void SetBrushType(const BrushType& type)		{ paintData.BrushType = type; }
	void SetBrushRange(const float& range)			{ paintData.Range = range; }
	void SetBrushIntensity(const float& intensity)	{ paintData.Intensity = intensity; }
	void SetBrush(Texture* tex)						{ brushTex = tex; }
	void SetTerrainSize(const uint& width, const uint& height) { paintData.trSize[0] = width; paintData.trSize[1] = height; }

//Picked Position
	void SetBrushPosition(const D3DXVECTOR3& pos)	{ paintData.Position = pos; }

	void Update(VertexPTNTBC* vertices, uint size);
	
	const BrushData GetBrushData();
	ID3D11ShaderResourceView* GetBrushSRV();

//TextureBrush
	void SetChannelTexture(uint Channel, Texture* diffuse, Texture* normal, Texture* specular);
	void SetChannel(uint Channel)
	{
		paintData.ChannelNum = Channel;
	}

	void SetInputTexture();
	void CreateSplatTexture(const uint& width,const uint& height);
	void LoadSplatTexture(std::string& fileName);

	ID3D11ShaderResourceView* GetSplatChannel(uint channel, uint dns);
	ID3D11ShaderResourceView* GetSplatMap() { return splatSrv; }



	void SaveSplatMap();

private:
	class Context* context;
	class Graphics* graphics;
	bool active;


//BrushData
	PaintData paintData;

	class Texture* brushTex;

	ComputeBuffer* inputBuffer;
	ComputeBuffer* outputBuffer;
	ComputeShader* computeShader;
	ConstantBuffer* paintBuffer;

//Splat Brush

	ComputeBuffer* inputTexture;
	ComputeBuffer* outputTexture;
	ConstantBuffer* rowPitchBuffer;

	ID3D11Texture2D* splatTex;
	ID3D11ShaderResourceView* splatSrv;
	class Texture* splatChannel[4][3]; 
};