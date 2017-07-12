#pragma once
#include <GL/glew.h> 
#include "glmIncluder.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <PxPhysicsAPI.h>

inline bool operator<(glm::vec3 const& lhs, glm::vec3 const&rhs)
{
	return lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z;
}

inline bool operator>(glm::vec3 const& lhs, glm::vec3 const&rhs)
{
	return lhs.x > rhs.x && lhs.y > rhs.y && lhs.z > rhs.z;
}

namespace Util {

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

/** Convert glm::vec3 to Physx::PxVec3
@param[in] vec3 The glm::vec3
@return Converted Physx::PxVec3 */
inline physx::PxVec3 glmVec3ToPhysXVec3(const glm::vec3& vec3)
{
	return physx::PxVec3(vec3.x, vec3.y, vec3.z);
}
/** Convert glm::quat to Physx::PxQuat
@param[in] quat The glm::quat
@return Converted Physx::PxQuat */
inline physx::PxQuat glmQuatToPhysXQuat(const glm::quat& quat)
{
	return physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
}
/** Convert Physx::PxVec3 to glm::vec3
@param[in] vec3 The Physx::PxVec3
@return Converted glm::vec3 */
inline glm::vec3 PhysXVec3ToglmVec3(const physx::PxVec3& vec3)
{
	return glm::vec3(vec3.x, vec3.y, vec3.z);
}
/** Convert Physx::PxExtendedVec3 to glm::vec3
@param[in] vec3 The Physx::PxVec3
@return Converted glm::vec3 */
inline glm::vec3 PhysXVec3ToglmVec3(const physx::PxExtendedVec3& vec3)
{
	return glm::vec3(vec3.x, vec3.y, vec3.z);
}
/** Convert Physx::PxQuat to glm::quat
@param[in] quat The Physx::PxQuat
@return Converted glm::quat */
inline glm::quat PhysXQuatToglmQuat(const physx::PxQuat& quat)
{
	return glm::quat(quat.w, quat.x, quat.y, quat.z);
}

///** Convert Transform (position and rotation) to Physx::PxTransform
//@param[in] transform The transform that will be converted
//@return Converted Physx::PxTransform */
//physx::PxTransform transformToPhysXTransform(const shared_ptr<physx::PxTransform>& transform)
//{
//	return physx::PxTransform(glmVec3ToPhysXVec3(transform->getWorldPosition()), glmQuatToPhysXQuat(transform->rotation));
//}

/** Convert glm::mat4 to PhysX::PxMat44
@param[in] mat4 The glm::mat4
@return Converted PhysX::PxMat44 */
inline physx::PxMat44 glmMat4ToPhysxMat4(const glm::mat4& mat4)
{
	physx::PxMat44 newMat;

	newMat[0][0] = mat4[0][0];
	newMat[0][1] = mat4[0][1];
	newMat[0][2] = mat4[0][2];
	newMat[0][3] = mat4[0][3];

	newMat[1][0] = mat4[1][0];
	newMat[1][1] = mat4[1][1];
	newMat[1][2] = mat4[1][2];
	newMat[1][3] = mat4[1][3];

	newMat[2][0] = mat4[2][0];
	newMat[2][1] = mat4[2][1];
	newMat[2][2] = mat4[2][2];
	newMat[2][3] = mat4[2][3];

	newMat[3][0] = mat4[3][0];
	newMat[3][1] = mat4[3][1];
	newMat[3][2] = mat4[3][2];
	newMat[3][3] = mat4[3][3];


	return newMat;
}

/** Convert PhysX::PxMat44 to glm::mat4
@param[in] mat4 The PhysX::PxMat44
@param[ou] Converted glm::mat4
*/
inline void PhysXMat4ToglmMat4(const physx::PxMat44& mat4, glm::mat4& newMat)
{

	newMat[0][0] = mat4[0][0];
	newMat[0][1] = mat4[0][1];
	newMat[0][2] = mat4[0][2];
	newMat[0][3] = mat4[0][3];

	newMat[1][0] = mat4[1][0];
	newMat[1][1] = mat4[1][1];
	newMat[1][2] = mat4[1][2];
	newMat[1][3] = mat4[1][3];

	newMat[2][0] = mat4[2][0];
	newMat[2][1] = mat4[2][1];
	newMat[2][2] = mat4[2][2];
	newMat[2][3] = mat4[2][3];

	newMat[3][0] = mat4[3][0];
	newMat[3][1] = mat4[3][1];
	newMat[3][2] = mat4[3][2];
	newMat[3][3] = mat4[3][3];

}

}// NS Util