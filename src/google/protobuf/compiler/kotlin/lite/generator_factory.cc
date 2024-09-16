// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/kotlin/generator_factory.h"

#include <memory>

#include "google/protobuf/compiler/java/context.h"
#include "google/protobuf/compiler/kotlin/lite/message.h"
#include "google/protobuf/descriptor.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace kotlin {

using ::google::protobuf::compiler::java::Context;

// Factory that creates generators for immutable-default messages.
class LiteGeneratorFactory : public GeneratorFactory {
 public:
  explicit LiteGeneratorFactory(Context* context) : context_(context) {}
  LiteGeneratorFactory(const LiteGeneratorFactory&) = delete;
  LiteGeneratorFactory& operator=(const LiteGeneratorFactory&) = delete;
  ~LiteGeneratorFactory() override = default;

  std::unique_ptr<MessageGenerator> NewMessageGenerator(
      const Descriptor* descriptor) const override {
    return std::make_unique<LiteMessageGenerator>(descriptor, context_);
  }

 private:
  Context* context_;
};

std::unique_ptr<GeneratorFactory> MakeLiteGeneratorFactory(Context* context) {
  return std::make_unique<LiteGeneratorFactory>(context);
}

}  // namespace kotlin
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
