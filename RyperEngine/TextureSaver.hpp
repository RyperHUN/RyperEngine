#pragma once

#include "glmIncluder.h"
#include <gl/glew.h>
#include <lodepng.h>
#include "Defs.h"

struct TextureSaver	
{
	using ByteArr = std::unique_ptr<BYTE[]>;
	static void SaveTexture (GLuint texture, glm::ivec2 screenSize)
	{
		static int num = 0;

		const int numberOfComponents = 3;
		ByteArr pixels(new BYTE[screenSize.x * screenSize.y * numberOfComponents]);

		glBindTexture(GL_TEXTURE_2D, texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
		glBindTexture(GL_TEXTURE_2D, 0);

		std::string name = "TemporaryFiles/screenshot" + std::to_string(num++) + ".png";
		lodepng::encode(name, pixels.get(), screenSize.x, screenSize.y,LodePNGColorType::LCT_RGB);
	}
	//static ByteArr FlipVertical (BYTE *ptr, glm::ivec2 size)
	//{
	//	size.x = size.x * 3;
	//	ByteArr flippedImage(new BYTE(size.x * size.y * 3));
	//	for (int x = 0; x < size.x; x++) {
	//		for (int y = 0; y < size.y / 2; y++) {
	//			int temp = ptr[Util::CV::Index2Dto1D(x, y, size.x)];
	//			ptr[Util::CV::Index2Dto1D(x,y,size.x)] = ptr[Util::CV::Index2Dto1D(x, size.y - y, size.x)];
	//			ptr[Util::CV::Index2Dto1D(x,size.y - y,size.x)] = temp;
	//		}
	//	}
	//}
};