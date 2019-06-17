#pragma once

class ImageImporter final
{
public:
    ImageImporter(class Context* context);
    ~ImageImporter();

    ImageImporter(const ImageImporter&) = delete;
    ImageImporter& operator=(const ImageImporter&) = delete;

    auto Load(const std::string& path, Texture* texture) -> const bool;

private:
    auto GetBitsFromFIBITMAP
    (
        std::vector<std::byte>* data,
        struct FIBITMAP* bitmap,
        const uint& width,
        const uint& height,
        const uint& channels
    ) -> const bool;

    void GenerateMipmaps
    (
        struct FIBITMAP* bitmap,
        Texture* texture,
        uint width,
        uint height,
        uint channels
    );

    auto ComputeChannelCount(struct FIBITMAP* bitmap) -> const uint;
    auto ComputeBitsPerChannel(struct FIBITMAP* bitmap) -> const uint;
    auto ComputeTextureFormat(const uint& bpp, const uint& channels) -> const DXGI_FORMAT;
    auto IsVisuallyGrayscale(struct FIBITMAP* bitmap) -> const bool;
    auto ApplyBitmapCorrections(struct FIBITMAP* bitmap) -> struct FIBITMAP*;
    auto ConvertTo32Bits(struct FIBITMAP* bitmap) -> struct FIBITMAP*;
    auto Rescale(struct FIBITMAP* bitmap, const uint& width, const uint& height) -> struct FIBITMAP*;
    auto SwapRedBlue32_Wrapper(struct FIBITMAP* bitmap) -> const bool;

private:
    class Context* context;
};