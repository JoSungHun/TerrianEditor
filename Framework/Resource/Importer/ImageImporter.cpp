#include "Framework.h"
#include "ImageImporter.h"
#include <FreeImage.h>
#include <Utilities.h>

/*
	capacity small  ------------------------> capacity big
	Blur            ------------------------> Sharp
	Fast            ------------------------> Slow
	==========================================================
	bilinear                bicubic              lanczos
*/

namespace Rescale
{
	FREE_IMAGE_FILTER rescaleFilter = FILTER_LANCZOS3;

	struct RescaleJob
	{
		uint width;
		uint height;
		uint channels;
		std::vector<std::byte>* data;
		bool done;

		RescaleJob(const uint& width, const uint& height, const uint& channels)
			: width(width)
			, height(height)
			, channels(channels)
			, data(nullptr)
			, done(false)
		{}
	};
}

ImageImporter::ImageImporter(Context * context)
	: context(context)
{
	FreeImage_Initialise();

	auto FreeImageErrorHandler = [](FREE_IMAGE_FORMAT fif, const char* message)
	{
		const char* text = (message != nullptr) ? message : "Unknown error";
		const char* format = (fif != FIF_UNKNOWN) ? FreeImage_GetFormatFromFIF(fif) : "Unknown";

		Log::ErrorF("%s, Format : %s", text, format);
	};

	FreeImage_SetOutputMessage(FreeImageErrorHandler);
}

ImageImporter::~ImageImporter()
{
	FreeImage_DeInitialise();
}

auto ImageImporter::Load(const std::string & path, Texture * texture) -> const bool
{
	if (!texture)
	{
		LOG_ERROR("ImageImporter::Load : Invalid parameter");
		return false;
	}

	if (!FileSystem::ExistFile(path))
	{
		Log::ErrorF("Path \"%s\" is invalid.", path.c_str());
		return false;
	}

	//�̹��� ���� ��������
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path.c_str(), 0);
	format = (format == FIF_UNKNOWN) ? FreeImage_GetFIFFromFilename(path.c_str()) : format;//������ �𸣸� ���� �̸����� ������

	//�𸣴� �����̸̹� ��
	if (!FreeImage_FIFSupportsReading(format))
	{
		Log::ErrorF("Unknown or unsupported format.");
		return false;
	}

	//�̹��� �ε�
	FIBITMAP* bitmap = FreeImage_Load(format, path.c_str());

	//�Ϻ� ����
	bitmap = ApplyBitmapCorrections(bitmap);
	if (!bitmap)
		return false;

	//��� ���� ����(�ʿ��)
	const bool userDefineDimensions = (texture->GetWidth() != 0 && texture->GetHeight() != 0);
	const bool dimensionMismatch = (FreeImage_GetWidth(bitmap) != texture->GetWidth() && FreeImage_GetHeight(bitmap) != texture->GetHeight());
	const bool scale = userDefineDimensions && dimensionMismatch;
	bitmap = scale ? Rescale(bitmap, texture->GetWidth(), texture->GetHeight()) : bitmap;

	//�̹��� �Ӽ� ����
	const bool image_transparency = FreeImage_IsTransparent(bitmap);
	const uint image_width = FreeImage_GetWidth(bitmap);
	const uint image_height = FreeImage_GetHeight(bitmap);
	const uint image_bpp = FreeImage_GetBPP(bitmap);
	const uint image_byesPerChannel = ComputeBitsPerChannel(bitmap);
	const uint image_channels = ComputeChannelCount(bitmap);
	const DXGI_FORMAT image_format = ComputeTextureFormat(image_bpp, image_channels);
	const bool image_grayscale = IsVisuallyGrayscale(bitmap);

	//FIBITMAP �����ͷ� RGBA ���͸� ü���
	const auto mip = texture->AddMipLevelData();
	GetBitsFromFIBITMAP(mip, bitmap, image_width, image_height, image_channels);

	//�ؽ��Ŀ� Mipmap�� �ʿ��Ѱ�� ����
	if (texture->IsNeedsMipChain())
		GenerateMipmaps(bitmap, texture, image_width, image_height, image_channels);

	//�޸� ����
	FreeImage_Unload(bitmap);

	//�̹����Ӽ��� Texture �� ����
	texture->SetBPP(image_bpp);
	texture->SetBPC(image_byesPerChannel);
	texture->SetWidth(image_width);
	texture->SetHeight(image_height);
	texture->SetChannels(image_channels);
	texture->SetIsTransparency(image_transparency);
	texture->SetFormat(image_format);
	texture->SetIsGrayscale(image_grayscale);

	return true;
}

auto ImageImporter::GetBitsFromFIBITMAP(std::vector<std::byte>* data, FIBITMAP * bitmap, const uint & width, const uint & height, const uint & channels) -> const bool
{
	if (!data || width == 0 || height == 0 || channels == 0)
	{
		LOG_ERROR("ImageImporter::GetBitsFromFIBITMAP : Invalid parameter");
		return false;
	}

	//���� ������ ũ�⸦ ����ϰ� ����� �޸� Ȯ��
	uint size = width * height * channels *  (ComputeBitsPerChannel(bitmap) / 8);
	if (size != data->size())
	{
		data->clear();
		data->reserve(size);
		data->resize(size);
	}

	// vector�� ������ ����
	auto bits = FreeImage_GetBits(bitmap);
	memcpy(&(*data)[0], bits, size);

	return true;
}

void ImageImporter::GenerateMipmaps(FIBITMAP * bitmap, Texture * texture, uint width, uint height, uint channels)
{
	if (!texture)
	{
		LOG_ERROR("ImageImporter::GenerateMipmaps : Invalid parameter");
		return;
	}

	//�ʿ��� ��� Mipmap�� ���� RescaleJob�� ����
	std::vector<Rescale::RescaleJob> jobs;
	while (width > 1 && height > 1)
	{
		width = std::max<uint>(width / 2, 1);
		height = std::max<uint>(height / 2, 1);
		jobs.emplace_back(width, height, channels);

		//�����Ϳ� ���� Texture�� ������ ũ�⸦ ����
		uint size = width * height * channels;
		std::vector<std::byte>* mip = texture->AddMipLevelData();
		mip->reserve(size);
		mip->resize(size);
	}

	//������ Mipmap�� ������ �����͸� ����
	for (uint i = 0; i < jobs.size(); i++)
		jobs[i].data = texture->GetMipLevelData(i + 1);//0�� Mipmap�� �⺻ �̹��� ũ���̱� ������ i + 1�� ����

	//FILTER_LANCZOS3�� ����ϴ� Rescale�Լ��� ��α⶧���� ���߽����带 ����ؼ� Mipmap ������ ����ȭ��
	auto threading = context->GetSubsystem<Thread>();
	for (auto& job : jobs)
	{
		threading->AddTask([this, &job, &bitmap]()
		{
			FIBITMAP* bitmapScaled = FreeImage_Rescale(bitmap, job.width, job.height, Rescale::rescaleFilter);
			if (!GetBitsFromFIBITMAP(job.data, bitmapScaled, job.width, job.height, job.channels))
				Log::ErrorF("Failed to create mip level %dx%d", job.width, job.height);

			FreeImage_Unload(bitmapScaled);
			job.done = true;
		});
	}

	//��� Mipmap�� �����ɶ����� ���
	bool ready = false;
	while (!ready)
	{
		ready = true;
		for (const auto& job : jobs)
		{
			if (!job.done)
				ready = false;
		}
	}
}

auto ImageImporter::ComputeChannelCount(FIBITMAP * bitmap) -> const uint
{
	if (!bitmap)
	{
		LOG_ERROR("ImageImporter::ComputeChannelCount : Invalid parameter");
		return 0;
	}

	//pixel �� ����Ʈ ���� ���
	uint bytespp = FreeImage_GetLine(bitmap) / FreeImage_GetWidth(bitmap);

	//pixel �� ���ü��� ���
	uint channels = bytespp / (ComputeBitsPerChannel(bitmap) / 8);

	return channels;
}

auto ImageImporter::ComputeBitsPerChannel(FIBITMAP * bitmap) -> const uint
{
	if (!bitmap)
	{
		LOG_ERROR("ImageImporter::ComputeBitsPerChannel : Invalid parameter");
		return 0;
	}

	FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);
	unsigned int size = 0;

	if (type == FIT_BITMAP)
		size = sizeof(BYTE);
	else if (type == FIT_UINT16 || type == FIT_RGB16 || type == FIT_RGBA16)
		size = sizeof(WORD);
	else if (type == FIT_FLOAT || type == FIT_RGBF || type == FIT_RGBAF)
		size = sizeof(float);

	return size * 8;
}

auto ImageImporter::ComputeTextureFormat(const uint & bpp, const uint & channels) -> const DXGI_FORMAT
{
	if (channels == 3)
	{
		if (bpp == 96) return DXGI_FORMAT_R32G32B32_FLOAT;
	}
	else if (channels == 4)
	{
		if (bpp == 32)	return DXGI_FORMAT_R8G8B8A8_UNORM;
		if (bpp == 64)	return DXGI_FORMAT_R16G16B16A16_FLOAT;
		if (bpp == 128) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}

	LOG_ERROR("ImageImporter::ComputeTextureFormat : Invalid parameter");
	return DXGI_FORMAT_R8_UNORM;
}

auto ImageImporter::IsVisuallyGrayscale(FIBITMAP * bitmap) -> const bool
{
	if (!bitmap)
	{
		LOG_ERROR("ImageImporter::IsVisuallyGrayscale : Invalid parameter");
		return false;
	}

	switch (FreeImage_GetBPP(bitmap))
	{
	case 1:
	case 4:
	case 8:
	{
		const auto ncolors = FreeImage_GetColorsUsed(bitmap);
		const auto rgb = FreeImage_GetPalette(bitmap);
		for (uint i = 0; i < ncolors; i++)
		{
			if ((rgb->rgbRed != rgb->rgbGreen) || (rgb->rgbRed != rgb->rgbBlue))
				return false;
		}
		return true;
	}
	default:
		return (FreeImage_GetColorType(bitmap) == FIC_MINISBLACK);
	}
}

auto ImageImporter::ApplyBitmapCorrections(FIBITMAP * bitmap) -> struct FIBITMAP *
{
	if (!bitmap)
	{
		LOG_ERROR("ImageImporter::ApplyBitmapCorrections : Invalid parameter");
		return nullptr;
	}

	unsigned int channels = ComputeChannelCount(bitmap);
	if (channels == 1)
	{
		int bpp = ComputeBitsPerChannel(bitmap);
		if (bpp == 16)
		{
			FIBITMAP* previousBitmap = bitmap;
			bitmap = FreeImage_ConvertTo8Bits(bitmap);
			FreeImage_Unload(previousBitmap);
		}
	}

	//32bit �� ��ȯ
	if (FreeImage_GetBPP(bitmap) < 32)
		bitmap = ConvertTo32Bits(bitmap);

	//���� ä�ΰ� �Ķ� ä���� �ٲ�(�ʿ��� ���)
	if (FreeImage_GetBPP(bitmap) == 32)
	{
		if (FreeImage_GetRedMask(bitmap) == 0xff0000 && ComputeChannelCount(bitmap) >= 2)
		{
			bool swapped = SwapRedBlue32_Wrapper(bitmap);
			if (!swapped)
				LOG_ERROR("ImageImporter::ApplyBitmapCorrections : Failed to swap red with blue channel");
		}
	}

	//�������� ������
	FreeImage_FlipVertical(bitmap);

	return bitmap;
}

auto ImageImporter::ConvertTo32Bits(FIBITMAP * bitmap) -> struct FIBITMAP *
{
	if (!bitmap)
	{
		LOG_ERROR("ImageImporter::ConvertTo32Bits : Invalid parameter");
		return nullptr;
	}

	FIBITMAP* previousBitmap = bitmap;
	bitmap = FreeImage_ConvertTo32Bits(previousBitmap);
	if (!bitmap)
	{
		Log::ErrorF("ImageImporter::ConvertTo32Bits : Failed (%d bpp, %d channels).", FreeImage_GetBPP(previousBitmap), ComputeChannelCount(previousBitmap));
		return nullptr;
	}

	FreeImage_Unload(previousBitmap);
	return bitmap;
}

auto ImageImporter::Rescale(FIBITMAP * bitmap, const uint & width, const uint & height) -> struct FIBITMAP *
{
	if (!bitmap || width == 0 || height == 0)
	{
		LOG_ERROR("ImageImporter::Rescale : Invalid parameter");
		return nullptr;
	}

	FIBITMAP* previousBitmap = bitmap;
	bitmap = FreeImage_Rescale(previousBitmap, width, height, Rescale::rescaleFilter);
	if (!bitmap)
	{
		LOG_ERROR("ImageImporter::Rescale : Failed");
		return previousBitmap;
	}

	FreeImage_Unload(previousBitmap);
	return bitmap;
}

auto ImageImporter::SwapRedBlue32_Wrapper(FIBITMAP * bitmap) -> const bool
{
	if (FreeImage_GetImageType(bitmap) != FIT_BITMAP) {
		return false;
	}

	const unsigned bytesperpixel = FreeImage_GetBPP(bitmap) / 8;
	if (bytesperpixel > 4 || bytesperpixel < 3) {
		return false;
	}

	const uint height = FreeImage_GetHeight(bitmap);
	const uint pitch = FreeImage_GetPitch(bitmap);
	const uint lineSize = FreeImage_GetLine(bitmap);

	BYTE* line = FreeImage_GetBits(bitmap);
	for (uint y = 0; y < height; ++y, line += pitch) {
		for (BYTE* pixel = line; pixel < line + lineSize; pixel += bytesperpixel) {
			INPLACESWAP(pixel[0], pixel[2]);
		}
	}

	return true;
}
