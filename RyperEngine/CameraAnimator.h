#pragma once
#include "Camera.h"
#include <spline_library\splines\uniform_cr_spline.h>

using SplineType = UniformCRSpline<glm::vec3>;
using SplinePtr = std::shared_ptr<SplineType>;
struct CameraAnimator
{
	CameraPtr camera;
	void SetCamera (CameraPtr camera)
	{
		this->camera = camera;
	}
	std::vector<glm::vec3> splinePoints;
	std::vector<glm::vec3> cameraDirections;
	SplinePtr spline;

	float elapsedTime = 0;
	bool isAnimating = false;
	void Update (float dt)
	{
		if (isAnimating)
		{
			elapsedTime += dt;
			const float SCALE_FACTOR = 0.1f;
			elapsedTime = glm::mod(elapsedTime, 1.0f / SCALE_FACTOR);
			float scaledTime	= elapsedTime * SCALE_FACTOR;
			glm::vec3 pos		= spline->getPosition (scaledTime); // uniform [0,1]
			camera->SetEye (pos);
		}
	}
	virtual void KeyboardDown(SDL_KeyboardEvent& key)
	{
		//if (key.keysym.sym == SDLK_k)
		//{
		//	splinePoints.push_back (camera->GetEye ());
		//}
		//if (key.keysym.sym == SDLK_l)
		//{
		//	spline.reset (new SplineType (splinePoints));
		//	//isAnimating = true;
		//}
	}
};

//class CatmullRom {
//	vector<glm::vec3>cps;// control points
//	vector<float> ts; // parameter (knot) values
//
//	using vec4 = glm::vec4;
//	using vec3 = glm::vec3;
//	
//	vec3 Hermite(vec3 p0, vec3 v0, float t0,
//		vec3 p1, vec3 v1, float t1,
//		float t) {
//
//		//Boldi fele
//		vec3 a0 = p0;
//		vec3 a1 = v0;
//		vec3 a2 = ((p1 - p0) * 3.0f / (float)powf((t1 - t0), 2)) - ((v1 + v0 * 2.0f) / (float)(t1 - t0));
//		vec3 a3 = ((p0 - p1) * 2.0f / (float)powf((t1 - t0), 3)) + ((v1 + v0) / (float)powf((t1 - t0), 2));
//		return a3 * powf((t - t0), 3) + a2 * powf((t - t0), 2) + a1 * (t - t0) + a0;
//	}
//public:
//	void AddControlPoint(glm::vec3 cp) {
//		float ti = cps.size(); // or something better
//		cps.push_back(cp); ts.push_back(ti);
//	}
//	glm::vec3 r(float t) {
//		glm::vec3 rr(0, 0, 0);
//		for (int i = 0; i <cps.size(); i++) 
//			rr += cps[i] * L(i,t);
//		return rr;
//	}
//};

struct SplineRenderer
{
	std::vector<glm::vec3> lineStripPoints;

	gl::VertexArray VAO;
	void UpdateLinestrip (SplinePtr spline)
	{
		if (spline)
		{
			lineStripPoints.clear();
			for(float t = 0.0; t <= 1.0f; t+= 0.01f)
			{
				lineStripPoints.push_back(spline->getPosition (t));
			}
		}
	}
	void Draw (RenderState& state)
	{
		gShaderProgram * shader = Shader::ShaderManager::GetShader<Shader::BoundingBox> ();
		shader->On();
		shader->SetUniform("PVM", state.PV);
		shader->SetUniform("isSelected", true);
		glLineWidth (5.0f);
		{
			auto bindVao = gl::MakeTemporaryBind(VAO);
			{
				gl::ArrayBuffer VBO;
				auto bindVBO = gl::MakeTemporaryBind(VBO);
				{
					gl::VertexAttrib attrib(0);
					attrib.pointer(3, gl::kFloat);
					attrib.enable();
					VBO.data(lineStripPoints, gl::kDynamicDraw);

					gl::TemporaryDisable cullFace(gl::kCullFace);
					gl::TemporaryEnable blend(gl::kBlend);
					gl::BlendFunc(gl::kSrcAlpha, gl::kOneMinusSrcAlpha);

					gl::DrawArrays(gl::kLineStrip, 0, lineStripPoints.size());
				}
			}
		}
		shader->Off();
	}
};