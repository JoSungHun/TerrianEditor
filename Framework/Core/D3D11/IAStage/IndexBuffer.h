#pragma once
#include "../IStage.h"

class IndexBuffer final : public IStage
{
public:
	IndexBuffer(class Context* context);
	~IndexBuffer();

	IndexBuffer(const IndexBuffer& rhs) = delete;
	IndexBuffer& operator=(const IndexBuffer& rhs) = delete;

	auto GetBuffer() const -> ID3D11Buffer* { return buffer; }
	auto GetIndexCount() const -> const uint& { return indexCount; }
	auto GetOffset() const -> const uint& { return offset; }
	void Create
	(
		const std::vector<uint>& indices,
		const D3D11_USAGE& usage = D3D11_USAGE_DEFAULT
	);

	void Clear();
	void BindPipeline();

private:
	class Graphics* graphics;

	ID3D11Buffer* buffer;
	uint offset;
	uint indexCount;
};