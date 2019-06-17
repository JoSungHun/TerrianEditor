#include "Framework.h"
#include "Texture.h"
#include "./Importer/ImageImporter.h"

const std::string Texture::SaveTextureToFile(const std::string & path, const std::string & format, const uint & width, const uint & height, void * data)
{
	const auto texture_path = FileSystem::GetPathWithoutExtension(path) + "." + "format";

	auto w = std::make_unique<FileStream>(texture_path, StreamMode::Write);
	if (!w->IsOpen())
		return "N/A";

	if (height == 0)
		w->Write(data, width);
	else
		w->Write(data, width*height);

	return texture_path;
}

//====================================================================================
//	Initialize
//====================================================================================
Texture::Texture(Context * context)
	: IResource(context)
	, srv(nullptr)
	, format(DXGI_FORMAT_R8G8B8A8_UNORM)
	, texture(nullptr)
	, bpp(0)
	, bpc(0)
	, width(0)
	, height(0)
	, channels(0)
	, bGrayscale(false)
	, bTransparent(false)
	, bNeedsMipChain(true)
{
	graphics = context->GetSubsystem<Graphics>();
}

const bool Texture::Load(const std::string & path)
{
	auto importer = resourceManager->GetImageImporter();
	if (!importer->Load(path, this))
	{
		LOG_ERROR("Texture::Load : Failed loading texture");
		return false;
	}

	SetResourcePath(FileSystem::GetPathWithoutExtension(path) + ".texture");
	SetResourceName(FileSystem::GetIntactFileNameFromPath(GetResourcePath()));

	return true;
}

const bool Texture::Create(const uint & width, const uint & height, const uint & channels, const DXGI_FORMAT & format, const MipLevel & mipLevel, const bool & bGenerateMipChain)
{  
	if (width == 0 || height == 0 || mipChain.empty())
	{
		LOG_ERROR("Texture::Create : Invalied parameter");
		return false;
	}

	if (bGenerateMipChain)
	{
		if (width < 4 || height < 4)
		{
			Log::ErrorF("Texture::Create : MipChain won't be genderated as dimension %dx%d is too small", width, height);
			return false;
		}
	}

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = 0;
	desc.CPUAccessFlags = 0;

	if (bGenerateMipChain)
	{
		desc.MipLevels = 7;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = mipLevel.data();
	subResourceData.SysMemPitch = width * channels * (bpc / 8);
	subResourceData.SysMemSlicePitch = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;

	ID3D11Texture2D* texture = nullptr;
	auto result = SUCCEEDED(graphics->GetDevice()->CreateTexture2D
	(
		&desc,
		bGenerateMipChain ? nullptr : &subResourceData,
		&texture
	));

	if (!result)
	{
		LOG_ERROR("Texture::Create : Failed to create ID3D11Texture2D");
		return false;
	}

	result = SUCCEEDED(graphics->GetDevice()->CreateShaderResourceView
	(
		texture,
		&srvDesc,
		&srv
	));

	if (result)
	{
		if (bGenerateMipChain)
		{
			graphics->GetDeviceContext()->UpdateSubresource
			(
				texture,
				0,
				nullptr,
				mipLevel.data(),
				width * channels * (bpc / 8),
				0
			);

			graphics->GetDeviceContext()->GenerateMips(srv);
		}
	}
	else
		LOG_ERROR("Texture::Create : Failed to create ID3D11ShaderResourceView");

	SAFE_RELEASE(texture);
	return result;
}

const bool Texture::Create(const uint & width, const uint & height, const uint & channel, const DXGI_FORMAT & format, const std::vector<MipLevel>& mipChain)
{
	if (width == 0 || height == 0 || mipChain.empty())
	{
		LOG_ERROR("Texture::Create : Invalid parameter");
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mipChain.size();
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = 0;
	desc.CPUAccessFlags = 0;

	std::vector<D3D11_SUBRESOURCE_DATA> subResourceDatas;
	uint mipWidth = width;
	uint mipHeight = height;

	for (uint i = 0; i < mipChain.size(); i++)
	{
		if (mipChain[i].empty())
		{
			Log::ErrorF("Texture::Create : Mip Levels %d has invalid data", i);
			continue;
		}

		uint rowBytes = mipWidth * channels * (bpc / 8);

		D3D11_SUBRESOURCE_DATA& subData = subResourceDatas.emplace_back(D3D11_SUBRESOURCE_DATA{});
		subData.pSysMem = mipChain[i].data();
		subData.SysMemPitch = rowBytes;
		subData.SysMemSlicePitch = 0;

		mipWidth = std::max<uint>(mipWidth / 2, 1);
		mipHeight = std::max<uint>(mipHeight / 2, 1);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;

	auto result = SUCCEEDED(graphics->GetDevice()->CreateTexture2D
	(
		&desc,
		subResourceDatas.data(),
		&texture
	));

	if (!result)
	{
		LOG_ERROR("Texture::Create : Failed to create ID3D11Texture2D");
		return false;
	}

	result = SUCCEEDED(graphics->GetDevice()->CreateShaderResourceView
	(
		texture,
		&srvDesc,
		&srv
	));

	if (!result)
		LOG_ERROR("Texture::Create : Failed to create ID3D11ShaderResourceView");

	return result;
	
}

const bool Texture::Create(const uint & width, const uint & height, const DXGI_FORMAT & format)
{
	if (width == 0 || height == 0)
	{
		LOG_ERROR("Texture::Create : Invalid parameter");
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	/*uint mipWidth = width;
	uint mipHeight = height;
	uint factor = 0;
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		factor = 16;
		break;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
		factor = 8;
		break;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		factor = 4;
		break;
	}
	if (factor == 0)
	{
		LOG_ERROR("Texture:Create : Format is invalid");
		return false;
	}

	uint rowBytes = mipWidth*factor;*/

	/*
	std::vector<D3D11_SUBRESOURCE_DATA> subResourceDatas;
	uint mipWidth = width;
	uint mipHeight = height;

	for (uint i = 0; i < mipChain.size(); i++)
	{
		if (mipChain[i].empty())
		{
			Log::ErrorF("Texture::Create : Mip Levels %d has invalid data", i);
			continue;
		}

		uint rowBytes = mipWidth * channels * (bpc / 8);

		D3D11_SUBRESOURCE_DATA& subData = subResourceDatas.emplace_back(D3D11_SUBRESOURCE_DATA{});
		subData.pSysMem = mipChain[i].data();
		subData.SysMemPitch = rowBytes;
		subData.SysMemSlicePitch = 0;

		mipWidth = std::max<uint>(mipWidth / 2, 1);
		mipHeight = std::max<uint>(mipHeight / 2, 1);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;

	auto result = SUCCEEDED(graphics->GetDevice()->CreateTexture2D
	(
		&desc,
		subResourceDatas.data(),
		&texture
	));
	*/
	mipChain.push_back((MipLevel(width*height*sizeof(D3DXCOLOR),std::byte(0))));

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = mipChain[0].data();
	subResourceData.SysMemPitch = sizeof(D3DXCOLOR)*width;
	subResourceData.SysMemSlicePitch = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;


	auto result = SUCCEEDED(graphics->GetDevice()->CreateTexture2D
	(
		&desc,
		&subResourceData,
		&texture
	));

	if (!result)
	{
		LOG_ERROR("Texture::Create : Failed to create ID3D11Texture2D");
		return false;
	}

	result = SUCCEEDED(graphics->GetDevice()->CreateShaderResourceView
	(
		texture,
		&srvDesc,
		&srv
	));

	if (!result)
		LOG_ERROR("Texture::Create : Failed to create ID3D11ShaderResourceView");

	return result;
}

const bool Texture::UpdateData(const uint & width, const uint & height, const uint & stride)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto result = SUCCEEDED(
		graphics->GetDeviceContext()->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)
	);
	if (!result)
	{
		LOG_ERROR("! Error Map splatTexture");
		return false;
	}

	auto mappedData = reinterpret_cast<std::byte*>(mappedResource.pData);
	auto buffer = mipChain[0].data();
	for (uint i = 0; i < height; ++i)
	{
		memcpy(mappedData, buffer, stride*width);
		mappedData += mappedResource.RowPitch;
		buffer += stride * width;
	}

	graphics->GetDeviceContext()->Unmap(texture, 0);

	if (srv)
		SAFE_RELEASE(srv);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = SUCCEEDED(
		graphics->GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv)
	);
	if (!result)
	{
		LOG_ERROR("! Error Create ShaderResourceView");
		return false;
	}
	
	return true;
}


//====================================================================================
//	Clear
//====================================================================================

Texture::~Texture()
{
	Clear();
}

void Texture::Clear()
{
	SAFE_RELEASE(srv);
	format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	bpp					= 0;
	bpc					= 0;
	width				= 0;
	height				= 0;
	channels			= 0;
	bGrayscale			= false;
	bTransparent		= false;
	bNeedsMipChain		= false;
}

void Texture::ClearTextureBytes()
{
	for (auto& mip : mipChain)
	{
		mip.clear();
		mip.shrink_to_fit();
	}

	mipChain.clear();
	mipChain.shrink_to_fit();
}



//====================================================================================
//	Getter
//====================================================================================


void Texture::GetTextureBytes(std::vector<MipLevel>* textureBytes)
{
	if (!mipChain.empty())
	{
		textureBytes = &mipChain;
		return;
	}

	auto r = std::make_unique<FileStream>(resourcePath, StreamMode::Read);
	if (!r->IsOpen())
		return;

	uint mipCount = r->Read<uint>();
	for (uint i = 0; i < mipCount; i++)
	{
		textureBytes->emplace_back(MipLevel());
		r->Read(mipChain[i]);
	}

}

auto Texture::GetMipLevelData(const uint & index) -> MipLevel *
{
	if (index >= mipChain.size())
	{
		LOG_ERROR("Texture::GetMipLevelData : Index out of range");
		return nullptr;
	}

	return &mipChain[index];
}


//====================================================================================
//	SAVE & LOAD
//====================================================================================

void Texture::LoadFromFile(const std::string & path)
{
	auto filePath = FileSystem::GetRelativeFromPath(path);
	if (!FileSystem::ExistFile(filePath))
	{
		Log::ErrorF("Texture::LoadFromFile : Path \"%s\" is invalid", filePath.c_str());
		return;
	}

	ClearTextureBytes();

	bool bLoading = false;
	if (FileSystem::GetExtensionFromPath(filePath) == ".texture")
		bLoading = Deserialize(filePath);
	else if (FileSystem::IsSupportedImageFile(filePath))
		bLoading = Load(path);

	if (!bLoading)
	{
		Log::ErrorF("Texture::LoadFromFile : Failed to load \"%s\"", filePath.c_str());
		return;
	}

	if (width == 0 || height == 0 || channels == 0 || mipChain.empty() || mipChain.front().empty())
	{
		LOG_ERROR("Texture::LoadFromFile : Invalid parameter");
		return;
	}

	HasMipChain() ?
		Create(width, height, channels, format, mipChain) :
		Create(width, height, channels, format, mipChain.front(), bNeedsMipChain);

	if (FileSystem::GetExtensionFromPath(filePath) == ".texture")
		ClearTextureBytes();
}

void Texture::SaveToFile(const std::string & path)
{
}


//====================================================================================
//	Serialize
//====================================================================================
auto Texture::Serialize(const std::string & path) -> const bool
{
	GetTextureBytes(&mipChain);

	auto w = std::make_unique<FileStream>(path, StreamMode::Write);
	if (!w->IsOpen())
		return false;

	w->Write(mipChain.size());
	for (auto& mip : mipChain)
		w->Write(mip);

	w->Write(bpp);
	w->Write(bpc);
	w->Write(width);
	w->Write(height);
	w->Write(channels);
	w->Write(bGrayscale);
	w->Write(bTransparent);
	w->Write(bNeedsMipChain);
	w->Write(resourceName);
	w->Write(resourcePath);

	ClearTextureBytes();
	return true;
}

auto Texture::Deserialize(const std::string & path) -> const bool
{
	auto r = std::make_unique<FileStream>(path, StreamMode::Read);
	if (!r->IsOpen())
		return false;

	ClearTextureBytes();

	mipChain.resize(r->Read<uint>());
	for (auto& mip : mipChain)
		r->Read(mip);

	r->Read(bpp);
	r->Read(bpc);
	r->Read(width);
	r->Read(height);
	r->Read(channels);
	r->Read(bGrayscale);
	r->Read(bTransparent);
	r->Read(bNeedsMipChain);
	r->Read(resourceName);
	r->Read(resourcePath);

	return true;
}
