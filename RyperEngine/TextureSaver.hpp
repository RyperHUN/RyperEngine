#pragma once

#include "glmIncluder.h"
#include <gl/glew.h>
#include <lodepng.h>
#include "Defs.h"
#include <thread>

struct TextureSaver	
{
	int screenshotId = 0;
	static const int numberOfComponents = 3;
	using ByteArr = std::shared_ptr<BYTE>;
	std::mutex lock;
	void SaveTextureThreaded (GLuint texture, glm::ivec2 screenSize)
	{	
		ByteArr pixels(new BYTE[screenSize.x * screenSize.y * numberOfComponents]);

		int actualId = -1;
		{
			std::lock_guard<std::mutex> locked(lock);
			glBindTexture(GL_TEXTURE_2D, texture);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
			glBindTexture(GL_TEXTURE_2D, 0);
			actualId = screenshotId++;
		}
		std::thread thread{ SaveTextureFromArray, pixels, screenSize, actualId};
		thread.detach ();
	}
	static void SaveTextureFromArray(ByteArr pixels, glm::ivec2 screenSize, int num)
	{
		FlipVertical(pixels.get(), screenSize);
		std::string name = "TemporaryFiles/screenshot" + Util::to_string(num, 4, '0') + ".png";
		lodepng::encode(name, pixels.get(), screenSize.x, screenSize.y, LodePNGColorType::LCT_RGB);
	}
	static void SaveTexture (GLuint texture, glm::ivec2 screenSize)
	{	
		static int num = 0;
		ByteArr pixels(new BYTE[screenSize.x * screenSize.y * numberOfComponents]);
		glBindTexture(GL_TEXTURE_2D, texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
		glBindTexture(GL_TEXTURE_2D, 0);

		FlipVertical(pixels.get(), screenSize);
		std::string name = "TemporaryFiles/saved" + std::to_string(num) + ".png";
		lodepng::encode(name, pixels.get(), screenSize.x, screenSize.y,LodePNGColorType::LCT_RGB);
	}
	static void FlipVertical (BYTE *ptr, glm::ivec2 size)
	{
		int height = size.y;
		int width = size.x;
		int bytesPerPixel = 3;
		for (int y = 0; y < height / 2; y++)
		{
			const int swapY = height - y - 1;
			for (int x = 0; x < width; x++)
			{
				const int offset = bytesPerPixel* (x + y * width);
				const int swapOffset = numberOfComponents* (x + swapY * width);

				// Swap R, G and B of the 2 pixels
				std::swap(ptr[offset + 0], ptr[swapOffset + 0]);
				std::swap(ptr[offset + 1], ptr[swapOffset + 1]);
				std::swap(ptr[offset + 2], ptr[swapOffset + 2]);
			}
		}
	}
};