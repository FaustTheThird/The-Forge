/*
 * Copyright (c) 2017-2025 The Forge Interactive Inc.
 *
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#pragma once

#ifndef THEFORGE_INCLUDE_MATHTYPES_H
#define THEFORGE_INCLUDE_MATHTYPES_H

#include "../../Application/Config.h"

// ModifiedSonyMath ReadMe:
// - All you need to do is include the public header file vectormath.hpp. It will expose the relevant parts of
//   the library for you and try to select the SSE implementation if supported.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-aliasing"
#endif
#include "../ThirdParty/OpenSource/ModifiedSonyMath/vectormath.hpp"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(__clang__)
#pragma clang diagnostic pop
#endif

// Qualified through VectormathForge:: (see vectormath.hpp) so these resolve even when Bloom
// suppresses the global Vectormath using-directives via BLOOM_NO_GLOBAL_VECTORMATH — the vec*/
// mat* aliases stay global, but the raw Vector2/3/4 names never leak to collide with Bloom's
// native math. Identical types either way.
typedef VectormathForge::Vector2 vec2;
typedef VectormathForge::Vector3 vec3;
typedef VectormathForge::Vector4 vec4;

typedef VectormathForge::IVector2 ivec2;
typedef VectormathForge::IVector3 ivec3;
typedef VectormathForge::IVector4 ivec4;

typedef VectormathForge::UVector2 uvec2;
typedef VectormathForge::UVector3 uvec3;
typedef VectormathForge::UVector4 uvec4;

typedef VectormathForge::Matrix2 mat2;
typedef VectormathForge::Matrix3 mat3;
typedef VectormathForge::Matrix4 mat4;

// Double-precision's inception was to fix facebook maps. Initial implementation was with scalars.
// Neon implementation was made to improve performance. Neon mirrors sse (see sse2neon.h) so sse exists as a bonus.
// Playstation uses its own SCE implementation hence this preprocessor.
// (Currently no need to support double-precision elsewhere then moble).
#if !VECTORMATH_MODE_SCE
typedef VectormathForge::Vector3d vec3d;
typedef VectormathForge::Vector4d vec4d;
typedef VectormathForge::Matrix3d mat3d;
typedef VectormathForge::Matrix4d mat4d;
#endif

#endif // THEFORGE_INCLUDE_MATHTYPES_H
