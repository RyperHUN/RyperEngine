#pragma once

#include "Camera.h"
#include "glmIncluder.h"
#include "gShaderProgram.h"
#include "ChunkManager.h"
#include "Geometry.h"
#include <oglwrap\oglwrap.h> //TODO
#include "Ray.h"
#include <map>

#include "Defs.h"
#include "glQuery.h"

class FrustumRenderer
{
	Shader::BoundingBox * shader;
public:
	FrustumRenderer ()
	{
		shader = Shader::ShaderManager::GetShader<Shader::BoundingBox>();
	}
	void Render(glm::mat4 VP, CameraPtr camera)
	{
		shader->On();
		shader->SetUniform ("PVM", VP);
		shader->SetUniform ("isSelected", true);
		{
			gl::VertexArray VAO;
			auto bindVao = gl::MakeTemporaryBind (VAO);
			{
				gl::ArrayBuffer VBO;
				auto bindVBO = gl::MakeTemporaryBind (VBO);
				{
					std::vector<glm::vec3> triangles;
					FillVectorWithFrustumData (triangles, camera->GetFrustum());

					gl::VertexAttrib attrib(0);
					attrib.pointer(3, gl::kFloat);
					attrib.enable();
					VBO.data (triangles, gl::kDynamicDraw);
					
					gl::TemporaryDisable cullFace(gl::kCullFace);
					gl::TemporaryEnable blend (gl::kBlend);
					gl::BlendFunc (gl::kSrcAlpha, gl::kOneMinusSrcAlpha);
					
					gl::DrawArrays(gl::kTriangleFan, 0, triangles.size() * 3);
				}
			}
		}
		shader->Off();
	}
	//TODO Fix triangles better visualization
	static void FillVectorWithFrustumData (std::vector<glm::vec3> &triangles, FrustumG* frustum)
	{
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
	}
};

struct BoundingBoxRenderer
{
	static Geometry * geom_box;
	Shader::BoundingBox* shader;
	std::vector<GameObj*>& gameObjs;
	BoundingBoxRenderer(std::vector<GameObj*>& gameObj)
		:gameObjs(gameObj)
	{
		shader = Shader::ShaderManager::GetShader<Shader::BoundingBox>();
	}
	void Draw(RenderState state)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (GameObj * obj : gameObjs)
			DrawBox(state, obj->geometry->getModelMatrixForBoxGeom(obj->pos, obj->scale, obj->quaternion), obj->isSelected);
		glDisable(GL_BLEND);
	}
	void DrawChunks (RenderState &state, ChunkManager &manager)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for(int i = 0 ; i < manager.chunks.size(); i++)
		{
			Chunk& chunk = (*manager.chunks[i]);
			bool isSelected = false;
			for(auto &val: chunkIndices)
				if (val.second == i)
					isSelected = true;
			//DrawChunkBox (state, chunk);
			DrawBox(state, chunk.getBox().GetLocalMatrix(), isSelected);
		}
		glDisable(GL_BLEND);
	}
	void DrawChunkBox (RenderState &state, Chunk& chunk)
	{
		for(int i = 0 ; i < Chunk::BlockCount (); i++)
		{
			Geom::Box box;
			if(chunk.GetBoxForBlock (i,box))
			{
				DrawBox (state, box.GetLocalMatrix (), false);
			}
		}
	}
	int FindObject(glm::vec3 eye, glm::vec3 world)
	{
		Ray ray = Ray::createRay(eye, world - eye);
		float smallest = -1.0f;
		int savedIndex = -1;
		for (int i = 0; i < gameObjs.size(); i++)
		{
			GameObj* obj = gameObjs[i];
			obj->isSelected = false; //Mellekhatas
			Geom::Box box = obj->geometry->getModelBox(obj->pos, obj->scale, obj->quaternion);
			float t = Ray::intersection(box, ray);
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
	std::map<float,size_t> chunkIndices;
	void FindChunk (glm::vec3 eye, glm::vec3 world, ChunkManager &manager)
	{
		chunkIndices.clear();
		Ray ray = Ray::createRay(eye, world - eye);
		float smallest = -1.0f;
		int savedIndex = -1;
		for (int i = 0; i < manager.chunks.size(); i++)
		{
			Chunk& obj = (*manager.chunks[i]);
			Geom::Box box = obj.getBox ();
			float t = Ray::intersection(box, ray);
			if (t <= 0)
				t = Ray::checkinside(box, ray);
			if ((savedIndex == -1) && t >= 0)
			{
				if (smallest > t)
				{
					savedIndex = i;
					smallest = t;
				}
				chunkIndices[t] = i;
			}
		}
		FindChunkBox (ray, manager);
	}
	void FindChunkBox (Ray ray,ChunkManager &manager)
	{
		for(auto &val : chunkIndices)
		{
			if (val.first > 50.0f) //Max distance for raytracing
				break;
			
			int savedIndex = -1;
			float smallest = -1.0f;
			Chunk& chunk = (*manager.chunks[val.second]);
			for(int i = 0 ; i < Chunk::BlockCount (); i++)
			{
				Geom::Box box;
				if(chunk.GetBoxForBlock (i,box))
				{
					float t = Ray::intersection(box, ray);
					if ((smallest > t || savedIndex == -1) && t >= 0)
					{
						savedIndex = i;
						smallest = t;
						chunkIndices[t] = i;
					}
				}
			}
			if (savedIndex >= 0)
			{
				auto index = Chunk::D3Index::convertIto3DIndex(savedIndex);
				BlockData & block = chunk.chunkInfo[index.x][index.y][index.z];
				block.isExist = false;
				chunk.ChunkModified (block);
				return; //found the block
			}
			
		}
	}
private:
	void DrawBox(RenderState state, glm::mat4 M, bool isSelected)
	{
		shader->On();
		glm::mat4 Minv = glm::inverse(M);
		glm::mat4 PVM = state.PV * M;
		shader->SetUniform("uIsAnimated", false);
		shader->SetUniform("isSelected", isSelected);
		shader->SetUniform("PVM", PVM);
		shader->SetUniform("M", M);
		shader->SetUniform("Minv", Minv);

		geom_box->buffer.On();
		geom_box->buffer.Draw(GL_TRIANGLES);
		geom_box->buffer.Off();
		shader->Off();
	}
};

struct QuadTexturer
{
	enum POS
	{
		TOP_RIGHT, TOP_LEFT, BOTTOM_LEFT, BOTTOM_RIGHT, FULL_CENTER
	};
	Shader::QuadTexturer* shader;
	Geometry * geom;
	QuadTexturer(Geometry * geom)
		: geom(geom)
	{
		shader = Shader::ShaderManager::GetShader<Shader::QuadTexturer>();
	}
	void Draw(GLuint texId, bool isInvertY , POS pos, float scaleMultiplier = 1.0)
	{
		const glm::vec3 scale = glm::vec3(0.5 * scaleMultiplier, 0.5 * scaleMultiplier, 1);

		switch(pos)
		{
		case TOP_RIGHT:
			Draw(texId, isInvertY, glm::translate(glm::vec3(0.5, 0.5, 0))*glm::scale(scale)); //Right top corner
			break;
		case TOP_LEFT:
			Draw(texId, isInvertY, glm::translate(glm::vec3(-0.5, 0.5, 0))*glm::scale(scale)); 
			break;
		case BOTTOM_LEFT:
			Draw(texId, isInvertY, glm::translate(glm::vec3(-0.5, -0.5, 0))*glm::scale(scale));
			break;
		case BOTTOM_RIGHT:
			Draw(texId, isInvertY, glm::translate(glm::vec3(0.5, -0.5, 0))*glm::scale(scale));
			break;
		case FULL_CENTER:
			Draw(texId, isInvertY, glm::mat4(1.0));
			break;
		}
	}
	void Draw(GLuint texId, bool isInvertY = false, glm::mat4 Model = glm::mat4(1.0),
			  bool isBackPos = true, float alpha = 0.0f)
	{
		MAssert(alpha < 1.0f && alpha >= 0.0f, "Invalid alpha value");
		shader->On(); //Shader debug texturer
		{
			shader->SetTexture("loadedTex", 15, texId);
			shader->SetUniform("M", Model);
			shader->SetUniform("isInvertY", isInvertY);
			shader->SetUniform("isTexture", true);
			if (alpha > 0.0f)
			{
				shader->SetUniform("isAddedAlpha", true);
				shader->SetUniform("uAlpha", alpha);
			}
			else
				shader->SetUniform("isAddedAlpha", false);
			shader->SetUniform("isBackPos", isBackPos);

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
			shader->SetUniform("isAddedAlpha", false);
			shader->SetUniform("isBackPos", false);
			shader->SetUniform("uColor", color);

			geom->Draw();
		}
		shader->Off();
	}
	static glm::mat4 CreateCameraFacingQuadMatrix (RenderState &state, glm::vec3 wPos, glm::vec3 scale, float rotationZ = 0)
	{
		glm::mat4 model = glm::translate(wPos);
		glm::mat4 VM = Util::createVMWithoutCamRotation(state.V, model); //Quad will face the camera
		VM = VM * glm::rotate(rotationZ, glm::vec3(0,0,1));
		VM = VM * glm::scale(scale);
		VM = state.P * VM;
		return VM;
	}
};

struct SkyboxRenderer : public IRenderable
{
private:
	Shader::SkyBox* shader;
	Geometry * geom;
	GLuint textureCube;
public:
	SkyboxRenderer(Geometry * geom, GLuint textureCube)
		:geom(geom), textureCube(textureCube)
	{
		shader = Shader::ShaderManager::GetShader<Shader::SkyBox>();
	}
	virtual void Draw(RenderState & state) override
	{
		shader->On();
		{
			shader->SetUniform("rayDirMatrix", state.rayDirMatrix);
			shader->SetCubeTexture("skyBox", 14, textureCube);
			geom->Draw();
		}
		shader->Off();
	}
	void SetTexture(GLuint textureCubeID) {textureCube = textureCubeID;}
};

struct LightRenderer
{
	std::vector<Light*> lights;
	Geometry* geom;
	Shader::LightRender * shader;
	LightRenderer() {}
	LightRenderer(Geometry * geom)
		: geom(geom)
	{
		shader = Shader::ShaderManager::GetShader<Shader::LightRender>();
	}
	void AddLight(Light * light)
	{
		lights.push_back(light);
	}

	void Draw(glm::mat4 VP)
	{
		shader->On();
		{
			for (auto& light : lights)
			{
				glm::mat4 PVM = VP * glm::translate(light->position) * glm::scale(glm::vec3(0.5));
				shader->SetUniform("PVM", PVM);
				shader->SetUniform("color", light->color);
				geom->Draw();
			}
		}
		shader->Off();
	}
};

struct FlareManager
{
	const glm::vec2 SCREEN_CENTER /*(0)*/;
	std::vector<GLuint> flareTextures;
	const float spacing = 0.4f;
	FlareManager()
		:SCREEN_CENTER(0.0)
	{
		static const int AMOUNT_OF_FLARES = 9;
		flareTextures.resize(AMOUNT_OF_FLARES);
		int i = 1;
		for (auto& tex : flareTextures)
			tex = Util::TextureFromFile("Pictures/lensFlare/tex" + std::to_string(i++) + ".png");
	}
	void Draw(QuadTexturer& quadTexturer, glm::vec2 sunPosNDC)
	{
		auto blend = gl::TemporaryEnable(gl::kBlend);
		gl::BlendFunc(gl::kSrcAlpha, gl::kOne); //Additive blending
		auto depth = gl::TemporaryDisable(gl::kDepthTest);
		if (Util::CV::IsNDC(sunPosNDC)) //Sun is on the screen
		{
			//Calculate line throught center of the screen from sun
			glm::vec2 sunToCenter = SCREEN_CENTER - sunPosNDC;
			//TODO Calc brightness
			const float SunLength = glm::length(sunToCenter);
			float brightness = 1 - (SunLength / 0.7f);
			if (brightness > 0)
			{
				//Draw flares
				
				for (int i = flareTextures.size() - 1; i >= 0; i--)
				{
					glm::vec2 flarePos = sunPosNDC + sunToCenter * spacing * (float)i;
					quadTexturer.Draw(flareTextures[i], false, glm::translate(glm::vec3{ flarePos, 0 }) * glm::scale(glm::vec3(0.1)),false, brightness);
				}
			}
		}
		glDisable(GL_BLEND);
	}
};

struct SunRenderer
{
	glQuery query;
	FlareManager flareManager;
	QuadTexturer &quadTexturer;
	DirLight &sun;
	GLuint sunTexture;
	SunRenderer (QuadTexturer &quadTexturer, DirLight &sun)
		:quadTexturer(quadTexturer), sun(sun), query{GL_ANY_SAMPLES_PASSED}
	{}
	void Init (GLuint sunTexture)
	{
		this->sunTexture = sunTexture;	
	}
	void Draw (RenderState &state)
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		{
			glm::mat4 PVM = QuadTexturer::CreateCameraFacingQuadMatrix (state, GetSunPos(state.wEye), glm::vec3{24.0f});
			quadTexturer.Draw(sunTexture, false,PVM);
		}
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	bool isSunVisible = false;
	void DoOcculusionTest (RenderState &state)
	{
		if(query.isInUse() && query.isResultReady ())
		{
			isSunVisible = query.GetResult ();
		}
		if (!query.isInUse ())
		{
			gl::ColorMask (false, false, false, false); //This way we cannot see the quad
			gl::DepthMask(false);
			{
				query.StartQuery ();
				auto depth = gl::TemporaryEnable (gl::kDepthTest);
				glm::mat4 PVM = QuadTexturer::CreateCameraFacingQuadMatrix(state, GetSunPos(state.wEye), glm::vec3{ 15.0f });
				quadTexturer.Draw(2, false, PVM, true);
				query.EndQuery ();
			}
			gl::DepthMask (true);
			gl::ColorMask (true, true, true, true);
		}

	}
	//Must be called after all drawing
	void DrawLensFlareEffect (RenderState &state)
	{
		DoOcculusionTest (state);
		if (isSunVisible)
			flareManager.Draw(quadTexturer, GetSunPosNdc(state));
	}
	glm::vec3 GetSunPos (glm::vec3 wEye)
	{
		glm::vec3 pos = -sun.direction * 100.f;
		return  wEye + pos;
	}
	glm::vec2 GetSunPosNdc (RenderState &state)
	{
		glm::vec3 wPos = GetSunPos (state.wEye);
		glm::mat4 PVM = state.PV;

		return Util::CV::Transform (PVM, wPos); //TODO Fix this offset??
	}
};

