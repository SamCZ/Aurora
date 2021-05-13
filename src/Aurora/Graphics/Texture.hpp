#pragma once

#include <Texture.h>
#include <RefCntAutoPtr.hpp>

using namespace Diligent;

namespace Aurora
{
	typedef Diligent::RefCntAutoPtr<Diligent::ITexture> Texture_ptr;
	#define Texture_ptr_null Texture_ptr(nullptr)
}