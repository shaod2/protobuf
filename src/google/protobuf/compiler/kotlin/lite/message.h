// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_COMPILER_KOTLIN_LITE_MESSAGE_H__
#define GOOGLE_PROTOBUF_COMPILER_KOTLIN_LITE_MESSAGE_H__

#include "google/protobuf/compiler/java/context.h"
#include "google/protobuf/compiler/java/generator_common.h"
#include "google/protobuf/compiler/java/lite/field_generator.h"
#include "google/protobuf/compiler/java/name_resolver.h"
#include "google/protobuf/compiler/kotlin/generator_factory.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace kotlin {

using ::google::protobuf::compiler::java::ClassNameResolver;
using ::google::protobuf::compiler::java::Context;

class LiteMessageGenerator : public MessageGenerator {
 public:
  LiteMessageGenerator(const Descriptor* descriptor, Context* context);
  LiteMessageGenerator(const LiteMessageGenerator&) = delete;
  LiteMessageGenerator& operator=(const LiteMessageGenerator&) = delete;
  ~LiteMessageGenerator() override = default;

  void Generate(io::Printer* printer) const override;
  void GenerateMembers(io::Printer* printer) const override;
  void GenerateTopLevelMembers(io::Printer* printer) const override;

 private:
  void GenerateExtensions(io::Printer* printer) const;
  void GenerateOrNull(io::Printer* printer) const;

  Context* context_;
  ClassNameResolver* name_resolver_;
  java::FieldGeneratorMap<java::ImmutableFieldLiteGenerator> field_generators_;
};

}  // namespace kotlin
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_KOTLIN_LITE_MESSAGE_H__
