#pragma once

#include <GL\glew.h>
#include <iostream>

class glFrameBuffer
{
	GLuint FBO;
	GLuint rboId = -1;
public:
	GLuint textureId = -1;
	glFrameBuffer()
	{
		glGenFramebuffers(1, &FBO);
	}
	void On ()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	}
	void Off ()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	//Returns tex id
	GLuint CreateAttachments(int width, int height)
	{
		if (textureId != -1)
			glDeleteTextures (1 , &textureId);
		if (rboId != -1)
			glDeleteRenderbuffers(1, &rboId);

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		{
			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_2D, textureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			// attach it to currently bound framebuffer object
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

			//CReate DEPTH render buffer object
			//RBO faster > texture but write only
			glGenRenderbuffers(1, &rboId);
			glBindRenderbuffer(GL_RENDERBUFFER, rboId);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			//Attach
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return textureId;
	}
};