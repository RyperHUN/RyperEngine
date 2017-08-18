#pragma once

#include <memory>

class Camera;
using CameraPtr = std::shared_ptr<Camera>;
class FPSCamera;
using FPSCameraPtr = std::shared_ptr<FPSCamera>;

struct Material;
using MaterialPtr = std::shared_ptr<Material>;

struct Widget;
using WidgetPtr = std::shared_ptr<Widget>;

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#include <glm/glm.hpp>
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

#include <array>
template <class T, size_t ROW>
using Array1D = std::array<T, ROW>;

template <class T, size_t ROW, size_t COL>
using Array2D = std::array<std::array<T, COL>, ROW>;

template <class T, size_t ROW, size_t COL, size_t HEIGHT>
using Array3D = std::array<std::array<std::array<T,HEIGHT>, COL>, ROW>;


#ifndef NDEBUG
#   define MAssert(Expr, Msg) \
    __M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define MAssert(Expr, Msg) ;
#endif

#include <iostream>

static inline void __M_Assert(char* expr_str, bool expr, char* file, int line, char* msg)
{
	if (!expr)
	{
		std::cerr << "Assert failed:\t" << msg << "\n"
			<< "Expected:\t" << expr_str << "\n"
			<< "Source:\t\t" << file << ", line " << line << "\n";
		abort();
	}
}

namespace Ryper {
	struct NonCopyable 
	{
		NonCopyable () = default;
		NonCopyable(NonCopyable const&)  = delete;
		NonCopyable& operator=(NonCopyable const&) = delete;
	};
	struct NonMoveable 
	{
		NonMoveable () = default;
		NonMoveable (NonMoveable &&) = delete;
		NonMoveable& operator=(NonMoveable &&) = delete;
	};
} //NS Ryper
