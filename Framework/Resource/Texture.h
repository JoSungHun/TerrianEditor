#pragma once
#include "IResource.h"

class Texture final : public IResource
{
public:
	static const std::string SaveTextureToFile(const std::string& path, const std::string& format, const uint& width, const uint& height, void* data);

public:
	typedef std::vector<std::byte> MipLevel;

public:
	Texture(class Context* context);
	~Texture();


	//====================================================================================
	//	Initialize
	//====================================================================================

	void Clear();
	void ClearTextureBytes();


	const bool Load(const std::string& path);
	const bool Create(const uint& width, const uint& height, const uint& channels, const DXGI_FORMAT& format, const MipLevel& mipLevel, const bool& bGenerateMipChain = false);
	const bool Create(const uint& width, const uint& height, const uint& channel, const DXGI_FORMAT& format, const std::vector<MipLevel>& mipChain);
	const bool Create(const uint& width, const uint& height, const DXGI_FORMAT& format);
	
	const bool UpdateData(const uint& width, const uint& height, const uint& stride);

	//====================================================================================
	//	GETTER
	//====================================================================================

	ID3D11ShaderResourceView* GetShaderResourceView()	const { return srv; }
	const DXGI_FORMAT& GetFormat()	const { return format; }
	const uint& GetBPP()			const { return bpp; }
	const uint& GetBPC()			const { return bpc; }
	const uint& GetWidth()			const { return width; }
	const uint& GetHeight()			const { return height; }
	const uint& GetChannels()		const { return channels; }
	const bool& IsGrayScale()		const { return bGrayscale; }
	const bool& IsTransparent()		const { return bTransparent; }
	const bool& IsNeedsMipChain()	const { return bNeedsMipChain; }
	
	void GetTextureBytes(std::vector<MipLevel>* textureBytes);
	auto GetMipLevelData(const uint& index) -> MipLevel*;

	const std::vector<MipLevel>& GetMipChain() { return mipChain; }
	const bool HasMipChain() { return !mipChain.empty(); }


	//====================================================================================
	//	SETTER
	//====================================================================================

	void SetFormat(const DXGI_FORMAT& format)				{ this->format = format; }
	void SetBPP(const uint& bpp)							{ this->bpp = bpp; }
	void SetBPC(const uint& bpc)							{ this->bpc = bpc; }
	void SetWidth(const uint& width)						{ this->width = width; }
	void SetHeight(const uint& height)						{ this->height = height; }
	void SetChannels(const uint& channel)					{ this->channels = channel; }
	void SetIsGrayscale(const bool& isGrayScale)			{ this->bGrayscale = isGrayScale; }
	void SetIsTransparency(const bool& isTransparent)		{ this->bTransparent = isTransparent;  }
	void SetIsNeedsMipChain(const bool& isNeedsMipChain)	{ this->bNeedsMipChain = isNeedsMipChain; }
	
	void SetTextureBytes(const std::vector<MipLevel>& textureBytes) { this->mipChain = textureBytes; }
	std::vector<std::byte>* AddMipLevelData() { return &mipChain.emplace_back(MipLevel()); }
	
	//====================================================================================
	//	SAVE & LOAD
	//====================================================================================
	// IResource을(를) 통해 상속됨
	virtual void LoadFromFile(const std::string & path) override;
	virtual void SaveToFile(const std::string & path) override;

private:
	auto Serialize(const std::string& path) -> const bool;
	auto Deserialize(const std::string& path) -> const bool;

private:
	class Graphics* graphics;

	uint bpp;
	uint bpc;
	uint width;
	uint height;
	uint channels;
	bool bGrayscale;
	bool bTransparent;
	bool bNeedsMipChain;
	DXGI_FORMAT format;
	std::vector<MipLevel> mipChain;

	ID3D11ShaderResourceView* srv;
	ID3D11Texture2D* texture;
};