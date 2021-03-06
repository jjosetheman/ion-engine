#include "Actor.h"

#include <ion/core/string/String.h>

#define HEX1(val) std::hex << std::setfill('0') << std::setw(1) << std::uppercase << (int)val
#define HEX2(val) std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)val
#define HEX4(val) std::hex << std::setfill('0') << std::setw(4) << std::uppercase << (int)val

Actor::Actor()
{

}

const std::string& Actor::GetName() const
{
	return m_name;
}

void Actor::SetName(const std::string& name)
{
	m_name = name;
}

SpriteSheetId Actor::CreateSpriteSheet()
{
	SpriteSheetId spriteSheetId = ion::GenerateUUID64();
	m_spriteSheets.insert(std::make_pair(spriteSheetId, SpriteSheet()));
	return spriteSheetId;
}

void Actor::DeleteSpriteSheet(SpriteSheetId spriteSheetId)
{
	TSpriteSheetMap::iterator it = m_spriteSheets.find(spriteSheetId);
	if(it != m_spriteSheets.end())
	{
		m_spriteSheets.erase(it);
	}
}

SpriteSheet* Actor::GetSpriteSheet(SpriteSheetId spriteSheetId)
{
	SpriteSheet* spriteSheet = NULL;

	TSpriteSheetMap::iterator it = m_spriteSheets.find(spriteSheetId);
	if(it != m_spriteSheets.end())
	{
		spriteSheet = &it->second;
	}

	return spriteSheet;
}

const SpriteSheet* Actor::GetSpriteSheet(SpriteSheetId spriteSheetId) const
{
	const SpriteSheet* spriteSheet = NULL;

	TSpriteSheetMap::const_iterator it = m_spriteSheets.find(spriteSheetId);
	if(it != m_spriteSheets.end())
	{
		spriteSheet = &it->second;
	}

	return spriteSheet;
}

SpriteSheet* Actor::FindSpriteSheet(const std::string& name)
{
	for(TSpriteSheetMap::iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
        if(ion::string::CompareNoCase(it->second.GetName(), name))
		{
			return &it->second;
		}
	}

	return NULL;
}

const TSpriteSheetMap::const_iterator Actor::SpriteSheetsBegin() const
{
	return m_spriteSheets.begin();
}

const TSpriteSheetMap::const_iterator Actor::SpriteSheetsEnd() const
{
	return m_spriteSheets.end();
}

TSpriteSheetMap::iterator Actor::SpriteSheetsBegin()
{
	return m_spriteSheets.begin();
}

TSpriteSheetMap::iterator Actor::SpriteSheetsEnd()
{
	return m_spriteSheets.end();
}

int Actor::GetSpriteSheetCount() const
{
	return m_spriteSheets.size();
}

void Actor::SetMasterPalette(SpriteSheetId spriteSheetId)
{
	m_masterPaletteHolder = spriteSheetId;
}

const Palette* Actor::GetMasterPalette() const
{
	const Palette* palette = nullptr;

	if (m_masterPaletteHolder)
	{
		if (const SpriteSheet* sheet = GetSpriteSheet(m_masterPaletteHolder))
		{
			palette = &sheet->GetPalette();
		}
	}

	if(!palette && !m_spriteSheets.empty())
	{
		palette = &m_spriteSheets.begin()->second.GetPalette();
	}

	return palette;
}

void Actor::Serialise(ion::io::Archive& archive)
{
	archive.Serialise(m_name, "name");
	archive.Serialise(m_masterPaletteHolder, "masterPaletteHolder");
	archive.Serialise(m_spriteSheets, "spriteSheets");
}

void Actor::ExportSpriteSheets(const PlatformConfig& config, std::stringstream& stream)
{
	u32 tileIndex = 0;
	u32 frameIndex = 0;

	stream << "actor_" << m_name << ":" << std::endl << std::endl;

	//Calc largest frame size
	int largestWidthTiles = 0;
	int largestHeightTiles = 0;

	//Crop all sprites
	for (TSpriteSheetMap::iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		it->second.CropAllFrames(config.tileWidth, config.tileHeight);
	}

	for(TSpriteSheetMap::iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		u32 vramWidthTiles = it->second.GetMaxWidthTiles(config.tileWidth);
		u32 vramHeightTiles = it->second.GetMaxHeightTiles(config.tileHeight);

		if(vramWidthTiles > largestWidthTiles)
			largestWidthTiles = vramWidthTiles;

		if(vramHeightTiles > largestHeightTiles)
			largestHeightTiles = vramHeightTiles;
	}

	stream << "actor_" << m_name << "_VRAM_size_b\t\tequ 0x" << HEX2(largestWidthTiles * largestHeightTiles * 32) << "\t; VRAM size to alloc (size of largest frame, bytes)" << std::endl;
	stream << std::endl;

	//Export sprite sheet structures
	for (TSpriteSheetMap::iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		//	SpriteSheet_TileDataAddr			rs.l 1; Base address of tile data
		//	SpriteSheet_DimensionsArrAddr		rs.l 1; Address of VDP dimentions per subsprite array
		//	SpriteSheet_SubspriteOffsetArrAddr	rs.l 1; Address of position offsets per subsprite array
		//	SpriteSheet_PaletteAddr				rs.l 1; Address of palette
		//	SpriteSheet_WidthHeightTiles		rs.w 1; Sprite object width / height in tiles(0xWWHH)
		//	SpriteSheet_SizeBytes				rs.w 1; VRAM size(bytes)
		//	SpriteSheet_SizeTiles				rs.w 1; Sprite object single frame size in tiles
		//	SpriteSheet_SizeSubsprites			rs.b 1; Sprite object single frame size in subsprites
		//	SpriteSheet_PaletteId				rs.b 1; Palette ID
		//	SpriteSheet_Priority				rs.b 1; Sprite draw priority(0 / 1)

		std::stringstream label;
		label << "actor_" << m_name << "_sheet_" << it->second.GetName();

		stream << ";==============================================================" << std::endl;
		stream << "; Actor: " << m_name << " Sprite: " << it->second.GetName() << std::endl;
		stream << ";==============================================================" << std::endl;

		stream << "actor_" << m_name << "_sprite_" << it->second.GetName() << ":" << std::endl;
		stream << "\tdc.l spritesheet_" << m_name << "_" << it->second.GetName() << std::endl;
		stream << "\tdc.l actor_" << m_name << "_sheet_" << it->second.GetName() << "_frame_0_subsprite_dimensions_bits" << std::endl;
		stream << "\tdc.l actor_" << m_name << "_sheet_" << it->second.GetName() << "_frame_0_subsprite_pos_offsets" << std::endl;
		stream << "\tdc.l palette_" << m_name << std::endl;
		stream << "\tdc.w (actor_" << m_name << "_sheet_" << it->second.GetName() << "_width<<8)|"
			<< "actor_" << m_name << "_sheet_" << it->second.GetName() << "_height" << std::endl;
		stream << "\tdc.w actor_" << m_name << "_VRAM_size_b" << std::endl;
		stream << "\tdc.w actor_" << m_name << "_sheet_" << it->second.GetName() << "_frame_0_size_t" << std::endl;
		stream << "\tdc.w actor_" << m_name << "_sheet_" << it->second.GetName() << "_frame_0_size_subsprites" << std::endl;
		stream << "\tdc.b 0x0" << std::endl;
		stream << "\tdc.b 0x0" << std::endl;
		stream << ";==============================================================" << std::endl;

		stream << std::endl;
	}

	//Export sprite sheet size headers
	for(TSpriteSheetMap::iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		u32 widthTiles = it->second.GetWidthTiles();
		u32 heightTiles = it->second.GetHeightTiles();

		std::stringstream label;
		label << "actor_" << m_name << "_sheet_" << it->second.GetName();

		stream << label.str() << "_width\t\tequ 0x" << HEX2(widthTiles * config.tileWidth) << "\t; Max width of largest frame in pixels" << std::endl;
		stream << label.str() << "_height\t\tequ 0x" << HEX2(heightTiles * config.tileHeight) << "\t; Max height of largest frame in pixels" << std::endl;
		stream << std::dec;

		stream << std::endl << std::endl;

		tileIndex += it->second.GetNumCroppedTiles(config.tileWidth, config.tileHeight);
		frameIndex += it->second.GetNumFrames();
	}

	//Export frame size/offsets tables
	for(TSpriteSheetMap::const_iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		it->second.ExportSpriteFrameOffsetsTable(config, stream, m_name);
	}

	//Export sprite frame dimension tables
	for(TSpriteSheetMap::const_iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		it->second.ExportSpriteFrameDimensions(config, stream, m_name);
	}

	stream << "spritesheets_" << m_name << ":" << std::endl << std::endl;

	//Export sprite sheet tile data
	for(TSpriteSheetMap::const_iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		std::stringstream label;
		label << "spritesheet_" << m_name << "_" << it->second.GetName();

		stream << label.str() << ":" << std::endl << std::endl;

		it->second.ExportSpriteTiles(config, stream, m_name);

		stream << std::endl << std::endl;
	}
}

void Actor::ExportSpriteSheets(const PlatformConfig& config, ion::io::File& file) const
{

}

void Actor::ExportSpriteAnims(const PlatformConfig& config, std::stringstream& stream) const
{
	stream << "spriteanims_" << m_name << ":" << std::endl << std::endl;

	//Export sprite anim data
	for(TSpriteSheetMap::const_iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		it->second.ExportAnims(config, stream, m_name);
		stream << std::endl;
	}
}

void Actor::ExportSpriteAnims(const PlatformConfig& config, ion::io::File& file) const
{

}

void Actor::ExportSpritePalettes(const PlatformConfig& config, std::stringstream& stream) const
{
	if(const SpriteSheet* spriteSheet = GetSpriteSheet(m_masterPaletteHolder))
	{
		//Export palette from palette master
		stream << "palette_" << m_name << ":" << std::endl;
		spriteSheet->ExportPalette(config, stream);
		stream << std::endl;
	}
	else if(m_spriteSheets.size() > 0)
	{
		//Use first palette
		stream << "palette_" << m_name << ":" << std::endl;
		m_spriteSheets.begin()->second.ExportPalette(config, stream);
		stream << std::endl;
	}
}
