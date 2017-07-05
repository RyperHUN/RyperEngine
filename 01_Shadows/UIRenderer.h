#pragma once

#include <GL\glew.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "UtilityFuncs.h"

struct UIRenderer 
{
	TTF_Font * font;

	//TODO std::vector<Widget>
	void Init ()
	{
		TTF_Init();
		font = TTF_OpenFont("calibri.ttf", 32);
		if (!font) {
			std::cout << "Calibri loading failed" << std::endl;
		}
	}
	void Render ()
	{
		SDL_Color red = { 255, 0, 0, 255 };
		SDL_Surface * felirat = TTF_RenderText_Blended(font, "OpenGL Text", red);
		//SDL_Surface * felirat = TTF_RenderUTF8_Solid(font, "Opengl Text", red);

		GLuint tex = Util::TextureFromSdlSurface(felirat); //TODO Free

		glEnable(GL_BLEND);
		//Draw texture
		glDisable(GL_BLEND);

		glDeleteTextures(1, &tex);

		SDL_FreeSurface(felirat);
	}
};