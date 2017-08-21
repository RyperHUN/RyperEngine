#pragma once

#include <glm/glm.hpp>

namespace glm 
{
	struct ivec2XZ
	{
		int x, z;
		ivec2XZ(ivec2XZ const&) = default;
		ivec2XZ(int x, int z)
			:x(x),z(z)
		{}
		ivec2XZ(vec2 const& vec)
			:x(vec.x), z(vec.y)
		{}
		ivec2XZ(vec3 const& vec)
			:x(vec.x), z(vec.z)
		{}
	};
}