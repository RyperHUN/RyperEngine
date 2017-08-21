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
		explicit ivec2XZ(ivec2 const& vec)
			:x(vec.x), z(vec.y)
		{}
		ivec2XZ(ivec3 const& vec)
			:x(vec.x), z(vec.z)
		{}
		bool operator<(glm::ivec2XZ const& rhs) const
		{
			if(x < rhs.x)
				return true;
			if(x > rhs.x)
				return false;

			if (z < rhs.z)
				return true;
			if (z > rhs.z)
				return false;

			return false;
		}
	};

	static inline std::ostream& operator<<(std::ostream& os, glm::ivec3 const& vec)
	{
		os << "x:" << vec.x << ", y:" << vec.y << ", z:" << vec.z;
		return os;
	}
}