#pragma once

#include "FireworkParticle.h"
#include <bitset>

namespace Engine
{
namespace Particle
{
	
struct Particle 
{
	glm::vec3 position;
	glm::vec3 velocity;
	float gravityEffect;
	float lifeLength;
	float rotationZ;
	float scale;
	float alpha;
	float elapsedTime = 0;
	Particle (glm::vec3 pos, glm::vec3 velocity, float gravityEffect, float lifeLength, float rotationZ = 0, float scale = 1, float alpha = 1)
		:position(pos), velocity(velocity), gravityEffect (gravityEffect), lifeLength(lifeLength), rotationZ(rotationZ), scale(scale), alpha(alpha)
	{}
	bool IsAlive ()
	{
		return elapsedTime < lifeLength;
	}
};

enum RenderType
{
	ADDITIVE, //Shiny feeling
	ALPHA_BLENDED // Ordered
};

class ParticleSystem 
{
public:
	struct Settings {
		bool isTextured, isArrayTextured, isAlphaAdded;
	};
private:
	enum SETTINGS
	{
		eIsTextured = 0,
		eIsArrayTextured = 1,
		eIsAlphaAdded = 2,
	};
	std::bitset<3> settingsBits;
	int textureArrayLayers;
	GLuint texId;
	std::vector<Particle> particles;
	std::function<bool(float dt, Particle&)> updateFv;
	std::function<Particle()>      regenerateFv;
public:
	ParticleSystem (GLuint texId, int textureArrayLayers,Settings settings)
		:textureArrayLayers(textureArrayLayers), texId(texId)
	{
		settingsBits[SETTINGS::eIsTextured] = settings.isTextured;
		settingsBits[SETTINGS::eIsArrayTextured] = settings.isArrayTextured;
		settingsBits[SETTINGS::eIsAlphaAdded] = settings.isAlphaAdded;
	}
	void SetFunctions (std::function<bool(float dt, Particle&)> update, std::function<Particle()> regenerate)
	{
		updateFv = update;
		regenerateFv = regenerate;
	}
	void GenParticles (int particleNum)
	{
		MAssert(updateFv && regenerateFv, "Update or Regenerate particle function is not set");

		for (int i = 0; i < particleNum; i++)
		{
			particles.reserve(particleNum);
			particles.push_back(regenerateFv());
		}
	}
	void Update(float dt)
	{
		MAssert (updateFv && regenerateFv, "Update or Regenerate particle function is not set");

		for (auto & particle : particles)
		{
			if(!updateFv (dt, particle))
				particle = regenerateFv ();
		}
	}
	const std::vector<Particle>& GetParticles () const {return particles;}
	void Sort (RenderState & state)
	{
		std::sort(particles.begin(), particles.end(), [&state](Particle& left, Particle& right) {  //TODO Multithreading
			return glm::length2(left.position - state.wEye) > glm::length2(right.position - state.wEye);
			//Farthest away in front of vector
		});
	}
	int GetTextureArrayLayers() const { return textureArrayLayers; }
	size_t ParticlesNum ()const {	return particles.size();							}
	GLuint GetTexture () const	{	return texId;										}
	bool IsTextured ()	const	{	return settingsBits[SETTINGS::eIsTextured];			}
	bool IsArrayTextured ()const{	return settingsBits[SETTINGS::eIsArrayTextured];	}
	bool IsAlphaAdded () const	{	return settingsBits[SETTINGS::eIsAlphaAdded];		}
};

//TODO More types
namespace FULL 
{
	struct InstanceData {
		float alpha;
		glm::mat4 MVP;
		float texId;
	};
	static inline gl::ArrayBuffer UploadVBOFull (RenderState &state, ParticleSystem & system)
	{
		gl::ArrayBuffer instancedVBO;
		std::vector<InstanceData> data;
		data.reserve(system.GetParticles().size());
		for (auto const& particle : system.GetParticles ())
		{
			float texId = (particle.elapsedTime / particle.lifeLength) / (1 / (float)system.GetTextureArrayLayers ());
			data.push_back(InstanceData{ particle.alpha,
				QuadTexturer::CreateCameraFacingQuadMatrix(state, particle.position, glm::vec3(particle.scale), particle.rotationZ), texId });
		}

		auto bind = gl::MakeTemporaryBind(instancedVBO);
		instancedVBO.data(data, gl::kDynamicDraw);
		return instancedVBO;
	}
	static inline void SetAttribPointers ()
	{
		gl::VertexAttrib alpha(3);
		alpha.divisor(1);
		alpha.enable();
		alpha.pointer(1, gl::kFloat, false, sizeof(InstanceData), (void*)offsetof(InstanceData, alpha));

		for (int i = 0; i < 4; i++)
		{
			gl::VertexAttrib mat4(4 + i);
			mat4.enable();
			mat4.divisor(1);
			size_t ptrOffset = offsetof(InstanceData, MVP) + i * sizeof(glm::vec4);
			mat4.pointer(4, gl::kFloat, false, sizeof(InstanceData), (void*)ptrOffset);
		}
		gl::VertexAttrib texId(8);
		texId.enable();
		texId.divisor(1);
		texId.pointer(1, gl::kFloat, false, sizeof(InstanceData), (void*)offsetof(InstanceData, texId));
	}
}// NS Full

static inline bool FullParticleUpdaterFountain(float dt, Engine::Particle::Particle& p) {
	static const float GRAVITY = -10.0f;
	p.velocity.y = p.velocity.y + GRAVITY * p.gravityEffect * dt;
	glm::vec3 movementDt = p.velocity * dt;
	p.position = p.position + movementDt;
	p.elapsedTime += dt;

	return p.elapsedTime < p.lifeLength;
}
static inline Particle FullParticleRegeneratorFountain()
{
	glm::vec3 initPos = glm::vec3(0, 48, 0);
	glm::vec3 velocity = Util::randomVec(-1, 1);
	velocity.y = glm::max(0.5f, velocity.y) * 2;
	velocity *= 5.0f;
	return Engine::Particle::Particle{ initPos, velocity,
		Util::randomPoint(0.5,1), //Gravity
		3.5,
		//Util::randomPoint(1, 8),  //Life length
		Util::randomPoint(0,glm::pi<float>()), //rotationZ
		Util::randomPoint(1,2),  //scale
		0.5 }; // Alpha
}
static inline bool ParticleUpdateSmoke (float dt, Engine::Particle::Particle& p)
{
	glm::vec3 movementDt = p.velocity * dt;
	p.position = p.position + movementDt;
	p.elapsedTime += dt;
	p.alpha = 1.0f - (p.elapsedTime / p.lifeLength) * 0.7;

	return p.elapsedTime < p.lifeLength;
}
static inline Particle ParticleRegeneratorSmoke ()
{
	glm::vec3 initPos = glm::vec3(0, 48, 0);
	glm::vec3 velocity = glm::vec3 (
		Util::randomPoint (-0.1, 0.1), 
		Util::randomPoint(0.8,1.0),
		Util::randomPoint(-0.1,0.1));
	velocity *= 4.0f;
	return Engine::Particle::Particle{ initPos, velocity,
		0.1f, //Gravity
		Util::randomPoint(4, 7),  //Life length
		0, //rotationZ
		2,  //scale
		1.0 }; // Alpha
}

static inline Particle ParticleRegenFire()
{
	float diameterAroundCenter = 3.0f;
	float diameterAroundCenterHalf = diameterAroundCenter / 2.0f;
	glm::vec3 offset (Util::randomVec());
	offset = offset / glm::vec3(3,10,3) * diameterAroundCenter - glm::vec3(diameterAroundCenterHalf);

	glm::vec3 initPos = glm::vec3(0, 48, 0) + offset;

	glm::vec3 velocity (Util::randomVec());
	velocity = velocity * glm::vec3(0.01,0.1, 0.02) * 4.0f;
	if (offset.x > 0) {
		velocity.x *= -1;
	}
	if (offset.z > 0) {
		velocity.z *= -1;
	}

	return Engine::Particle::Particle{ initPos, velocity,
		0.0f, //Gravity
		Util::randomPoint(0, 8),  //Life length
		0, //rotationZ
		2,  //scale
		1.0 }; // Alpha
}

class ParticleRenderer
{
	Shader::QuadTexturerInstanced *shader;
	struct ParticleRenderData
	{
		const RenderType type;
		std::function<gl::ArrayBuffer(RenderState &state, ParticleSystem & system)> uploadVBO;
		std::function<void()> setAttribPointers;
	};
	std::vector<ParticleSystem> particleSystems;
	std::vector<ParticleRenderData> renderDatas;
public:
	ParticleRenderer ()
	{
		shader = Shader::ShaderManager::GetShader <Shader::QuadTexturerInstanced> ();
	}
	
	void AddParticleSystem (ParticleSystem &&system, RenderType type)
	{
		renderDatas.push_back (ParticleRenderData{type, FULL::UploadVBOFull, FULL::SetAttribPointers});
		particleSystems.push_back(std::move(system));
	}
	void Update (float dt)
	{
		for (auto & particleSystem : particleSystems)
			particleSystem.Update (dt);
	}
	void Draw (RenderState & state)
	{
		for (int i = 0 ; i < particleSystems.size(); i++)
		{
			auto& particleSystem = particleSystems[i];
			auto& renderData       = renderDatas[i];
			
			gl::ArrayBuffer instancedVBO = renderData.uploadVBO (state, particleSystem); //TODO Multiple threads
			prepareDraw (state, renderData.type, particleSystem);
			UploadUniforms (particleSystem);

			Geometry * geom = Geom::GeometryManager::GetGeometry<Geom::Primitive::Quad> ();
			geom->DrawInstanced (particleSystem.ParticlesNum (), instancedVBO, renderData.setAttribPointers);
		
			endDraw();
		}
	}
private:
	void UploadUniforms (ParticleSystem& particleSystem)
	{
		shader->SetUniform("isTextureArray", particleSystem.IsArrayTextured());
		shader->SetUniform("isTexture", particleSystem.IsTextured());
		shader->SetUniform("isAddedAlpha", particleSystem.IsAlphaAdded());
		if (particleSystem.IsTextured())
		{
			if (particleSystem.IsArrayTextured())
				shader->SetTexture("texArray", 13, particleSystem.GetTexture(), GL_TEXTURE_2D_ARRAY);
			else
				shader->SetTexture("loadedTex", 13, particleSystem.GetTexture());
		}
		shader->SetUniform("uColor", glm::vec4(0, 1, 0, 1)); //TODO Set color
	}

	void prepareDraw (RenderState & state, RenderType type, ParticleSystem& particleSystem)
	{
		shader->On();

		gl::Enable(gl::kDepthTest);
		gl::DepthMask(false); //Need to disable depth writing, because then the particles behind it will be wrong
		gl::Enable(gl::kBlend);
		if (type == RenderType::ADDITIVE) 
			gl::BlendFunc(gl::kSrcAlpha, gl::kOne); //Additive blending	
		else if (type == RenderType::ALPHA_BLENDED) {
			gl::BlendFunc(gl::kSrcAlpha, gl::kOneMinusSrcAlpha); //Alpha blend
			particleSystem.Sort (state);
		}
	}

	void endDraw ()
	{
		shader->Off();
		gl::DepthMask(true);
		gl::Disable(gl::kBlend);
	}
};

} // NS Particle
} // NS Engine