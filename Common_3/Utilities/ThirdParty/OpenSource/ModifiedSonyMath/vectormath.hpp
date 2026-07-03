
// ================================================================================================
// -*- C++ -*-
// File: vectormath/vectormath.hpp
// Author: Guilherme R. Lampert
// Created on: 30/12/16
// Brief: This header exposes the Sony Vectormath library types and functions into the global scope.
// ================================================================================================

#ifndef VECTORMATH_HPP
#define VECTORMATH_HPP

#include "../../../../Application/Config.h"
#include "vectormath_settings.hpp"

// Sony's library includes:
#if VECTORMATH_MODE_SCE
#if defined(ORBIS)
#include "../../../../../PS4/Common_3/Utilities/ThirdParty/OpenSource/vectormath/VectorMathConfig.h"
#include "../../../../../PS4/Common_3/Utilities/ThirdParty/OpenSource/vectormath/cpp/vectormath_aos.h"
#include "../../../../../PS4/Common_3/Utilities/ThirdParty/OpenSource/vectormath/cpp/vectormath_namespace.h"
#elif defined(PROSPERO)
#include "../../../../../Prospero/Common_3/Utilities/ThirdParty/OpenSource/vectormath/VectorMathConfig.h"
#include "../../../../../Prospero/Common_3/Utilities/ThirdParty/OpenSource/vectormath/cpp/vectormath_aos.h"
#include "../../../../../Prospero/Common_3/Utilities/ThirdParty/OpenSource/vectormath/cpp/vectormath_namespace.h"
#endif
#elif (VECTORMATH_CPU_HAS_SSE1_OR_BETTER && !VECTORMATH_FORCE_SCALAR_MODE) // SSE
    #include "sse/vectormath.hpp"
#ifndef BLOOM_NO_GLOBAL_VECTORMATH
    using namespace Vectormath::SSE;
#endif
    namespace VectormathForge { using namespace Vectormath::SSE; }
#elif (VECTORMATH_CPU_HAS_NEON && !VECTORMATH_FORCE_SCALAR_MODE) // NEON
	#include "neon/vectormath.hpp"
#ifndef BLOOM_NO_GLOBAL_VECTORMATH
	using namespace Vectormath::Neon;
#endif
	namespace VectormathForge { using namespace Vectormath::Neon; }
#else // !SSE
    #include "scalar/vectormath.hpp"
#ifndef BLOOM_NO_GLOBAL_VECTORMATH
    using namespace Vectormath::Scalar;
#endif
    namespace VectormathForge { using namespace Vectormath::Scalar; }
#endif // Vectormath mode selection

#ifdef BLOOM_NO_GLOBAL_VECTORMATH
// Bloom: with the global hoist suppressed, this library's own headers below (soa/*.hpp,
// vec2d.hpp, common.hpp) still spell bare Vector3/Vector4/Quat/FloatInVec/... inside
// namespace Vectormath{::Soa}. These using-declarations lift the mode namespace's TYPES
// (resolved mode-independently through VectormathForge — qualified lookup follows its
// using-directive) into the parent Vectormath so those declarations keep parsing. TYPES
// ONLY, deliberately: a blanket `using namespace` here would also import the mode
// namespace's math FUNCTIONS, and unqualified lookup for e.g. common.hpp's sqrt(float)
// would then stop at Vectormath with only the FloatInVec/DoubleInVec overloads (explicit
// ctors, no float conversion) instead of walking out to the CRT's ::sqrt. Nothing here
// leaks to the global namespace.
namespace Vectormath
{
    using ::VectormathForge::Vector3;
    using ::VectormathForge::Vector4;
    using ::VectormathForge::Vector4Int;
    using ::VectormathForge::Point3;
    using ::VectormathForge::Quat;
    using ::VectormathForge::Matrix3;
    using ::VectormathForge::Matrix4;
    using ::VectormathForge::Transform3;
    using ::VectormathForge::FloatInVec;
    using ::VectormathForge::BoolInVec;
    using ::VectormathForge::IVector3;
    using ::VectormathForge::IVector4;
    using ::VectormathForge::UVector3;
    using ::VectormathForge::UVector4;
    using ::VectormathForge::Vector3d;
    using ::VectormathForge::Vector4d;
    using ::VectormathForge::Matrix3d;
    using ::VectormathForge::Matrix4d;
    // Two FUNCTION exceptions to the types-only rule: soa/float.hpp and soa/quaternion.hpp
    // call And/Or on Vector4Int (= __m128i, a global intrinsic type) — ADL associates only
    // the global namespace for it, so it can never reach the mode namespace. Neither name
    // collides with a CRT function.
    using ::VectormathForge::And;
    using ::VectormathForge::Or;
}
#endif // BLOOM_NO_GLOBAL_VECTORMATH

//========================================= #TheForgeMathExtensionsBegin ================================================
//========================================= #TheForgeAnimationMathExtensionsBegin =======================================
#include "soa/soa.hpp"
#ifndef BLOOM_NO_GLOBAL_VECTORMATH
using namespace Vectormath::Soa;
#endif
//========================================= #TheForgeAnimationMathExtensionsEnd =======================================
//========================================= #TheForgeMathExtensionsEnd ================================================

#include "vec2d.hpp"  // - Extended 2D vector and point classes; not aligned and always in scalar floats mode.
#include "common.hpp" // - Miscellaneous helper functions.

#ifndef BLOOM_NO_GLOBAL_VECTORMATH
using namespace Vectormath;
#else
// Bloom: the blanket hoist above is suppressed to keep the global Vector2/Vector3/Vector4
// names free for Bloom's native math. These targeted using-declarations re-export the
// NON-colliding names that Forge's public interface headers (IUI.h / IProfiler.h /
// IResourceLoader.h / RingBuffer.h) and Forge-consuming Bloom TUs (shader-mirror .srt.h
// structs) reference unqualified at global scope. Extend the list if a new unqualified
// use appears — never restore the blanket directive.
using Vectormath::half;
using Vectormath::float2;
using Vectormath::float3;
using Vectormath::float4;
using Vectormath::int2;
using Vectormath::int3;
using Vectormath::int4;
using Vectormath::uint;
using Vectormath::uint2;
using Vectormath::uint3;
using Vectormath::uint4;
using Vectormath::round_up;
using Vectormath::round_up_64;
using Vectormath::round_down;
using Vectormath::round_down_64;
#endif

// VectormathForge — a confined re-export of the Sony vector/matrix types (impl namespace
// SSE/Neon/Scalar plus the parent Vectormath's Vector2/Point2/IVector2/UVector2/Matrix2).
// When Bloom defines BLOOM_NO_GLOBAL_VECTORMATH the blanket global using-directives above
// are suppressed so Bloom's own global-scope Vector2/Vector3/Vector4 (Core/Math) don't
// collide (an unqualified OR ::-qualified Vector3 would otherwise be ambiguous). MathTypes.h
// still names the types through VectormathForge:: — qualified lookup follows the using-
// directives here without leaking the names into the global namespace. Always defined and
// harmless when the globals are on (it just re-finds the same types).
namespace VectormathForge { using namespace Vectormath; }

#endif // VECTORMATH_HPP
