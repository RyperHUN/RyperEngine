#include "ChunkManager.h"

const float Chunk::wHalfExtent = 4.0f;
Event::IBlockChanged * Chunk::blockChangedEvent = nullptr;

bool BlockTextureMapper::initialized /*= false*/;
std::map<BlockType, glm::ivec3> BlockTextureMapper::blockTextureData;
std::map<TextureType, std::string> BlockTextureMapper::textureMapping;