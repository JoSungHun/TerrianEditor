#pragma once


class TerrainPicker
{

public:
	TerrainPicker(class Context* context);
	~TerrainPicker();

private:

	struct PickOutputData
	{
		D3DXVECTOR3 pos;
		float distance;
		int picked;
	};

	
public:
	bool Picking(const D3DXMATRIX& world, D3DXVECTOR3& position,const D3DXVECTOR2& trSize);

	template<typename T>
	void CreateInput(const std::vector<T>& datas);

	template<typename T>
	void UpdateInput(const std::vector<T>& datas);

	void SetOutput();


	void DebugRender();
private:
	bool GetResult(D3DXVECTOR3& position);

	

private:
	class Context* context;
	class Graphics* graphics;
	class Camera* camera;
	uint num;
	 
	PickOutputData pickedData;

	ComputeShader* computeShader;
	ComputeBuffer* inputBuffer;
	ComputeBuffer* outputBuffer;
	ConstantBuffer* rayBuffer;
	ConstantBuffer* trInfoBuffer;
};



template <typename T>
inline void TerrainPicker::UpdateInput(const std::vector<T>& datas)
{
	inputBuffer->UpdateBuffer(datas.data(), datas.size());
	SetOutput();
}

template <typename T>
inline void TerrainPicker::CreateInput(const std::vector<T>& datas)
{
	inputBuffer->CreateBuffer(datas.data(), datas.size());
	SetOutput();
}