#pragma once

#include <memory>

class Camera;
using CameraPtr = std::shared_ptr<Camera>;
class FPSCamera;
using FPSCameraPtr = std::shared_ptr<FPSCamera>;

struct Material;
using MaterialPtr = std::shared_ptr<Material>;
