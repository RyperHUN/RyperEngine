#include "GeometryManager.h"
namespace Geom {
	std::unique_ptr<GeometryManager> GeometryManager::instance;
	std::once_flag GeometryManager::onceFlag;
}; //NS Geom