#pragma once
class IMaterial;

enum class OverrideType {
	Normal = 0,
	BuildShadows,
	DepthWrite,
	CustomMaterial, // weapon skins
	SsaoDepthWrite
};

class IStudioRender
{
private:
	std::byte pad_0[592];
	IMaterial* materialOverride;
	std::byte pad_1[12];
	OverrideType overrideType;
public:
	bool IsForcedMaterialOverride() {
		if (!materialOverride)
			return overrideType == OverrideType::DepthWrite || overrideType == OverrideType::SsaoDepthWrite;
		return std::string_view{ materialOverride->GetName() }.starts_with(str("dev/glow"));
	}
};