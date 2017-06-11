#pragma once

#include "gVertexBuffer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Mesh_OGL3.h"
#include "Bezier.h"

namespace Geom{
	struct Box
	{
		glm::vec3 max;
		glm::vec3 min;
	};
};

struct Geometry {
	gVertexBuffer buffer;
	Geometry(){}

	virtual void Draw(gShaderProgram * shader = nullptr) {
		buffer.On();
		buffer.Draw();
		buffer.Off();
	}
	Geom::Box getLocalAABB()
	{
		std::vector<glm::vec3> positions = buffer.GetPositionData();
		glm::vec3 max = positions[0];
		glm::vec3 min = positions[0];
		for(glm::vec3& pos : positions)
		{
			if(pos.x > max.x) max.x = pos.x;
			if(pos.y > max.y) max.y = pos.y;
			if(pos.z > max.z) max.z = pos.z;

			if(pos.x < min.x) min.x = pos.x;
			if(pos.y < min.y) min.y = pos.y;
			if(pos.z < min.z) min.z = pos.z;
		}
		return Geom::Box{max, min};
	}
	Geom::Box getModelBox(glm::vec3 translate, glm::vec3 scale)
	{
		Geom::Box localBox = getLocalAABB ();
		glm::vec4 min = glm::vec4{ localBox.min, 1.0};
		glm::vec4 max = glm::vec4{ localBox.max, 1.0 };
		glm::mat4 transform = glm::translate(translate) * glm::scale(scale);
		min = transform * min;
		max = transform * max;

		Geom::Box modelBox {min, max};
		return modelBox;
	}
	glm::mat4 getMatrixForBoxGeom(glm::vec3 translate, glm::vec3 scale)
	{
		Geom::Box box = getLocalAABB();
		glm::vec3 localCenterPos  = (box.max + box.min) / 2.0f;
		glm::vec3 localScale = box.max - localCenterPos; 

		return glm::translate(translate) * glm::scale(scale) * glm::translate(localCenterPos) * glm::scale(localScale);
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

struct HeightMap : public Geometry
{
	void Create(int N, int M)
	{
		BezierSurface bezier = BezierSurface::GenRandomSurface (6,6);
		BezierSurface bezierU = bezier.DerivativeByU ();
		BezierSurface bezierV = bezier.DerivativeByV ();
		buffer.AddAttribute(0, 3);  //Vertex Pos
		buffer.AddAttribute(1, 3); //Normal
		buffer.AddAttribute(2, 2); //UV

		for (int i = 0; i <= N; ++i)
			for (int j = 0; j <= M; ++j)
			{
				float u = i / (float)N;
				float v = j / (float)M;

				glm::vec3 pos = bezier.Evaulate (u,v);
				glm::vec3 normal = glm::normalize(glm::cross(bezierU.Evaulate(u,v), bezierV.Evaulate(u,v)));
				
				buffer.AddData(0, pos);
				buffer.AddData(1, normal);
				buffer.AddData(2, glm::vec2(u,v));
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
				buffer.AddIndex((i)+(j)*	(N + 1));
				buffer.AddIndex((i + 1) + (j)*	(N + 1));
				buffer.AddIndex((i)+(j + 1)*(N + 1));
				buffer.AddIndex((i + 1) + (j)*	(N + 1));
				buffer.AddIndex((i + 1) + (j + 1)*(N + 1));
				buffer.AddIndex((i)+(j + 1)*(N + 1));
			}

		buffer.InitBuffers();
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