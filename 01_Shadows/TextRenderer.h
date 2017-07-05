#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include <iostream>
#include "glmIncluder.h"
#include "UtilityFuncs.h"


struct TextData
{
	glm::ivec2 size;
	GLuint texCoord; //Texture must be deleted!
};

struct TextRenderer
{
	TTF_Font * font;

	TextRenderer()
	{
		Init();
	}
	//TODO std::vector<Widget>
	void Init()
	{
		TTF_Init();
		font = TTF_OpenFont("calibri.ttf", 26);
		if (!font) {
			std::cout << "Calibri loading failed" << std::endl;
		}
	}
	TextData RenderText(std::string const& str)
	{
		SDL_Color red = { 255, 0, 0, 255 };
		SDL_Surface * felirat = TTF_RenderText_Blended(font, str.c_str(), red);
		//SDL_Surface * felirat = TTF_RenderUTF8_Solid(font, "Opengl Text", red);

		GLuint tex = Util::TextureFromSdlSurface(felirat);

		TextData textData{ glm::ivec2{ felirat->w, felirat->h }, tex };

		SDL_FreeSurface(felirat);

		return textData;
	}
};