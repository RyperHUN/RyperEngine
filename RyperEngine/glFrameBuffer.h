#pragma once

#include <GL\glew.h>
#include <iostream>

#include <oglwrap\oglwrap.h>

class glFrameBuffer : public gl::Framebuffer
{
	gl::Renderbuffer rbo;
public:
	gl::Texture2D texture;
	glFrameBuffer()
	{
	}
	void On ()
	{
		gl::Bind (*this);
	}
	void Off ()
	{
		gl::Unbind (*this);
	}

	//Creates FBO with Depth + Color attachement
	void CreateAttachments(int width, int height)
	{
		texture = gl::Texture2D {};
		rbo = gl::Renderbuffer {};

		auto val = gl::MakeTemporaryBind (*this);
		{
			CreateDepthBufferAndStencil(width, height);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
	}
	//Creates FBO with only Depth attachement
	void CreateShadowAttachment (int width, int height)
	{
		texture = gl::Texture2D{};
		rbo = gl::Renderbuffer{};

		auto val = gl::MakeTemporaryBind (*this);
		{
			{
				auto bindTexture = gl::MakeTemporaryBind (texture);

				texture.upload (gl::kDepthComponent, width, height, gl::kDepthComponent, gl::kFloat, 0);
				texture.minFilter (gl::kNearest);
				texture.magFilter(gl::kNearest);
				texture.wrapS (gl::kClampToEdge);
				texture.wrapT (gl::kClampToEdge);
			}

			this->attachTexture(gl::kDepthAttachment , texture, 0);

			gl::DrawBuffer(gl::kNone);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: ShadowFramebuffer is not complete!" << std::endl;
		}
	}
	//Creates FBO with depth + Multisampled color attachement
	void CreateMultisampleAttachments(int width, int height)
	{
		texture = gl::Texture2D{};
		rbo = gl::Renderbuffer{};

		glBindFramebuffer(GL_FRAMEBUFFER, this->expose ());
		{
			static const int samples = 4;
			
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture.expose ());
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
			}
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

			//Attach to FBO
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture.expose(), 0);

			//CReate DEPTH render buffer object
			//RBO faster > texture but write only
			glBindRenderbuffer(GL_RENDERBUFFER, rbo.expose ());
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			//Attach
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo.expose ());

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
private:
	///Have to call on before init
	void CreateDepthBufferAndStencil (int width, int height)
	{
		{
			auto val = gl::MakeTemporaryBind (texture);
			texture.upload (gl::kRgb, width, height, gl::kRgb, gl::kUnsignedByte, NULL);
			texture.minFilter (gl::kLinear);
			texture.magFilter (gl::kLinear);
			texture.wrapS (gl::kClampToEdge);
			texture.wrapT (gl::kClampToEdge);
			
		}
		// attach it to currently bound framebuffer object
		this->attachTexture (gl::kColorAttachment0, texture, 0);

		//CReate DEPTH render buffer object
		//RBO faster > texture but write only
		{
			auto val = gl::MakeTemporaryBind (rbo);
			
			rbo.storage (gl::kDepthStencil, width, height);
		}
		//Attach
		this->attachBuffer(gl::kDepthStencilAttachment, rbo);
	}
};
