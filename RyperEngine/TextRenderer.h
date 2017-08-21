#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include <iostream>
#include "glmIncluder.h"
#include "UtilEngine.h"
#include "Renderers.h"

struct TextData
{
	glm::ivec2 size;
	GLuint texCoord; //Texture must be deleted!
};

struct TextRenderer
{
	TTF_Font * font;
	QuadTexturer &quadTexturer;

	TextRenderer(QuadTexturer &quadTexturer)
		:quadTexturer(quadTexturer)
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
	glm::ivec2 TextSize (std::string const& str)
	{
		glm::ivec2 result(0);
		if (str == "") return result;

		if(TTF_SizeText (font, str.c_str(), &result.x, &result.y) != 0)
			assert(false); //Error, text size not succeded
		return result;
	}
	TextData RenderStrToTexture(std::string const& str)
	{
		SDL_Color red = { 255, 255, 255, 255 };
		SDL_Surface * felirat = TTF_RenderText_Blended(font, str.c_str(), red);
		if (!felirat) // no str given
			return TextData {glm::ivec2{0}, GLuint(-1)};

		//SDL_Surface * felirat = TTF_RenderUTF8_Solid(font, "Opengl Text", red);

		GLuint tex = Util::TextureFromSdlSurface(felirat);

		TextData textData{ glm::ivec2{ felirat->w, felirat->h }, tex };

		SDL_FreeSurface(felirat);

		return textData;
	}

	void RenderStr (std::string const& str, glm::mat4 const& model)
	{
		RenderStr(RenderStrToTexture(str), model);
	}

	void RenderStr (TextData &&data, glm::mat4 const& model)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		quadTexturer.Draw (data.texCoord, false, model);
		glDisable(GL_BLEND);

		glDeleteTextures (1, &data.texCoord);
	}
};