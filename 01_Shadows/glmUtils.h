#pragma once
#include <GL/glew.h> 
#include "glmIncluder.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

inline bool operator<(glm::vec3 const& lhs, glm::vec3 const&rhs)
{
	return lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z;
}

inline bool operator>(glm::vec3 const& lhs, glm::vec3 const&rhs)
{
	return lhs.x > rhs.x && lhs.y > rhs.y && lhs.z > rhs.z;
}

inline glm::mat4 assimpToGlm(const aiMatrix4x4& from)
{
	//glm::mat4 m = glm::transpose(glm::make_mat4(&aiM.a1));
	glm::mat4 to;


	to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
	to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
	to[2][0] = (GLfloat)from.a3; to[2][1] = (GLfloat)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
	to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;

	return to;
}
inline glm::vec3 assimpToGlm(const aiVector3D& aiVec)
{
	glm::vec3 vec(aiVec.x, aiVec.y, aiVec.z);
	return vec;
}
inline glm::quat assimpToGlm(const aiQuaternion& aiQuat)
{
	glm::quat quaternion(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
	return quaternion;
}