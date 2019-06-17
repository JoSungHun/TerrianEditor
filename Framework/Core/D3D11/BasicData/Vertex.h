#pragma once

struct VertexColor
{
	D3DXVECTOR3 Position;
	D3DXCOLOR Color;

	static D3D11_INPUT_ELEMENT_DESC Desc[];
	static const uint Count = 2;

	VertexColor()
		: Position(0, 0, 0)
		, Color(0, 0, 0, 1)
	{}

	VertexColor(const D3DXVECTOR3& position, const D3DXCOLOR& color)
		: Position(position)
		, Color(color)
	{}
};

struct VertexTexture
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;

	static D3D11_INPUT_ELEMENT_DESC Desc[];
	static const uint Count = 2;

	VertexTexture()
		: Position(0, 0, 0)
		, Uv(0, 0)
	{}

	VertexTexture(const D3DXVECTOR3& position, const D3DXVECTOR2& uv)
		: Position(position)
		, Uv(uv)
	{}
};

struct VertexTextureNormal
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXVECTOR3 Normal;

	static D3D11_INPUT_ELEMENT_DESC Desc[];
	static const uint Count = 3;

	VertexTextureNormal()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Normal(0, 0, 0)
	{}

	VertexTextureNormal
	(
		const D3DXVECTOR3& position,
		const D3DXVECTOR2& uv,
		const D3DXVECTOR3& normal
	)
		: Position(position)
		, Uv(uv)
		, Normal(normal)
	{}
};

struct VertexTextureNormalTangent
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXVECTOR3 Normal;
	D3DXVECTOR3 Tangent;

	static D3D11_INPUT_ELEMENT_DESC Desc[];
	static const uint Count = 4;

	VertexTextureNormalTangent()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Normal(0, 0, 0)
		, Tangent(0, 0, 0)
	{}

	VertexTextureNormalTangent
	(
		const D3DXVECTOR3& position,
		const D3DXVECTOR2& uv,
		const D3DXVECTOR3& normal,
		const D3DXVECTOR3& tangent
	)
		: Position(position)
		, Uv(uv)
		, Normal(normal)
		, Tangent(tangent)
	{}
};

struct VertexTextureNormalTangentBlend
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXVECTOR3 Normal;
	D3DXVECTOR3 Tangent;
	D3DXVECTOR4 Indices;
	D3DXVECTOR4 Weights;

	static D3D11_INPUT_ELEMENT_DESC Desc[];
	static const uint Count = 6;

	VertexTextureNormalTangentBlend()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Normal(0, 0, 0)
		, Tangent(0, 0, 0)
		, Indices(0, 0, 0, 0)
		, Weights(0, 0, 0, 0)
	{}

	VertexTextureNormalTangentBlend
	(
		const D3DXVECTOR3& position,
		const D3DXVECTOR2& uv,
		const D3DXVECTOR3& normal,
		const D3DXVECTOR3& tangent,
		const D3DXVECTOR4& indices,
		const D3DXVECTOR4& weights
	)
		: Position(position)
		, Uv(uv)
		, Normal(normal)
		, Tangent(tangent)
		, Indices(indices)
		, Weights(weights)
	{}
};


struct VertexTerrain
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXVECTOR3 Normal;
	D3DXVECTOR3 Tangent;
	D3DXVECTOR3 Binormal;
	D3DXCOLOR Color;
	D3DXCOLOR Alpha;

	static D3D11_INPUT_ELEMENT_DESC Desc[];
	static const uint Count = 5;

	VertexTerrain()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Normal(0, 0, 0)
		, Tangent(0, 0, 0)
		, Binormal(0, 0, 0)
		, Color(0, 0, 0, 0)
		, Alpha(0, 0, 0, 0)
	{}

	VertexTerrain
	(
		const D3DXVECTOR3& position,
		const D3DXVECTOR2& uv,
		const D3DXVECTOR3& normal,
		const D3DXVECTOR3& tangent,
		const D3DXVECTOR3& binormal,
		const D3DXCOLOR& color,
		const D3DXCOLOR& alpha
	)
		: Position(position)
		, Uv(uv)
		, Normal(normal)
		, Tangent(tangent)
		, Binormal(binormal)
		, Color(color)
		, Alpha(alpha)
	{}
};

typedef VertexTextureNormalTangentBlend VertexModel;