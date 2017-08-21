#pragma once

#include <GL\glew.h>
#include <SDL.h>

#include "glmIncluder.h"
#include "UtilEngine.h"
#include "Renderers.h"
#include <string>
#include <functional>
#include "TextRenderer.h"
#include "WrapperStructs.h"

struct Widget {
	glm::ivec2 pos;
	glm::ivec2 size;

	Widget (glm::ivec2 pos, glm::ivec2 size)
		: pos(pos), size(size)
	{}

	bool isSelected = false;
	bool isVisible  = true;

	virtual void SetPos(glm::ivec2 newPos)
	{
		pos = newPos;
	}
	virtual glm::ivec2 GetSize() {return size; }
	virtual bool isInsideMouse(int x, int y) 
	{
		return pos.x <= x && pos.x + size.x >= x &&
			   pos.y <= y && pos.y + size.y >= y;
	}
	virtual void MouseMove(SDL_MouseMotionEvent&) {};
	virtual void MouseDown(SDL_MouseButtonEvent&) {};
	virtual void MouseUp(SDL_MouseButtonEvent&) {};
	virtual void MouseWheel(SDL_MouseWheelEvent&) {};
	virtual void Draw(WidgetRenderState &state) {}

	static glm::mat4 GetModelTransform(glm::ivec2 const& pos, 
		glm::ivec2 const& size, glm::ivec2 const& screenSize)
	{
		//Full screen quad to local size
		glm::vec3 scale = glm::vec3(glm::vec2(size) / glm::vec2(screenSize), 1);
		//Origo to the top left corner
		glm::vec3 translateOrigo = glm::vec3(scale.x, -scale.y, 0);
		glm::vec3 translatePos = glm::vec3(Util::pixelToNdc(pos, screenSize), 0);

		glm::mat4 model = glm::translate(translatePos)*
			glm::translate(translateOrigo)*
			glm::scale(scale);

		return model;
	}
};

struct ButtonA : public Widget
{
	std::string text;
	std::function<void()> callback;
	ButtonA (glm::ivec2 pos, glm::ivec2 size, std::string text, std::function<void()> fv = [](){})
		:Widget(pos, size), text(text), callback(fv)
	{}

	bool isClicked;
	virtual void MouseDown(SDL_MouseButtonEvent& mouse) override
	{
		if (mouse.button == SDL_BUTTON_LEFT && isInsideMouse (mouse.x, mouse.y))
		{
			callback();
			isClicked = !isClicked;
		}
	}
	virtual void Draw (WidgetRenderState &state) override
	{
		glDisable(GL_DEPTH_TEST);
		glm::mat4 model = Widget::GetModelTransform(pos, size, state.screenSize);

		//TODO Render texture
		if(isClicked)
			state.texturer.Draw(glm::vec4(0,1,0,1), model);
		else
			state.texturer.Draw(glm::vec4(1, 0, 0, 1), model);

		TextData data = state.textRenderer.RenderStrToTexture (text);
		//data.size     = size;
		if(data.texCoord != -1)
			state.textRenderer.RenderStr (std::move(data), model);
		glEnable(GL_DEPTH_TEST);
	}
};

struct Checkbox : public Widget
{
	TextRenderer& textRenderer;
	ButtonA button;
	bool * value;
	std::string text;
	Checkbox(glm::ivec2 pos, glm::ivec2 size, std::string text, bool * value, TextRenderer& textRenderer)
		:Widget(pos, size),
		button(pos, glm::ivec2(size.y,size.y), ""),
		text(text),
		value(value),
		textRenderer(textRenderer)
	{
		button.callback  = [value, this](){
			*value = !(*value);
			if (*value)
				button.text = "X"; //Checkbox like text
			else
				button.text = "";
		};
		button.isClicked = *value;
	}
	virtual void MouseDown(SDL_MouseButtonEvent& mouse) override
	{
		button.MouseDown (mouse);
	}
	virtual void SetPos(glm::ivec2 newPos) override
	{
		pos = newPos;
		button.SetPos (newPos);
	}
	virtual glm::ivec2  GetSize() override
	{
		glm::ivec2 buttonSize(size.y, size.y);
		buttonSize.x += textRenderer.TextSize(text).x;
		return  buttonSize;
	}
	virtual void Draw(WidgetRenderState &state) override
	{
		button.Draw (state);

		//TODO Render text
		TextData data = textRenderer.RenderStrToTexture (text);
		size.x = size.y + data.size.x;

		glm::ivec2 textPos = glm::ivec2(pos.x + size.y, pos.y);
		glm::ivec2 textSize = glm::ivec2(size.x - size.y, size.y);
		glm::mat4 model = Widget::GetModelTransform(textPos, textSize, state.screenSize);

		glEnable(GL_BLEND);
		state.texturer.Draw (data.texCoord,false, model);
		glDisable(GL_BLEND);

		glDeleteTextures (1,&data.texCoord);
	}
};

struct TextWidget : public Widget
{
	std::function<std::string()> stringCreator;
	TextRenderer& textRenderer;
	TextWidget(glm::ivec2 pos, glm::ivec2 size, TextRenderer& textRenderer, std::function<std::string()> stringCreator = []() {return "";})
		:Widget(pos, size), stringCreator(stringCreator), textRenderer(textRenderer)
	{}
	virtual glm::ivec2 GetSize() override
	{
		glm::ivec2 buttonSize(size.y, size.y);
		buttonSize.x += textRenderer.TextSize(stringCreator()).x;
		return  buttonSize;
	}
	virtual void Draw(WidgetRenderState &state) override
	{
		glDisable(GL_DEPTH_TEST);
		glm::mat4 model = Widget::GetModelTransform(pos, size, state.screenSize);
		std::string str = stringCreator();
		size.x = textRenderer.TextSize(str).x;

		TextData data = state.textRenderer.RenderStrToTexture(str);
		if (data.texCoord != -1)
			state.textRenderer.RenderStr(std::move(data), model);
		glEnable(GL_DEPTH_TEST);
	}
};

struct Container : public Widget
{
	glm::ivec2 padding;
	glm::ivec2 offset;
	using WidgetVec = std::vector<WidgetPtr>; 
	WidgetVec children;
	Container (glm::ivec2 pos)
		:Widget(pos, glm::ivec2(0)), padding(3), offset(4)
	{
	}
	void AddWidget (WidgetPtr widget)
	{
		glm::ivec2 newPos(0);
		if (children.size() > 0)
		{	
			WidgetPtr lastWidget = children.back (); ///TODO We need relative pos for the parent for moving etc -> Or call update pos after every move
			//TODO add getSize method
			newPos.y += lastWidget->size.y + lastWidget->pos.y;
			newPos.x += lastWidget->pos.x;
			newPos.y += offset.y; //Separate each widget
		}
		else
		{
			newPos += padding;
			newPos += pos;

			size += padding;
		}
		children.push_back(widget);
		widget->SetPos(newPos);

		size += widget->GetSize () + offset.y;
		size.x = MaxWidgetWidth () + 2 * padding.x;
	}
	void UpdatePos ()
	{
		WidgetVec childrenOld = children;
		children.clear();
		for(auto& widget : childrenOld)
			AddWidget(widget);
	}
	void Draw(WidgetRenderState &state) override
	{
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		state.texturer.Draw (glm::vec4(0.9, 0.9,0.9, 0.3), Widget::GetModelTransform (pos, size, state.screenSize));
		glDisable(GL_BLEND);

		for(auto& widget : children)
			widget->Draw (state);

		glEnable(GL_DEPTH_TEST);
	}
	virtual void MouseMove(SDL_MouseMotionEvent& mouse) 
	{
		for(auto& widget : children)
			widget->MouseMove(mouse);
	};
	virtual void MouseDown(SDL_MouseButtonEvent& mouse) 
	{
		for (auto& widget : children)
			widget->MouseDown(mouse);
	};
	virtual void MouseUp(SDL_MouseButtonEvent& mouse) 
	{
		for (auto& widget : children)
			widget->MouseUp(mouse);
	};
	virtual void MouseWheel(SDL_MouseWheelEvent& mouse) 
	{
		for (auto& widget : children)
			widget->MouseWheel(mouse);
	};
private:
	int MaxWidgetWidth()
	{
		int max = 0;
		for(auto widget : children)
		{
			if (widget->GetSize ().x > max)
				max = widget->GetSize().x;
		}
		return max;
	}
};