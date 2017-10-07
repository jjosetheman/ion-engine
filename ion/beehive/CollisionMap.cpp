///////////////////////////////////////////////////////
// Beehive: A complete SEGA Mega Drive content tool
//
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
///////////////////////////////////////////////////////

#include "CollisionMap.h"
#include "Project.h"
#include <algorithm>

#include <ion/core/memory/Memory.h>
#include <ion/core/memory/Endian.h>
#include <ion/maths/Geometry.h>

#pragma optimize( "", off )

CollisionMap::CollisionMap()
{
	const PlatformConfig& defaultPlatformConfig = PlatformPresets::s_configs[PlatformPresets::ePresetMegaDrive];
	m_width = 0;
	m_height = 0;
	Resize(defaultPlatformConfig.scrollPlaneWidthTiles, defaultPlatformConfig.scrollPlaneHeightTiles, false, false);
}

CollisionMap::CollisionMap(const PlatformConfig& platformConfig)
{
	m_width = 0;
	m_height = 0;
	Resize(platformConfig.scrollPlaneWidthTiles, platformConfig.scrollPlaneHeightTiles, false, false);
}

CollisionMap::CollisionMap(const PlatformConfig& platformConfig, int width, int height)
{
	m_width = 0;
	m_height = 0;
	Resize(width, height, false, false);
}

void CollisionMap::Clear()
{
	//Clear tiles
	for(int x = 0; x < m_width; x++)
	{
		for(int y = 0; y < m_height; y++)
		{
			SetTerrainTile(x, y, InvalidTerrainTileId);
		}
	}
}

void CollisionMap::Serialise(ion::io::Archive& archive)
{
	archive.Serialise(m_width, "width");
	archive.Serialise(m_height, "height");
	archive.Serialise(m_terrainBeziers, "beziers");
	archive.Serialise(m_collisionTiles, "collisionTiles");

	if(archive.GetDirection() == ion::io::Archive::eIn && m_terrainBeziers.size() == 0)
	{
		//Legacy, no bezier flags
		std::vector<ion::gamekit::BezierPath> beziers;
		archive.Serialise(beziers, "terrainBeziers");

		for(int i = 0; i < beziers.size(); i++)
		{
			m_terrainBeziers.push_back(TerrainBezier({ beziers[i], 0 }));
		}
	}
}

int CollisionMap::GetWidth() const
{
	return m_width;
}

int CollisionMap::GetHeight() const
{
	return m_height;
}

int CollisionMap::GetBlockAlignedWidth(int blockWidth) const
{
	return ion::maths::RoundUpToNearest(m_width, blockWidth);
}

int CollisionMap::GetBlockAlignedHeight(int blockHeight) const
{
	return ion::maths::RoundUpToNearest(m_height, blockHeight);
}

int CollisionMap::GetWidthBlocks(int blockWidth) const
{
	return GetBlockAlignedWidth(blockWidth) / blockWidth;
}

int CollisionMap::GetHeightBlocks(int blockHeight) const
{
	return GetBlockAlignedHeight(blockHeight) / blockHeight;
}

void CollisionMap::Resize(int width, int height, bool shiftRight, bool shiftDown)
{
	//Create new tile array
	std::vector<TerrainTileId> terrainTiles;

	//Set new size
	int size = width * height;
	terrainTiles.resize(size);

	//Fill with invalid tile
	std::fill(terrainTiles.begin(), terrainTiles.end(), InvalidTerrainTileId);

	//Copy tiles
	for(int x = 0; x < ion::maths::Min(width, m_width); x++)
	{
		for(int y = 0; y < ion::maths::Min(height, m_height); y++)
		{
			int destTileIdx = (y * width) + x;
			if(shiftRight && width > m_width)
				destTileIdx += (width - m_width);
			if(shiftDown && height > m_height)
				destTileIdx += (height - m_height) * width;

			terrainTiles[destTileIdx] = GetTerrainTile(x, y) | GetCollisionTileFlags(x, y);
		}
	}
	
	//Set new
	m_collisionTiles = terrainTiles;

	//Move splines
	for(int i = 0; i < m_terrainBeziers.size(); i++)
	{
		ion::Vector2 offset;

		if(shiftRight && width > m_width)
			offset.x += (float)(width - m_width) * 8.0f;
		if(!shiftDown && height > m_height)
			offset.y -= (float)(height - m_height) * 8.0f;

		m_terrainBeziers[i].bezier.Move(offset);
	}

	m_width = width;
	m_height = height;
}

ion::gamekit::BezierPath* CollisionMap::AddTerrainBezier()
{
	m_terrainBeziers.push_back(TerrainBezier({ ion::gamekit::BezierPath(), 0 }));
	return &m_terrainBeziers.back().bezier;
}

void CollisionMap::AddTerrainBezier(const ion::gamekit::BezierPath& bezier)
{
	m_terrainBeziers.push_back(TerrainBezier({ bezier, 0 }));
}

ion::gamekit::BezierPath* CollisionMap::GetTerrainBezier(u32 index)
{
	ion::debug::Assert(index < m_terrainBeziers.size(), "CollisionMap::GetTerrainBezier() - Out of range");
	return &m_terrainBeziers[index].bezier;
}

const ion::gamekit::BezierPath* CollisionMap::GetTerrainBezier(u32 index) const
{
	ion::debug::Assert(index < m_terrainBeziers.size(), "CollisionMap::GetTerrainBezier() - Out of range");
	return &m_terrainBeziers[index].bezier;
}

void CollisionMap::SetTerrainBezierFlags(u32 index, u16 flags)
{
	ion::debug::Assert(index < m_terrainBeziers.size(), "CollisionMap::SetTerrainBezierFlags() - Out of range");
	m_terrainBeziers[index].terrainFlags = flags;
}

u16 CollisionMap::GetTerrainBezierFlags(u32 index) const
{
	ion::debug::Assert(index < m_terrainBeziers.size(), "CollisionMap::GetTerrainBezierFlags() - Out of range");
	return m_terrainBeziers[index].terrainFlags;
}

const ion::gamekit::BezierPath* CollisionMap::FindTerrainBezier(int x, int y, ion::Vector2i& topLeft) const
{
	const ion::gamekit::BezierPath* bezier = NULL;
	ion::Vector2i bottomRight;

	ion::Vector2 bezierBoundsMin;
	ion::Vector2 bezierBoundsMax;

	for(int i = 0; i < m_terrainBeziers.size() && !bezier; i++)
	{
		m_terrainBeziers[i].bezier.GetBounds(bezierBoundsMin, bezierBoundsMax);

		if(x >= topLeft.x && y >= topLeft.y
			&& x < bottomRight.x && y < bottomRight.y)
		{
			bezier = &m_terrainBeziers[i].bezier;
			topLeft.x = bezierBoundsMin.x;
			topLeft.y = bezierBoundsMin.y;
		}
	}

	return bezier;
}

int CollisionMap::FindTerrainBeziers(int x, int y, int width, int height, std::vector<const ion::gamekit::BezierPath*>& beziers) const
{
	ion::Vector2i boundsMin(x * 8, y * 8);
	ion::Vector2i boundsMax((x + width) * 8, (y + height) * 8);

	ion::Vector2 bezierBoundsMin;
	ion::Vector2 bezierBoundsMax;

	for(int i = 0; i < m_terrainBeziers.size(); i++)
	{
		const ion::gamekit::BezierPath& bezier = m_terrainBeziers[i].bezier;
		bezier.GetBounds(bezierBoundsMin, bezierBoundsMax);

		//Invert Y
		float temp = ((m_height * 8) - bezierBoundsMin.y - 1);
		bezierBoundsMin.y = ((m_height * 8) - bezierBoundsMax.y - 1);
		bezierBoundsMax.y = temp;

		if(ion::maths::BoxIntersectsBox(boundsMin, boundsMax, ion::Vector2i(bezierBoundsMin.x, bezierBoundsMin.y), ion::Vector2i(bezierBoundsMax.x, bezierBoundsMax.y)))
		{
			beziers.push_back(&bezier);
		}
	}

	return beziers.size();
}

void CollisionMap::RemoveTerrainBezier(u32 index)
{
	ion::debug::Assert(index < m_terrainBeziers.size(), "CollisionMap::RemoveTerrainBezier() - Out of range");
	m_terrainBeziers.erase(m_terrainBeziers.begin() + index);
}

int CollisionMap::GetNumTerrainBeziers() const
{
	return m_terrainBeziers.size();
}

void CollisionMap::GenerateBlocks(const Project& project, int blockWidth, int blockHeight)
{
	//Align map size to block size
	int mapWidth = GetBlockAlignedWidth(blockWidth);
	int mapHeight = GetBlockAlignedHeight(blockHeight);

	//Use default tile if there is one, else use first tile
	u32 defaultTileId = project.GetDefaultTerrainTile();
	if(defaultTileId == InvalidTerrainTileId)
	{
		defaultTileId = 0;
	}

	//Copy tiles (using new aligned map size)
	std::vector<TTerrainTileDesc> tiles;
	tiles.resize(mapWidth * mapHeight);
	std::fill(tiles.begin(), tiles.end(), defaultTileId);

	for(int x = 0; x < m_width; x++)
	{
		for(int y = 0; y < m_height; y++)
		{
			TerrainTileId tileId = GetTerrainTile(x, y);
			u16 tileFlags = GetCollisionTileFlags(x, y);

			if(tileId == InvalidTerrainTileId)
			{
				tileId = defaultTileId;
			}

			tiles[(y * mapWidth) + x] = (u16)tileId | tileFlags;
		}
	}

	//Split map into NxN blocks
	int widthBlocks = mapWidth / blockWidth;
	int heightBlocks = mapHeight / blockHeight;

	m_blocks.clear();
	m_blocks.reserve(widthBlocks * heightBlocks);

	for(int blockY = 0; blockY < heightBlocks; blockY++)
	{
		for(int blockX = 0; blockX < widthBlocks; blockX++)
		{
			m_blocks.push_back(Block());
			Block& block = m_blocks.back();

			int blockStartOffset = (blockY * widthBlocks * blockWidth) + (blockX * blockWidth);

			for(int tileY = 0; tileY < blockHeight; tileY++)
			{
				for(int tileX = 0; tileX < blockWidth; tileX++)
				{
					int x = (blockX * blockWidth) + tileX;
					int y = (blockY * blockHeight) + tileY;

					int tileOffset = (y * mapWidth) + x;
					block.m_tiles.push_back(tiles[tileOffset]);
				}
			}
		}
	}
}

void CollisionMap::GetPhysicsWorldBounds(ion::Vector2i& topLeft, ion::Vector2i& size, int tileWidth, int tileHeight, int blockWidth, int blockHeight) const
{
	ion::Vector2i bottomRight;

	topLeft.x = ion::maths::S32_MAX;
	topLeft.y = ion::maths::S32_MAX;
	bottomRight.x = ion::maths::S32_MIN;
	bottomRight.y = ion::maths::S32_MIN;

	//Get terrain bezier bounds
	for(int i = 0; i < m_terrainBeziers.size(); i++)
	{
		const ion::gamekit::BezierPath& bezier = m_terrainBeziers[i].bezier;

		ion::Vector2 boundsMin;
		ion::Vector2 boundsMax;
		bezier.GetBounds(boundsMin, boundsMax);

		if(boundsMin.x < topLeft.x)
			topLeft.x = boundsMin.x;
		if(boundsMin.y < topLeft.y)
			topLeft.y = boundsMin.y;
		if(boundsMax.x > bottomRight.x)
			bottomRight.x = boundsMax.x;
		if(boundsMax.y > bottomRight.y)
			bottomRight.y = boundsMax.y;
	}

	topLeft.x /= tileWidth;
	topLeft.y /= tileHeight;
	bottomRight.x /= tileWidth;
	bottomRight.y /= tileHeight;

	//Get flagged terrain bounds
	for(int x = 0; x < m_width; x++)
	{
		for(int y = 0; y < m_height; y++)
		{
			if(GetCollisionTileFlags(x, y) != 0)
			{
				if(x < topLeft.x)
					topLeft.x = x;
				if(y < topLeft.y)
					topLeft.y = y;
				if(x > bottomRight.x)
					bottomRight.x = x;
				if(y > bottomRight.y)
					bottomRight.y = y;
			}
		}
	}

	size = bottomRight - topLeft;

	topLeft.x = ion::maths::RoundDownToNearest(topLeft.x, blockWidth);
	topLeft.y = ion::maths::RoundDownToNearest(topLeft.y, blockHeight);
	size.x = ion::maths::RoundUpToNearest(size.x, blockWidth);
	size.y = ion::maths::RoundUpToNearest(size.y, blockHeight);
}

void CollisionMap::GetPhysicsWorldBoundsBlocks(ion::Vector2i& topLeft, ion::Vector2i& size, int tileWidth, int tileHeight, int blockWidth, int blockHeight) const
{
	GetPhysicsWorldBounds(topLeft, size, tileWidth, tileHeight, blockWidth, blockHeight);
	topLeft.x = topLeft.x / blockWidth;
	topLeft.y =topLeft.y / blockHeight;
	size.x = size.x / blockWidth;
	size.y = size.y / blockHeight;
}

std::vector<CollisionMap::Block>& CollisionMap::GetBlocks()
{
	return m_blocks;
}

void CollisionMap::Export(const Project& project, std::stringstream& stream) const
{
	//Use default tile if there is one, else use first tile
	u32 defaultTileId = project.GetDefaultTerrainTile();
	if(defaultTileId == InvalidTerrainTileId)
	{
		defaultTileId = 0;
	}

	//Output to stream
	stream << std::hex << std::setfill('0') << std::uppercase;

	for(int y = 0; y < m_height; y++)
	{
		stream << "\tdc.w\t";

		for(int x = 0; x < m_width; x++)
		{
			TerrainTileId tileId = GetTerrainTile(x, y);
			u16 tileFlags = GetCollisionTileFlags(x, y);

			//If blank tile, use default tile
			u32 tileIndex = (tileId == InvalidTerrainTileId) ? defaultTileId : tileId;

			u16 word = (u16)tileIndex | tileFlags;
			stream << "0x" << std::setw(4) << (u32)word;

			if(x < (m_width - 1))
				stream << ",";
		}

		stream << std::endl;
	}

	stream << std::dec;
}

void CollisionMap::Export(const Project& project, ion::io::File& file) const
{
	//Use default tile if there is one, else use first tile
	u32 defaultTileId = project.GetDefaultTerrainTile();
	if(defaultTileId == InvalidTerrainTileId)
	{
		defaultTileId = 0;
	}

	for(int y = 0; y < m_height; y++)
	{
		for(int x = 0; x < m_width; x++)
		{
			TerrainTileId tileId = GetTerrainTile(x, y);
			u16 tileFlags = GetCollisionTileFlags(x, y);

			//If blank tile, use default tile
			u32 tileIndex = (tileId == InvalidTerrainTileId) ? defaultTileId : tileId;

			u16 word = (u16)tileIndex | tileFlags;
			ion::memory::EndianSwap(word);
			file.Write(&word, sizeof(u16));
		}
	}
}

void CollisionMap::ExportBlockMap(const Project& project, std::stringstream& stream, int blockWidth, int blockHeight) const
{
	int tileWidth = project.GetPlatformConfig().tileWidth;
	int tileHeight = project.GetPlatformConfig().tileHeight;

	ion::Vector2i topLeft;
	ion::Vector2i size;
	GetPhysicsWorldBoundsBlocks(topLeft, size, tileWidth, tileHeight, blockWidth, blockHeight);

	int widthBlocks = GetWidthBlocks(blockWidth);

	//Export block map
	stream << std::endl;
	stream << "Terrain_Block_Map:" << std::endl;

	for(int blockY = topLeft.y; blockY < (topLeft.y + size.y); blockY++)
	{
		stream << "\tdc.w\t";

		for(int blockX = topLeft.x; blockX < (topLeft.x + size.x); blockX++)
		{
			int blockId = (blockY * widthBlocks) + blockX;
			const Block& block = m_blocks[blockId];

			stream << "0x" << std::hex << std::setfill('0') << std::setw(4) << block.uniqueIndex;

			if(blockX < (size.x - 1))
			{
				stream << ",";
			}
		}

		stream << std::endl;
	}
}

void CollisionMap::ExportBlockMap(const Project& project, ion::io::File& file, int blockWidth, int blockHeight) const
{
	int tileWidth = project.GetPlatformConfig().tileWidth;
	int tileHeight = project.GetPlatformConfig().tileHeight;

	ion::Vector2i topLeft;
	ion::Vector2i size;
	GetPhysicsWorldBoundsBlocks(topLeft, size, tileWidth, tileHeight, blockWidth, blockHeight);

	int widthBlocks = GetWidthBlocks(blockWidth);

	//Export block map
	for(int blockY = topLeft.y; blockY < (topLeft.y + size.y); blockY++)
	{
		for(int blockX = topLeft.x; blockX < (topLeft.x + size.x); blockX++)
		{
			int blockId = (blockY * widthBlocks) + blockX;
			const Block& block = m_blocks[blockId];

			u16 word = block.uniqueIndex;

			//Endian flip
			ion::memory::EndianSwap(word);

			//Write
			file.Write(&word, sizeof(u16));
		}
	}
}

bool CollisionMap::Block::operator ==(const Block& rhs) const
{
	return m_tiles == rhs.m_tiles;
}

void CollisionMap::Block::Export(const Project& project, std::stringstream& stream, int blockWidth, int blockHeight)
{
	//Output to stream
	stream << std::hex << std::setfill('0') << std::uppercase;

	for(int y = 0; y < blockHeight; y++)
	{
		stream << "\tdc.w\t";

		for(int x = 0; x < blockWidth; x++)
		{
			const TTerrainTileDesc& tileDesc = m_tiles[(y * blockWidth) + x];

			//Generate word
			u16 word = tileDesc;

			//Write
			stream << "0x" << std::hex << std::setfill('0') << std::setw(4) << (u32)word;

			if(x < (blockWidth - 1))
			{
				stream << ",";
			}
		}

		stream << std::endl;
	}
}

void CollisionMap::Block::Export(const Project& project, ion::io::File& file, int blockWidth, int blockHeight)
{
	//Output to file
	for(int y = 0; y < blockHeight; y++)
	{
		for(int x = 0; x < blockWidth; x++)
		{
			const TTerrainTileDesc& tileDesc = m_tiles[(y * blockWidth) + x];

			//Generate word
			u16 word = tileDesc;

			//Endian flip
			ion::memory::EndianSwap(word);

			//Write
			file.Write(&word, sizeof(u16));
		}
	}
}

#pragma optimize( "", on )