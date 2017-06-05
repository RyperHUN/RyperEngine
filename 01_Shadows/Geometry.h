#pragma once

#include "gVertexBuffer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Mesh_OGL3.h"
#include "Bezier.h"

struct Geometry {
	gVertexBuffer buffer;
	Geometry(){}

	virtual void Draw() {
		buffer.On();
		buffer.DrawIndexed(GL_TRIANGLES);
		buffer.Off();
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

struct TriangleMeshLoaded : public Geometry
{
	OGL_Mesh * mesh;
	TriangleMeshLoaded(OGL_Mesh *mesh = nullptr)
		:mesh(mesh)
	{
	}
	virtual void Draw()  override
	{
		if(mesh)
			mesh->draw ();
	}
};