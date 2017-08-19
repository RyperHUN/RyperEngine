#include "ChunkManager.h"

const float Chunk::wHalfExtent = 4.0f;
Event::IBlockChanged * Chunk::blockChangedEvent = nullptr;