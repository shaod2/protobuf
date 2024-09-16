// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_COMPILER_KOTLIN_GENERATOR_FACTORY_H__
#define GOOGLE_PROTOBUF_COMPILER_KOTLIN_GENERATOR_FACTORY_H__

#include <memory>

#include "absl/container/btree_map.h"
#include "google/protobuf/compiler/java/context.h"
#include "google/protobuf/descriptor.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace kotlin {

class MessageGenerator {
 public:
  explicit MessageGenerator(const Descriptor* descriptor);
  MessageGenerator(const MessageGenerator&) = delete;
  MessageGenerator& operator=(const MessageGenerator&) = delete;
  virtual ~MessageGenerator() = default;

  virtual void Generate(io::Printer* printer) const = 0;
  virtual void GenerateMembers(io::Printer* printer) const = 0;
  virtual void GenerateTopLevelMembers(io::Printer* printer) const = 0;

 protected:
  const Descriptor* descriptor_;
  absl::btree_map<int, const OneofDescriptor*> oneofs_;
};

class GeneratorFactory {
 public:
  virtual ~GeneratorFactory() = default;

  virtual std::unique_ptr<MessageGenerator> NewMessageGenerator(
      const Descriptor* descriptor) const = 0;
};

}  // namespace kotlin
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_KOTLIN_GENERATOR_FACTORY_H__
