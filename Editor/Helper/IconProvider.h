#pragma once

enum class IconType : uint
{
	Custom,
	Thumbnail_Folder,
	Thumbnail_File,
	Button_Play_Big,
	Button_Play,
	Button_Pause,
	Button_Stop,
	Button_Option,
	Log_Info,
	Log_Warning,
	Log_Error,
	Terrain_Brush_Height,
	Terrain_Brush_Flat,
	Terrain_Brush_Paint,
};

struct Thumbnail
{
	Thumbnail()
		: type(IconType::Custom)
		, texture(nullptr)
		, path("")
	{}

	Thumbnail
	(
		const IconType& type,
		std::shared_ptr<Texture> texture,
		const std::string& path
	)
		: type(type)
		, texture(texture)
		, path(path)
	{}

	IconType type;
	std::shared_ptr<Texture> texture;
	std::string path;
};

class IconProvider final
{
public:
	static IconProvider& Get()
	{
		static IconProvider instance;
		return instance;
	}

	void Initialize(Context* context);

	auto GetShaderResourceFromType(const IconType& type)->ID3D11ShaderResourceView*;
	auto GetShaderResourceFromPath(const std::string& path)->ID3D11ShaderResourceView*;

	auto Load(const std::string& path, const IconType& type = IconType::Custom)->Thumbnail*;

	auto ImageButton(const Thumbnail& thumbnail, const ImVec2& size)->const bool;
	auto ImageButton(const IconType& type, const float& size)->const bool;
	auto ImageButton(const IconType& type, const float& width, const float& height) -> const bool;
	auto ImageButton(const std::string& path, const float& size)->const bool;

private:
	IconProvider() = default;
	~IconProvider() = default;

	IconProvider(const IconProvider&) = delete;
	IconProvider& operator=(const IconProvider&) = delete;

	auto GetThumbnailFromType(const IconType& type)->Thumbnail*;

private:
	Context* context;
	std::vector<Thumbnail> thumbnails;
};