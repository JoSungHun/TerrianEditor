#pragma once

#define MAX_TRIANGLES 1000
#define VERTEX_INTERVAL 1

class TerrainCell
{
public:
	TerrainCell(Context* context);
	~TerrainCell();

	void CreateTerrainCell(const D3DXVECTOR3& centerPositioin,const float& size,const D3DXVECTOR2& trSize,Geometry<VertexPTNTBC>& geometry);
	
	void Render(const CameraData& cameraData,Pipeline* pipeline, bool frustum);
	void DepthRender(const CameraData& cameraData, Pipeline* pipeline);
	void RefractionRender(const CameraData& cameraData, Pipeline* pipeline);

private: 

	int CountTriangles(float centerX, float centerZ, const float& size, Geometry<VertexPTNTBC>& geometry);
	bool IsTriangleContained(Geometry<VertexPTNTBC>& geometry, int index, float centerX, float centerZ, const float& size);


private:
	class Context* context;
	class Graphics* graphics;
	class Frustum* frustum;

	//QuadTree Data
	uint numTriangles; 
	D3DXVECTOR3 centerPos;
	float minHeight, maxHeight;
	float width;

	TerrainCell* childCell[4];

	//자체적으로 가지고 있어야 할 데이터
	std::vector<uint> indices;
	IndexBuffer* indexBuffer;
	
	//DomainShader HullShader ConstantBuffer
	ConstantBuffer* tessBuffer;


};