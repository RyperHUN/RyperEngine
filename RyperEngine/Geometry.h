#pragma once

#include "gVertexBuffer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Mesh_OGL3.h"
#include "Bezier.h"
#include "Material.h"

#include "glmIncluder.h"
#include "Defs.h"
#include "UtilEngine.h"
#include "HeightMapGenerators.h"

enum LOCATION {
	POSITION = 0,
	NORMAL = 1,
	UV = 2,
	WEIGHT = 3,
	BONEID = 4,
	TANGENT = 5,
	BITANGENT = 6,
};

struct MeshVertexData {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent; //== Weight//TODO now weights and bone id-s are stored in this two too
					   // bitangent
	glm::vec3 Bitangent; //== IDs
};

struct AnimatedVertexData {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec4 Weights; //== Weight//TODO now weights and bone id-s are stored in this two too
					   // bitangent
	glm::vec4 IDs;
};

namespace Geom{
	struct Box
	{
		glm::vec3 max;
		glm::vec3 min;
		Box(){}
		Box(glm::vec3 max, glm::vec3 min)
			:max(max), min(min)
		{}
		glm::vec3 getVertex(int n)
		{
			assert(0 <= n && n < 8);
			switch(n)
			{
			case 0: return min;
			case 1: return max;
			case 2: return glm::vec3(min.x, max.y, min.z);
			case 3: return glm::vec3(max.x, max.y, min.z);
			case 4: return glm::vec3(max.x, min.y, min.z);
			case 5: return glm::vec3(min.x, min.y, max.z);
			case 6: return glm::vec3(min.x, max.y, max.z);
			case 7: return glm::vec3(max.x, min.y, max.z);
			}

			assert(false); //Cannot reach this
			return min;
		}
		glm::vec3 GetCenter ()
		{
			return (max + min) / 2.0f;
		}
		float GetWidth  (){return (max.x - min.x);}
		float GetHeight () { return (max.y - min.y); }
		float GetDepth  () { return (max.z - min.z); }
		static bool IsCollidingAABB(Geom::Box &lhs, Geom::Box &rhs)
		{
			return (lhs.min.x <= rhs.max.x && lhs.max.x >= rhs.min.x) &&
				(lhs.min.y <= rhs.max.y && lhs.max.y >= rhs.min.y) &&
				(lhs.min.z <= rhs.max.z && lhs.max.z >= rhs.min.z);
		}
		static Box CreateBoxFromVec(std::vector<glm::vec3> positions)
		{
			glm::vec3 max = positions[0];
			glm::vec3 min = positions[0];
			for (glm::vec3& pos : positions)
			{
				if (pos.x > max.x) max.x = pos.x;
				if (pos.y > max.y) max.y = pos.y;
				if (pos.z > max.z) max.z = pos.z;

				if (pos.x < min.x) min.x = pos.x;
				if (pos.y < min.y) min.y = pos.y;
				if (pos.z < min.z) min.z = pos.z;
			}
			Geom::Box box {min, max};
			return box;
		}
	};
	struct Plane
	{
		glm::vec3 a, b, c;
		glm::vec3 normal;
		void set3Points(glm::vec3 a, glm::vec3 b, glm::vec3 c)
		{
			this->a = a;
			this->b = b;
			this->c = c;
			normal = glm::normalize(glm::cross(b - a, c - b));
		}
		float distance(glm::vec3 p)
		{
			glm::vec3 toPoint = p - a;
			return glm::dot(normal, toPoint);
		}
	};
}; //NS Geom

struct Geometry {
	gVertexBuffer buffer;
	Geometry(){}
	virtual ~Geometry(){}

	//TODO Idiot shader program is only here for AssimpModel
	virtual void Draw(gShaderProgram * shader = nullptr) {
		buffer.On();
		buffer.Draw();
		buffer.Off();
	}
	Geom::Box cacheBox;
	bool isCalculatedBox = false;
	virtual Geom::Box getLocalAABB()
	{
		if(isCalculatedBox)
			return cacheBox;
		std::vector<glm::vec3> positions = buffer.GetPositionData();
		
		isCalculatedBox = true;
		cacheBox = Geom::Box::CreateBoxFromVec (positions);

		return cacheBox;
	}
	Geom::Box getModelBox(glm::vec3 translate, glm::vec3 scale, glm::quat quaternion)
	{
		Geom::Box localBox = getLocalAABB ();
		glm::vec4 min = glm::vec4{ localBox.min, 1.0};
		glm::vec4 max = glm::vec4{ localBox.max, 1.0 };
		glm::mat4 transform = glm::translate(translate) * glm::toMat4(quaternion) * glm::scale(scale);
		min = transform * min;
		max = transform * max;

		Geom::Box modelBox {min, max};
		return modelBox;
	}
	glm::mat4 getModelMatrixForBoxGeom(glm::vec3 translate, glm::vec3 scale, glm::quat quaternion)
	{
		glm::mat4 Model = glm::translate(translate) * glm::toMat4(quaternion) * glm::scale(scale);

		return Model * getLocalMatrixForBoxGeom();
	}
	glm::mat4 getLocalMatrixForBoxGeom() //You have to multiply it with Model matrix to get exact position
	{
		Geom::Box box = getLocalAABB();
		glm::vec3 localCenterPos = box.GetCenter ();
		glm::vec3 localScale = box.max - localCenterPos;

		return glm::translate(localCenterPos) * glm::scale(localScale);
	}
protected:
	Geometry(gVertexBuffer &&buffer)
		:buffer(buffer) 
	{}
};

struct VertexData {
	glm::vec3 position, normal;
	glm::vec2 uv;
};

struct ParamSurface : public Geometry {
	virtual VertexData GenVertexData(float u, float v) = 0;
	void Create(int N, int M)
	{
		buffer.AddAttribute(0,3);  //Vertex Pos
		buffer.AddAttribute(1, 3); //Normal
		buffer.AddAttribute(2, 2); //UV

		for (int i = 0; i <= N; ++i)
			for (int j = 0; j <= M; ++j)
			{
				float u = i / (float)N;
				float v = j / (float)M;

				VertexData data = GenVertexData(u, v);
				buffer.AddData(0, data.position);
				buffer.AddData(1, glm::normalize(data.normal));
				buffer.AddData(2, data.uv);
			}

		// indexpuffer adatai: NxM négyszög = 2xNxM háromszög = háromszöglista esetén 3x2xNxM index
		for (int i = 0; i<N; ++i)
			for (int j = 0; j<M; ++j)
			{
				// minden négyszögre csináljunk kettõ háromszöget, amelyek a következõ 
				// (i,j) indexeknél született (u_i, v_i) paraméterértékekhez tartozó
				// pontokat kötik össze:
				//
				//		(i,j+1)
				//		  o-----o(i+1,j+1)
				//		  |\    |			a = p(u_i, v_i)
				//		  | \   |			b = p(u_{i+1}, v_i)
				//		  |  \  |			c = p(u_i, v_{i+1})
				//		  |   \ |			d = p(u_{i+1}, v_{i+1})
				//		  |	   \|
				//	(i,j) o-----o(i+1, j)
				//
				// - az (i,j)-hez tartózó 1D-s index a VBO-ban: i+j*(N+1)
				// - az (i,j)-hez tartózó 1D-s index az IB-ben: i*6+j*6*(N+1) 
				//		(mert minden négyszöghöz 2db háromszög = 6 index tartozik)
				//
				buffer.AddIndex(   (i)+(j)*	(N + 1) );
				buffer.AddIndex( (i + 1) + (j)*	(N + 1));
				buffer.AddIndex( (i)+(j + 1)*(N + 1));
				buffer.AddIndex( (i + 1) + (j)*	(N + 1));
				buffer.AddIndex( (i + 1) + (j + 1)*(N + 1));
				buffer.AddIndex( (i)+(j + 1)*(N + 1));
			}

		buffer.InitBuffers ();
	}
};

struct HeightMapPerlin : public ParamSurface
{
private:
	Util::PerlinGenerator perlinGenerator;
public:
	HeightMapPerlin(Vec2 noiseTopLeft, Vec2 noiseBottomRight)
		:perlinGenerator(noiseTopLeft, noiseBottomRight, 10)
	{}
	virtual VertexData GenVertexData(float u, float v) override
	{
		VertexData data;
		const Vec2 UV = Vec2{ u,v };
		float height = perlinGenerator.GetValueUV(UV).height;
		const Vec2 ndc2 = Util::CV::UVToNdc(UV);

		const Vec3 FinalPos = Vec3(ndc2, height);
		static float dt = 0.01;
		Vec2 UVu = UV + Vec2(dt, 0);
		Vec2 UVv = UV + Vec2(0, dt);
		float heightU = perlinGenerator.GetValueUV(UVu).height;
		float heightV = perlinGenerator.GetValueUV(UVv).height;
		Vec3 DerivatedU = Vec3(UVu, heightU);
		Vec3 DerivatedV = Vec3(UVv, heightV);

		data.position = FinalPos;
		data.normal = glm::normalize(glm::cross(DerivatedU, DerivatedV));;
		//TODO data.normal;
		data.uv = UV;
		return data;
	}
	virtual void Draw(gShaderProgram * shader = nullptr) override
	{
		MAssert(shader, "nullptr given to shader in geometry");
		shader->SetTexture("texture_diffuse", 10, perlinGenerator.GetTexture()); //TODO better solution

		ParamSurface::Draw (shader);
	}
};

struct HeightMapIsland : public ParamSurface
{
	Util::IslandGenerator islandGenerator;
	HeightMapIsland ()
		:islandGenerator (10)
	{}
	virtual VertexData GenVertexData(float u, float v) override
	{
		VertexData data;
		const Vec2 UV   = Vec2{ u,v };
		float height    = islandGenerator.GetValueUV (UV);
		const Vec2 ndc2 = Util::CV::UVToNdc (UV);

		const Vec3 FinalPos = Vec3(ndc2, height);
		static float dt = 0.01;
		Vec2 UVu = UV + Vec2(dt, 0);
		Vec2 UVv = UV + Vec2(0, dt);
		float heightU = islandGenerator.GetValueUV(UVu);
		float heightV = islandGenerator.GetValueUV(UVv);
		Vec3 DerivatedU = Vec3(UVu, heightU);
		Vec3 DerivatedV = Vec3(UVv, heightV);

		data.position = FinalPos;
		data.normal   = glm::normalize(glm::cross(DerivatedU, DerivatedV));;
		//TODO data.normal;
		data.uv = UV;
		return data;
	}
};

struct HeightMapBezier : public ParamSurface
{
	BezierSurface bezier; 
	BezierSurface bezierU;
	BezierSurface bezierV;
	HeightMapBezier()
	{
		bezier  = BezierSurface::GenRandomSurface(6, 6);
		bezierU = bezier.DerivativeByU ();
		bezierV = bezier.DerivativeByV ();
	}
	virtual VertexData GenVertexData(float u, float v) override
	{
		VertexData data;
		data.position = bezier.Evaulate(u, v);
		data.normal   = glm::normalize(glm::cross(bezierU.Evaulate(u, v), bezierV.Evaulate(u, v)));
		data.uv       = glm::ivec2 (u,v);
		return data;
	}
};

struct Sphere : public ParamSurface
{
	float radius;
	Sphere () {}
	Sphere (float radius)
		: radius(radius)
	{}
	VertexData GenVertexData(float u, float v) override
	{
		VertexData vd;
		vd.normal = glm::vec3(cosf(u * 2 * M_PI) * sin(v*M_PI),
			sin(u * 2 * M_PI) * sin(v*M_PI),
			cos(v*M_PI));
		vd.position = vd.normal * radius;
		vd.uv = glm::vec2(u,v);
		return vd;
	}
};

struct TriangleMesh : public Geometry
{
	TriangleMesh (gVertexBuffer buffer)
		:Geometry (std::move(buffer))
	{
	}
	TriangleMesh (){}
	void Create()
	{
		buffer.InitBuffers ();
	}
};

///TODO OGL_mesh helyett sajat vertexBuffer hasznalata ittis.
struct TriangleMeshLoaded : public Geometry
{
	OGL_Mesh * mesh;
	TriangleMeshLoaded(OGL_Mesh *mesh = nullptr)
		:mesh(mesh)
	{
	}
	virtual void Draw(gShaderProgram * shader = nullptr)  override
	{
		if(mesh)
			mesh->draw ();
	}
};

class BufferedMesh : public Geometry
{
	MaterialPtr material;
public:
	std::vector<Texture> textures;
	BufferedMesh(MaterialPtr material)
		:material(material)
	{
		buffer.AddAttribute(0, 3); //Pos
		buffer.AddAttribute(1, 3); //Norm
		buffer.AddAttribute(2, 2); //UV
		buffer.AddAttribute(3, 4); //Weight
		buffer.AddAttribute(4, 4); //Bone id
		buffer.AddAttribute(5, 3); //Tangeng
		buffer.AddAttribute(6, 3); //Bitangent
	}
	void Init() { buffer.InitBuffers(); }
	template<typename T>
	void AddAttribute(LOCATION loc, std::vector<T>& attribute)
	{
		for (int i = 0; i < attribute.size(); i++)
			buffer.AddData((int)loc, attribute[i]);
	}
	template<typename T>
	void AddAttribute(LOCATION loc, T & attribute)
	{
		buffer.AddData((int)loc, attribute);
	}
	void AddAttributes(std::vector<MeshVertexData> & vertices)
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			MeshVertexData& vertex = vertices[i];
			AddAttribute(POSITION, vertex.Position);
			AddAttribute(NORMAL, vertex.Normal);
			AddAttribute(UV, vertex.TexCoords);
			AddAttribute(TANGENT, vertex.Tangent);
			AddAttribute(TANGENT, vertex.Bitangent);
		}
	}
	void AddIndices(std::vector<unsigned int> const& indices)
	{
		for (int i = 0; i < indices.size(); i++)
			buffer.AddIndex(indices[i]);
	}
	void Draw(gShaderProgram *shader)
	{
		shader->On();
		material->uploadToGpu(*shader);

		// draw mesh
		buffer.On();
		buffer.Draw();
		buffer.Off();

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
		shader->Off();
	}
};