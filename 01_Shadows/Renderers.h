#pragma once

#include "Camera.h"
#include "glmIncluder.h"
#include "gShaderProgram.h"
#include "Geometry.h"
#include <oglwrap\oglwrap.h> //TODO

#include "Defs.h"

class FrustumRenderer
{
	gShaderProgram * shader;
public:
	FrustumRenderer (gShaderProgram * shader)
		:shader(shader)
	{}
	void Render(glm::mat4 VP, CameraPtr camera)
	{
		FrustumG* frustum = camera->GetFrustum ();
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

struct BoundingBoxRenderer
{
	static Geometry * geom_box;
	gShaderProgram * shader;
	std::vector<GameObj*>& gameObjs;
	BoundingBoxRenderer(std::vector<GameObj*>& gameObj, gShaderProgram * shader)
		:gameObjs(gameObj), shader(shader)
	{}
	void Draw(RenderState state)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (GameObj * obj : gameObjs)
			DrawBox(state, obj);
		glDisable(GL_BLEND);
	}
	struct Ray {
		glm::vec3 origin;
		glm::vec3 direction;
		glm::vec3 dir_inv;
	};
	Ray createRay(glm::vec3 o, glm::vec3 d)
	{
		Ray ray;
		ray.origin = o;
		ray.direction = glm::normalize(d);
		ray.dir_inv = (1.0f / d);
		return ray;
	}
	float intersection(Geom::Box &b, Ray const& r) {
		float t1 = (b.min[0] - r.origin[0])*r.dir_inv[0];
		float t2 = (b.max[0] - r.origin[0])*r.dir_inv[0];

		float tmin = glm::min(t1, t2);
		float tmax = glm::max(t1, t2);

		for (int i = 1; i < 3; ++i) {
			t1 = (b.min[i] - r.origin[i])*r.dir_inv[i];
			t2 = (b.max[i] - r.origin[i])*r.dir_inv[i];

			tmin = glm::max(tmin, glm::min(t1, t2));
			tmax = glm::min(tmax, glm::max(t1, t2));
		}

		bool intersect = tmax > glm::max(tmin, 0.0f);
		if (intersect)
			return tmin;

		return -1.0;
	}
	int FindObject(glm::vec3 eye, glm::vec3 world)
	{
		Ray ray = createRay(eye, world - eye);
		float smallest = -1.0f;
		int savedIndex = -1;
		for (int i = 0; i < gameObjs.size(); i++)
		{
			GameObj* obj = gameObjs[i];
			obj->isSelected = false; //Mellekhatas
			Geom::Box box = obj->geometry->getModelBox(obj->pos, obj->scale, obj->quaternion);
			float t = intersection(box, ray);
			if ((smallest > t || savedIndex == -1) && t >= 0)
			{
				savedIndex = i;
				smallest = t;
			}
		}
		if (savedIndex >= 0)
			gameObjs[savedIndex]->isSelected = true;

		return savedIndex;
	}
private:
	void DrawBox(RenderState state, GameObj* obj)
	{
		shader->On();
		state.M = obj->geometry->getModelMatrixForBoxGeom(obj->pos, obj->scale, obj->quaternion);
		state.Minv = glm::inverse(state.M);
		glm::mat4 PVM = state.PV * state.M;
		shader->SetUniform("isAnimated", false);
		shader->SetUniform("isSelected", obj->isSelected);
		shader->SetUniform("PVM", PVM);
		shader->SetUniform("M", state.M);
		shader->SetUniform("Minv", state.Minv);

		geom_box->buffer.On();
		geom_box->buffer.Draw(GL_TRIANGLES);
		geom_box->buffer.Off();
		shader->Off();
	}
};

struct QuadTexturer
{
	gShaderProgram* shader;
	Geometry * geom;
	QuadTexturer(Geometry * geom, gShaderProgram* shader)
		:shader(shader), geom(geom)
	{}
	void Draw(GLuint texId, bool isInvertY = false, glm::mat4 Model = glm::mat4(1.0))
	{
		shader->On(); //Shader debug texturer
		{
			shader->SetTexture("loadedTex", 15, texId);
			shader->SetUniform("M", Model);
			shader->SetUniform("isInvertY", isInvertY);
			shader->SetUniform("isTexture", true);

			geom->Draw();
		}
		shader->Off();
	}
	void Draw(glm::vec4 color, glm::mat4 Model = glm::mat4(1.0))
	{
		shader->On(); //Shader debug texturer
		{
			shader->SetUniform("M", Model);
			shader->SetUniform("isTexture", false);
			shader->SetUniform("uColor", color);

			geom->Draw();
		}
		shader->Off();
	}
};