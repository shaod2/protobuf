// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_COMPILER_KOTLIN_FULL_GENERATOR_FACTORY_H__
#define GOOGLE_PROTOBUF_COMPILER_KOTLIN_FULL_GENERATOR_FACTORY_H__

#include <memory>

#include "google/protobuf/compiler/java/context.h"
#include "google/protobuf/compiler/kotlin/generator_factory.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace kotlin {

std::unique_ptr<GeneratorFactory> MakeFullGeneratorFactory(
    java::Context* context);

}  // namespace kotlin
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_KOTLIN_FULL_GENERATOR_FACTORY_H__
