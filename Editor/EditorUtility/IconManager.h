#pragma once

enum class IconType : uint
{
	TerrainBrush,
	TerrainBrush_Height,
	TerrainBrush_Texture_D,
	TerrainBrush_Texture_N,
	TerrainBrush_Texture_S
};


struct Icon
{
	Texture* texture;
	std::string name;
	std::string filePath; 
	IconType type;
};

class IconManager
{
public:
	static bool ImageButton(const Icon& icon, const float& width, const float& height);
	static void Image(const Icon& icon, const float& width, const float& height);


public:
	static IconManager& Get()
	{
		static IconManager instance;
		return instance;
	}

	void Initialize(class Context* context);

	std::vector<ID3D11ShaderResourceView*> GetSRVfromType(const IconType& type);
	ID3D11ShaderResourceView* GetSRVfromName(const std::string& name);

	const std::vector<Icon> GetIconFromType(const IconType& type);

	const bool ImageButton(const std::string& filePath, const float & width, const float& height);


private:
	IconManager();
	~IconManager();

	class Context* context;

	std::vector<Icon> icons;

};