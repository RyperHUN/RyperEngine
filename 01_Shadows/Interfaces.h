#pragma once

#include "gShaderProgram.h"
#include "WrapperStructs.h"
#include "Material.h"

struct Renderable {
	bool isInsideFrustum = true;
	virtual void Draw (RenderState const& state) 
	{
		if (!isInsideFrustum)
			return;

		MAssert(state.geom != nullptr, "Geometry was nullptr, aborting");
		
		gShaderProgram * shader = state.shader;
		shader->On();
		{
			glm::mat4 PVM = state.PV * state.M;
			shader->SetUniform("PVM", PVM);
			shader->SetUniform("M", state.M);
			shader->SetUniform("Minv", state.Minv);
			shader->SetUniform("wEye", state.wEye);
			shader->SetUniform("LightSpaceMtx", state.LightSpaceMtx);
			shader->SetUniform("isAnimated", false);

			///TODO
			for (auto& light : *(state.shaderLights))
				light.uploadToGPU(*shader);
			if(state.material)
				state.material->uploadToGpu(*shader);

			state.geom->Draw(shader);
		}
		shader->Off();
	}
};