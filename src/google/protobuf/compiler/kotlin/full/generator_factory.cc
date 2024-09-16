// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/kotlin/generator_factory.h"

#include <memory>

#include "google/protobuf/compiler/java/context.h"
#include "google/protobuf/compiler/kotlin/full/message.h"
#include "google/protobuf/descriptor.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace kotlin {

using ::google::protobuf::compiler::java::Context;

// Factory that creates generators for immutable-default messages.
class FullGeneratorFactory : public GeneratorFactory {
 public:
  explicit FullGeneratorFactory(Context* context) : context_(context) {}
  FullGeneratorFactory(const FullGeneratorFactory&) = delete;
  FullGeneratorFactory& operator=(const FullGeneratorFactory&) = delete;
  ~FullGeneratorFactory() override = default;

  std::unique_ptr<MessageGenerator> NewMessageGenerator(
      const Descriptor* descriptor) const override {
    return std::make_unique<FullMessageGenerator>(descriptor, context_);
  }

 private:
  Context* context_;
};

std::unique_ptr<GeneratorFactory> MakeFullGeneratorFactory(Context* context) {
  return std::make_unique<FullGeneratorFactory>(context);
}

}  // namespace kotlin
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
