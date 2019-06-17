#pragma once

class Grass
{
public:
	Grass(Context* context);
	~Grass();

	void Update();
	void Render(CameraData cameraData);
	void DepthRender(CameraData cameraData);

	void SetLight(Light* light) { this->sunLight = light; }
	void SetRenderTexture(RenderTexture* renderTexture) { this->renderTexture = renderTexture; }

	struct GrassTest
	{
		D3DXVECTOR3 pos;
		D3DXCOLOR color;
	};

private:
	Context* context;
	Graphics* graphics;

	RenderTexture* renderTexture;
	Light* sunLight;

	Pipeline* pipeline;
	Pipeline* depth_pipeline;
	Geometry<VertexTexture> geometry;
	VertexBuffer* vertexBuffer;

	std::vector<InstanceGrass> instances;
	std::vector<GrassTest> instanceDatas;
	InstanceBuffer* instanceBuffer;
	IndexBuffer* indexBuffer;

	ID3D11ShaderResourceView* grassTex;
	

	float m_windRotation;
	int m_windDirection;
};