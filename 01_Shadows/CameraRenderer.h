#pragma once

#include "fpsCamera.h"
#include "glmIncluder.h"
#include <oglwrap\oglwrap.h>

class CameraRenderer
{
	gShaderProgram * shader;
public:
	CameraRenderer (gShaderProgram * shader)
		:shader(shader)
	{}
	void Render(glm::mat4 VP, std::shared_ptr<FPSCamera> camera)
	{
		FrustumG* frustum = &camera->frustum;
		shader->On();
		shader->SetUniform ("PVM", VP);
		shader->SetUniform ("isSelected", true);
		{
			//gl::VertexArray VAO;
			//gl::Bind(VAO);
			//gl::ArrayBuffer VBO;
			//gl::Bind(VBO);
			//gl::VertexAttrib attrib(0);
			//attrib.enable();

			gVertexBuffer buffer;
			
			std::vector<glm::vec3> triangles;
			triangles.push_back(frustum->nbr);	//Right side
			triangles.push_back(frustum->fbr);
			triangles.push_back(frustum->ftr);

			triangles.push_back(frustum->ntr);
			/////////////////////////////////////////
			//Left side
			triangles.push_back(frustum->nbl);
			triangles.push_back(frustum->ntl);
			triangles.push_back(frustum->ftl);
			triangles.push_back(frustum->fbl);

			//Front
			triangles.push_back(frustum->ntl);
			triangles.push_back(frustum->nbl);
			triangles.push_back(frustum->nbr);
			triangles.push_back(frustum->ntr);

			//Back
			triangles.push_back(frustum->ftl);
			triangles.push_back(frustum->fbl);
			triangles.push_back(frustum->fbr);
			triangles.push_back(frustum->ftr);

			//Top
			triangles.push_back(frustum->ntl);
			triangles.push_back(frustum->ntr);
			triangles.push_back(frustum->ftr);
			triangles.push_back(frustum->ftl);

			//Bottom
			triangles.push_back(frustum->nbl);
			triangles.push_back(frustum->fbl);
			triangles.push_back(frustum->fbr);
			triangles.push_back(frustum->nbr);

			buffer.AddAttribute(0, 3);
			for(int i = 0 ; i < triangles.size(); i++)
				buffer.AddData(0, triangles[i]);

			buffer.InitBuffers();

			buffer.On();
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
				buffer.Draw (GL_QUADS);
			glDisable(GL_BLEND);
			glEnable(GL_CULL_FACE);
			buffer.Off();

			//VBO.setup (3, gl::kFloat, 0, &triangles[0]);
			//gl::VertexAttrib(0).pointer(
			//	3, gl::kFloat, false, 0, 0).enable();

			//VBO.data(triangles, gl::kDynamicDraw);

			//gl::DrawArrays (gl::kTriangles, 0, triangles.size() * 3);
			//gl::Unbind(VBO);
			//gl::Unbind(VAO);
		}
		shader->Off();
	}
};