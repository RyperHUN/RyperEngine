#pragma once

#include <GL\glew.h>
#include <iostream>

#include <oglwrap\oglwrap.h>

class glFrameBuffer : public gl::Framebuffer
{
	GLuint rboId = -1;
public:
	GLuint textureId = -1;
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

	//Returns tex id
	void CreateAttachments(int width, int height)
	{
		if (textureId != -1)
			glDeleteTextures (1 , &textureId);
		if (rboId != -1)
			glDeleteRenderbuffers(1, &rboId);

		auto val = gl::MakeTemporaryBind (*this);
		{
			CreateDepthBufferAndStencil(width, height);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
	}
	void CreateShadowAttachment (int width, int height)
	{
		if (textureId != -1)
			glDeleteTextures(1, &textureId);
		if (rboId != -1)
			glDeleteRenderbuffers(1, &rboId);

		auto val = gl::MakeTemporaryBind (*this);
		{
			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_2D, textureId);
			gl::Texture2D texture (textureId);
			{
				auto bindTexture = gl::MakeTemporaryBind (texture);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
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

	void CreateMultisampleAttachments(int width, int height)
	{
		if (textureId != -1)
			glDeleteTextures(1, &textureId);
		if (rboId != -1)
			glDeleteRenderbuffers(1, &rboId);

		glBindFramebuffer(GL_FRAMEBUFFER, this->expose ());
		{
			static const int samples = 4;
			glGenTextures(1, &textureId);
			
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureId);
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
			}
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

			//Attach to FBO
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureId, 0);

			//CReate DEPTH render buffer object
			//RBO faster > texture but write only
			glGenRenderbuffers(1, &rboId);
			glBindRenderbuffer(GL_RENDERBUFFER, rboId);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			//Attach
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
private:
	///Have to call on before init
	void CreateDepthBufferAndStencil (int width, int height)
	{
		glGenTextures(1, &textureId);
		
		gl::Texture2D texture(textureId);
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
		glGenRenderbuffers(1, &rboId);
		gl::Renderbuffer renderBuffer(rboId); //TODO Make it class member
		{
			auto val = gl::MakeTemporaryBind (renderBuffer);
			
			renderBuffer.storage (gl::kDepthStencil, width, height);
		}
		//Attach
		this->attachBuffer(gl::kDepthStencilAttachment, renderBuffer);
	}
};
