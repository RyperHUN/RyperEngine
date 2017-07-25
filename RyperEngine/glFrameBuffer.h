#pragma once

#include <GL\glew.h>
#include <iostream>

#include <oglwrap\oglwrap.h>
#include "Defs.h"


class AFrameBuffer : public gl::Framebuffer
{
public:
	AFrameBuffer()
	{
	}
	void On()
	{
		gl::Bind(*this);
	}
	void Off()
	{
		gl::Unbind(*this);
	}
	virtual void Recreate(glm::ivec2 size) = 0;
	virtual GLuint GetColorAttachment() = 0;
	virtual GLuint GetDepthAttachment() = 0;
};

//Color can be read, Depth cannot (RBO)
class glFrameBuffer : public AFrameBuffer
{
	gl::Texture2D    texture;
	gl::Renderbuffer rbo;
public:
	//Creates FBO with Depth + Color attachement
	void Recreate(glm::ivec2 size) override
	{
		texture = gl::Texture2D{};
		rbo     = gl::Renderbuffer{};

		auto val = gl::MakeTemporaryBind(*this);
		{
			CreateColorAndDepthAttach(size);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
	}
	GLuint GetColorAttachment() override
	{
		return texture.expose ();
	}
	GLuint GetDepthAttachment() override
	{
		MAssert(false, "Depth attachment cannot be accesed on glFrameBuffer");
		return -1;
	}
private:
	///Have to call on before init
	void CreateColorAndDepthAttach(glm::ivec2 size)
	{
		{
			auto val = gl::MakeTemporaryBind(texture);
			texture.upload(gl::kRgb, size.x, size.y, gl::kRgb, gl::kUnsignedByte, NULL);
			texture.minFilter(gl::kLinear);
			texture.magFilter(gl::kLinear);
			texture.wrapS(gl::kClampToBorder);
			texture.wrapT(gl::kClampToBorder);
			texture.borderColor(glm::vec4(1, 0, 0, 1));

		}
		// attach it to currently bound framebuffer object
		this->attachTexture(gl::kColorAttachment0, texture, 0);

		//CReate DEPTH render buffer object
		//RBO faster > texture but write only
		{
			auto val = gl::MakeTemporaryBind(rbo);

			rbo.storage(gl::kDepthStencil, size.x, size.y);
		}
		//Attach
		this->attachBuffer(gl::kDepthStencilAttachment, rbo);
	}
};

//Color and depth can be read
class ColorDepthFrameBuffer : public AFrameBuffer
{
	gl::Texture2D    texture;
	gl::Texture2D texDepth;
public:
	//Creates FBO with Depth + Color attachement
	void Recreate(glm::ivec2 size) override
	{
		texture  = gl::Texture2D{};
		texDepth = gl::Texture2D{};

		auto val = gl::MakeTemporaryBind(*this);
		{
			CreateColorAndDepthAttach(size);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
	}
	GLuint GetColorAttachment() override
	{
		return texture.expose();
	}
	GLuint GetDepthAttachment() override
	{
		return texDepth.expose();
	}
private:
	///Have to call on before init
	void CreateColorAndDepthAttach(glm::ivec2 size)
	{
		{
			auto val = gl::MakeTemporaryBind(texture);
			texture.upload(gl::kRgb, size.x, size.y, gl::kRgb, gl::kUnsignedByte, NULL);
			texture.minFilter(gl::kLinear);
			texture.magFilter(gl::kLinear);
			texture.wrapS(gl::kClampToBorder);
			texture.wrapT(gl::kClampToBorder);
			texture.borderColor(glm::vec4(1, 0, 0, 1));

		}
		// attach it to currently bound framebuffer object
		this->attachTexture(gl::kColorAttachment0, texture, 0);

		{
			{
				auto val = gl::MakeTemporaryBind(texDepth);

				texDepth.upload(gl::kDepthComponent, size.x, size.y, gl::kDepthComponent, gl::kFloat, 0);
				texDepth.minFilter(gl::kNearest);
				texDepth.magFilter(gl::kNearest);
				texDepth.wrapS(gl::kClampToBorder);
				texDepth.wrapT(gl::kClampToBorder);
				texDepth.borderColor(glm::vec4(1, 0, 0, 1));
			}
		}
		this->attachTexture(gl::kDepthAttachment, texDepth, 0);
	}
};

//Creates FBO with depth + Multisampled color attachement
class MultiFrameBuffer : public AFrameBuffer
{
	gl::Texture2D    texture;
	gl::Renderbuffer rbo;
public:
	void Recreate(glm::ivec2 size) override
	{
		texture = gl::Texture2D{};
		rbo = gl::Renderbuffer{};

		glBindFramebuffer(GL_FRAMEBUFFER, this->expose());
		{
			static const int samples = 4;

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture.expose());
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, size.x, size.y, GL_TRUE);
			}
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

			//Attach to FBO
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture.expose(), 0);

			//CReate DEPTH render buffer object
			//RBO faster > texture but write only
			glBindRenderbuffer(GL_RENDERBUFFER, rbo.expose());
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, size.x, size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			//Attach
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo.expose());

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	GLuint GetColorAttachment() override
	{
		MAssert(false, "Color attachment cannot be accesed on MultiFrameBuffer, you have to copy it to a single texture");
		return -1;
	}
	GLuint GetDepthAttachment() override
	{
		MAssert(false, "Depth attachment cannot be accesed on MultiFrameBuffer");
		return -1;
	}
};

//Creates FBO with only Depth attachement
class ShadowFrameBuffer : public AFrameBuffer
{
	gl::Texture2D texture;
public:
	void Recreate(glm::ivec2 size) override
	{
		texture = gl::Texture2D{};

		auto val = gl::MakeTemporaryBind(*this);
		{
			{
				auto bindTexture = gl::MakeTemporaryBind(texture);

				texture.upload(gl::kDepthComponent, size.x, size.y, gl::kDepthComponent, gl::kFloat, 0);
				texture.minFilter(gl::kNearest);
				texture.magFilter(gl::kNearest);
				texture.wrapS(gl::kClampToEdge);
				texture.wrapT(gl::kClampToEdge);
			}

			this->attachTexture(gl::kDepthAttachment, texture, 0);

			gl::DrawBuffer(gl::kNone);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: ShadowFramebuffer is not complete!" << std::endl;
		}
	}
	GLuint GetColorAttachment() override
	{
		MAssert(false, "Color attachment cannot be accesed on MultiFrameBuffer, you have to copy it to a single texture");
		return -1;
	}
	GLuint GetDepthAttachment() override
	{
		return texture.expose();
	}
};

//Frame buffer for deferred rendering.
//Stores Albedo, Normal, Position geometry data and rbo depth buffer
class gFrameBuffer : public AFrameBuffer
{
	gl::Texture2D    texPosition;
	gl::Texture2D    texNormal;
	gl::Texture2D    texAlbedo;
	gl::Renderbuffer rbo;
public:
	//Creates FBO with Depth + Color attachement
	void Recreate(glm::ivec2 size) override
	{
		texPosition = gl::Texture2D{};
		texNormal   = gl::Texture2D{};
		texAlbedo   = gl::Texture2D{};
		texPosition = gl::Texture2D{};
		rbo = gl::Renderbuffer{};

		auto val = gl::MakeTemporaryBind(*this);
		{
			{
				auto val = gl::MakeTemporaryBind(rbo);
				rbo.storage(gl::kDepthStencil, size.x, size.y);
			}
			this->attachBuffer(gl::kDepthStencilAttachment, rbo);

			ConfigureTexture (texAlbedo,size, gl::kColorAttachment2);
			ConfigureTexture (texPosition, size, gl::kColorAttachment0);
			ConfigureTexture (texNormal,  size, gl::kColorAttachment1);

			unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
			glDrawBuffers(3, attachments); ///TODO OGLWrap eqvivalent??

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
	}
	GLuint GetColorAttachment() override{return texAlbedo.expose();  }
	GLuint GetNormalAttachment()		{return texNormal.expose();  }
	GLuint GetPositionAttachment()		{return texPosition.expose();}
	GLuint GetDepthAttachment() override
	{
		MAssert(false, "Depth attachment cannot be accesed on gFrameBuffer");
		return -1;
	}
private:
	void ConfigureTexture (gl::Texture2D &texture,glm::ivec2 const& size, gl::FramebufferAttachment attachment)
	{
		auto bind = gl::MakeTemporaryBind (texture);
		texture.upload (gl::kRgb16F, size.x, size.y, gl::kRgb, gl::kFloat, nullptr);
		texture.minFilter (gl::kNearest);
		texture.magFilter (gl::kNearest);
		this->attachTexture (attachment, texture);
	}
};