#pragma once

#include "Geometry.h"
#include "GeometryCreator.h"

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
		ShaderManager& manager = Instance();

		for (Geometry* shader : manager.shaders)
		{
			T* casted = dynamic_cast<T*>(shader);
			if (casted != nullptr)
				return casted;
		}

		SAssert(false, "Shader is not found in shader manager");
		throw "Shader not found";
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