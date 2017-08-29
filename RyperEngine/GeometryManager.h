#pragma once

#include "Geometry.h"
#include "GeometryCreator.h"
#include "WrapperStructs.h"

namespace Geom {

namespace Primitive {
	struct Box : public PrimitiveGeom {
		virtual void Load() override
		{
			CreateBoxGeom (this->buffer);
		}
	};

	struct Quad : public PrimitiveGeom {
		virtual void Load() override 
		{
			CreateQuadGeom (this->buffer);
		}
	};

	struct CoordAxes : public PrimitiveGeom {
		virtual void Load() override
		{
			CreateCoordAxes (this->buffer);
		}
	}; 

} // NS Primitive

namespace Spline {
	class ISpline
	{
	public:
		virtual glm::vec3 Evaluate(float t) = 0;
	};

	class CatmullRom : public ISpline {
		std::vector<glm::vec3>cps;// control points
		std::vector<float> ts; // Stores non uniform values
		std::vector<glm::vec3> velocity; //first and last speed is special
		float firstTime;

		using vec4 = glm::vec4;
		using vec3 = glm::vec3;

		static vec3 HermiteInterpolation(vec3 p0, vec3 v0, float t0, vec3 p1, vec3 v1, float t1, float t)
		{
			vec3 a0 = p0;
			vec3 a1 = v0;
			vec3 a2 = ((p1 - p0) * 3.0f / (float)powf((t1 - t0), 2)) - ((v1 + v0 * 2.0f) / (float)(t1 - t0));
			vec3 a3 = ((p0 - p1) * 2.0f / (float)powf((t1 - t0), 3)) + ((v1 + v0) / (float)powf((t1 - t0), 2));
			return a3 * powf((t - t0), 3) + a2 * powf((t - t0), 2) + a1 * (t - t0) + a0;
		}
	public:
		void AddControlPoint(glm::vec3 cp, float timeFromStart) {
			if (cps.size() == 0)
				firstTime = timeFromStart;


			cps.push_back(cp);
			ts.push_back(timeFromStart - firstTime);
			if (cps.size() >= 2)
			{
				static const float tenzio = 0.5;
				velocity.resize(cps.size());
				velocity.front() = { 0,0,0 };
				velocity.back() = { 0,0,0 }; //TODO Starting points for starting velocities
				int maxIndex = velocity.size() - 1;
				for (int i = 1; i < maxIndex; i++) // Csak kozepsonek szamol sebességet
				{
					vec3 ujseb;
					ujseb = ((cps[i] - cps[i - 1]) / (ts[i] - ts[i - 1]) + (cps[i + 1] - cps[i]) / (ts[i + 1] - ts[i])) * ((1.0f - tenzio) / 2.0f);
					velocity[i] = ujseb;
				}

			}
		}
		glm::vec3 Evaluate(float t) override
		{
			for (int i = 0; i < cps.size() - 1; i++) {
				// Ekkor vagyok 2 kontrollpont között
				if (ts[i] <= t && t <= ts[i + 1])
				{
					return HermiteInterpolation(cps[i], velocity[i], ts[i],
						cps[i + 1], velocity[i + 1], ts[i + 1], t);
				}
			}
			MAssert(false, "invalid t parameter");
			throw "invalid";
			//Looping catmull
			//if (ts[maxIndex] <= t && t <= lastts)
			//{
			//	return HermiteInterpolation(cps[maxIndex], seb[maxIndex], ts[maxIndex],
			//		lastcps, lastseb, lastts, t);
			//}
		}
	};
} //NS Splines

class LineStrip {
	GLuint vao, vbo;        // vertex array object, vertex buffer object
							//float  vertexData[100]; // interleaved data of coordinates and colors
							//int    nVertices;       // number of vertices
	bool isLoaded = false;
	Shader::BoundingBox *shader;
	std::vector<glm::vec3> vertices; // Csúcsok

public:
	LineStrip()
	{
		Init();
	}
	LineStrip(std::vector<glm::vec3> const& points)
		: vertices(points)
	{
		Init();
	}
	void Create()
	{
		glGenVertexArrays(1, &vao);

		glBindVertexArray(vao);

		glGenBuffers(1, &vbo); // Generate 1 vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		// Enable the vertex attribute arrays
		glEnableVertexAttribArray(0);  // attribute array 0
									   // Map attribute array 0 to the vertex data of the interleaved vbo
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); // attribute array, components/attribute, component type, normalize?, stride, offset
		glBindVertexArray(0);
	}

	void AddPoint(glm::vec3 point)
	{
		vertices.push_back(point);
		isLoaded = false;
	}

	void ClearPoints()
	{
		vertices.clear();
	}
	void Draw(RenderState& state)
	{
		if (vertices.size() == 0)
			return;
		if (!isLoaded)
			CopyPointsToGPU(); //TODO Different thread

		shader->On();
		shader->SetUniform("PVM", state.PV);
		{
			glBindVertexArray(vao);
			glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
		}
		shader->Off();
	}
private:
	void Init()
	{
		shader = Shader::ShaderManager::GetShader<Shader::BoundingBox>();
		Create();
	}
	void CopyPointsToGPU()
	{
		isLoaded = true;
		// copy data to the GPU
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
		glBindVertexArray(0);
	}
};

class GeometryManager : public Ryper::Singleton
{
private:
	std::vector<PrimitiveGeom*> geometries; //std::get with an std::tuple is another alternative
	GeometryManager()
	{
		geometries.push_back (new Primitive::Box);
		geometries.push_back (new Primitive::Quad);
		geometries.push_back (new Primitive::CoordAxes);
		for (PrimitiveGeom* geom : geometries)
			geom->Load ();
	}
	static std::unique_ptr<GeometryManager> instance;
	static std::once_flag onceFlag; //wraps a callable object and ensure it is called only once. 
									//Even if multiple threads try to call it at the same time
public:
	static GeometryManager& Instance() //TODO Refactor instance to getShader
	{
		std::call_once(GeometryManager::onceFlag, []() {
			instance.reset(new GeometryManager);
		});

		return *(instance.get());
	}

	template <typename T>
	static T* GetGeometry()
	{
		static_assert(std::is_base_of<Geometry, T>::value, "T should inherit from gShaderProgram");
		GeometryManager& manager = Instance();

		for (Geometry* shader : manager.geometries)
		{
			T* casted = dynamic_cast<T*>(shader);
			if (casted != nullptr)
				return casted;
		}

		SAssert(false, "Shader is not found in shader manager");
		throw "Geometry not found";
	}

	void Clean()
	{
		for (PrimitiveGeom* geom : geometries)
		{
			geom->buffer.Clean ();
			delete geom;
		}
	}
};

} //NS Geom