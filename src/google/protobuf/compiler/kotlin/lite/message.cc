// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/kotlin/lite/message.h"

#include <string>

#include "absl/log/absl_check.h"
#include "google/protobuf/compiler/java/context.h"
#include "google/protobuf/compiler/java/doc_comment.h"
#include "google/protobuf/compiler/java/field_common.h"
#include "google/protobuf/compiler/java/helpers.h"
#include "google/protobuf/compiler/java/lite/make_field_gens.h"
#include "google/protobuf/compiler/kotlin/generator_factory.h"
#include "google/protobuf/io/printer.h"

// Must be last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace compiler {
namespace kotlin {

LiteMessageGenerator::LiteMessageGenerator(const Descriptor* descriptor,
                                           Context* context)
    : MessageGenerator(descriptor),
      context_(context),
      name_resolver_(context->GetNameResolver()),
      field_generators_(MakeImmutableFieldLiteGenerators(descriptor, context)) {
  ABSL_CHECK(
      !java::HasDescriptorMethods(descriptor->file(), context->EnforceLite()))
      << "Generator factory error: A lite message generator is used to "
         "generate non-lite messages.";
  for (int i = 0; i < descriptor_->field_count(); i++) {
    if (java::IsRealOneof(descriptor_->field(i))) {
      const OneofDescriptor* oneof = descriptor_->field(i)->containing_oneof();
      ABSL_CHECK(oneofs_.emplace(oneof->index(), oneof).first->second == oneof);
    }
  }
}

void LiteMessageGenerator::Generate(io::Printer* printer) const {
  printer->Print(
      "@kotlin.OptIn"
      "(com.google.protobuf.kotlin.OnlyForUseByGeneratedProtoCode::class)\n"
      "@com.google.protobuf.kotlin.ProtoDslMarker\n");
  printer->Print(
      "public class Dsl private constructor(\n"
      "  private val _builder: $message$.Builder\n"
      ") {\n"
      "  public companion object {\n"
      "$    jvm_synthetic$"
      "    @kotlin.PublishedApi\n"
      "    internal fun _create(builder: $message$.Builder): Dsl = "
      "Dsl(builder)\n"
      "  }\n"
      "\n"
      "$  jvm_synthetic$"
      "  @kotlin.PublishedApi\n"
      "  internal fun _build(): $message$ = _builder.build()\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      java::EscapeKotlinKeywords(
          name_resolver_->GetClassName(descriptor_, true)));

  printer->Indent();

  for (int i = 0; i < descriptor_->field_count(); i++) {
    printer->Print("\n");
    field_generators_.get(descriptor_->field(i))
        .GenerateKotlinDslMembers(printer);
  }

  for (auto& kv : oneofs_) {
    java::JvmNameContext name_ctx = {context_->options(), printer};
    const OneofDescriptor* oneof = kv.second;
    auto oneof_name = context_->GetOneofGeneratorInfo(oneof)->name;
    printer->Emit(
        {
            {"jvm_name",
             [&] { JvmName("get$oneof_capitalized_name$Case", name_ctx); }},
            {"oneof_name", oneof_name},
            {"oneof_capitalized_name",
             context_->GetOneofGeneratorInfo(oneof)->capitalized_name},
            {"oneof_property_name", java::GetKotlinPropertyName(oneof_name)},
            {"message", java::EscapeKotlinKeywords(
                            name_resolver_->GetClassName(descriptor_, true))},
        },
        "public val $oneof_name$Case: $message$.$oneof_capitalized_name$Case\n"
        "$jvm_name$"
        "  get() = _builder.$oneof_property_name$Case\n\n"
        "public fun clear$oneof_capitalized_name$() {\n"
        "  _builder.clear$oneof_capitalized_name$()\n"
        "}\n");
  }

  if (descriptor_->extension_range_count() > 0) {
    GenerateExtensions(printer);
  }

  printer->Outdent();
  printer->Print("}\n");
}

void LiteMessageGenerator::GenerateMembers(io::Printer* printer) const {
  if (context_->options().jvm_dsl) {
    printer->Print("@kotlin.jvm.JvmName(\"-initialize$camelcase_name$\")\n",
                   "camelcase_name",
                   name_resolver_->GetKotlinFactoryName(descriptor_));
  }

  printer->Print(
      "public inline fun $camelcase_name$(block: $message_kt$.Dsl.() -> "
      "kotlin.Unit): $message$ =\n"
      "  $message_kt$.Dsl._create($message$.newBuilder()).apply { block() "
      "}._build()\n",
      "camelcase_name", name_resolver_->GetKotlinFactoryName(descriptor_),
      "message_kt",
      java::EscapeKotlinKeywords(
          name_resolver_->GetKotlinExtensionsClassName(descriptor_)),
      "message",
      java::EscapeKotlinKeywords(
          name_resolver_->GetClassName(descriptor_, true)));

  WriteMessageDocComment(printer, descriptor_, context_->options(),
                         /* kdoc */ true);
  printer->Print("public object $name$Kt {\n", "name", descriptor_->name());
  printer->Indent();
  Generate(printer);
  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    if (java::IsMapEntry(descriptor_->nested_type(i))) continue;
    LiteMessageGenerator(descriptor_->nested_type(i), context_)
        .GenerateMembers(printer);
  }
  printer->Outdent();
  printer->Print("}\n");
}

void LiteMessageGenerator::GenerateTopLevelMembers(io::Printer* printer) const {
  printer->Print(
      "public inline fun $message$.copy(block: $message_kt$.Dsl.() -> "
      "kotlin.Unit): $message$ =\n"
      "  $message_kt$.Dsl._create(this.toBuilder()).apply { block() "
      "}._build()\n\n",
      "message",
      java::EscapeKotlinKeywords(
          name_resolver_->GetClassName(descriptor_, true)),
      "message_kt",
      name_resolver_->GetKotlinExtensionsClassNameEscaped(descriptor_));

  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    if (java::IsMapEntry(descriptor_->nested_type(i))) continue;
    LiteMessageGenerator(descriptor_->nested_type(i), context_)
        .GenerateTopLevelMembers(printer);
  }

  GenerateOrNull(printer);
}

void LiteMessageGenerator::GenerateOrNull(io::Printer* printer) const {
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (!field->has_presence() ||
        java::GetJavaType(field) != java::JAVATYPE_MESSAGE) {
      continue;
    }
    if (context_->options().jvm_dsl) {
      // On the JVM, we can use `FooOrBuilder`, and it saves code size to
      // generate only one method instead of two.
      if (field->options().deprecated()) {
        printer->Print(
            "@kotlin.Deprecated(message = \"Field $name$ is deprecated\")\n",
            "name", context_->GetFieldGeneratorInfo(field)->name);
      }
      printer->Print(
          "public val $full_classname$OrBuilder.$camelcase_name$OrNull: "
          "$full_name$?\n"
          "  get() = if (has$name$()) get$name$() else null\n\n",
          "full_classname",
          java::EscapeKotlinKeywords(
              name_resolver_->GetClassName(descriptor_, true)),
          "camelcase_name", context_->GetFieldGeneratorInfo(field)->name,
          "full_name",
          java::EscapeKotlinKeywords(
              name_resolver_->GetImmutableClassName(field->message_type())),
          "name", context_->GetFieldGeneratorInfo(field)->capitalized_name);
    } else {
      // Non-JVM platforms don't have `FooOrBuilder`, so we generate `Foo`
      // and `Foo.Builder` methods.
      if (field->options().deprecated()) {
        printer->Print(
            "@kotlin.Deprecated(message = \"Field $name$ is deprecated\")\n",
            "name", context_->GetFieldGeneratorInfo(field)->name);
      }
      printer->Print(
          "public val $full_classname$.$camelcase_name$OrNull: "
          "$full_name$?\n"
          "  get() = if (has$capitalized_name$()) this.$name$ else null\n\n",
          "full_classname",
          java::EscapeKotlinKeywords(
              name_resolver_->GetClassName(descriptor_, true)),
          "camelcase_name", context_->GetFieldGeneratorInfo(field)->name,
          "full_name",
          java::EscapeKotlinKeywords(
              name_resolver_->GetImmutableClassName(field->message_type())),
          "capitalized_name",
          context_->GetFieldGeneratorInfo(field)->capitalized_name, "name",
          java::EscapeKotlinKeywords(java::GetKotlinPropertyName(
              context_->GetFieldGeneratorInfo(field)->capitalized_name)));
      if (field->options().deprecated()) {
        printer->Print(
            "@kotlin.Deprecated(message = \"Field $name$ is deprecated\")\n",
            "name", context_->GetFieldGeneratorInfo(field)->name);
      }
      printer->Print(
          "public val $full_classname$.Builder.$camelcase_name$OrNull: "
          "$full_name$?\n"
          "  get() = if (has$capitalized_name$()) this.$name$ else null\n\n",
          "full_classname",
          java::EscapeKotlinKeywords(
              name_resolver_->GetClassName(descriptor_, true)),
          "camelcase_name", context_->GetFieldGeneratorInfo(field)->name,
          "full_name",
          java::EscapeKotlinKeywords(
              name_resolver_->GetImmutableClassName(field->message_type())),
          "capitalized_name",
          context_->GetFieldGeneratorInfo(field)->capitalized_name, "name",
          java::EscapeKotlinKeywords(java::GetKotlinPropertyName(
              context_->GetFieldGeneratorInfo(field)->capitalized_name)));
    }
  }
}

void LiteMessageGenerator::GenerateExtensions(io::Printer* printer) const {
  java::JvmNameContext name_ctx = {context_->options(), printer};
  std::string message_name = java::EscapeKotlinKeywords(
      name_resolver_->GetClassName(descriptor_, true));

  printer->Print(
      "@Suppress(\"UNCHECKED_CAST\")\n"
      "$jvm_synthetic$"
      "public operator fun <T : kotlin.Any> get(extension: "
      "com.google.protobuf.ExtensionLite<$message$, T>): T {\n"
      "  return if (extension.isRepeated) {\n"
      "    get(extension as com.google.protobuf.ExtensionLite<$message$, "
      "kotlin.collections.List<*>>) as T\n"
      "  } else {\n"
      "    _builder.getExtension(extension)\n"
      "  }\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  if (context_->options().jvm_dsl) {
    // TODO: generate this on Kotlin Native once the [Mutable]List
    // issue is resolved.
    printer->Emit(
        {
            {"jvm_name", [&] { JvmName("-getRepeatedExtension", name_ctx); }},
            {"jvm_synthetic", JvmSynthetic(context_->options())},
            {"message", message_name},
        },
        "$jvm_synthetic$"
        "@kotlin.OptIn"
        "(com.google.protobuf.kotlin.OnlyForUseByGeneratedProtoCode::class)\n"
        "$jvm_name$"
        "public operator fun <E : kotlin.Any> get(\n"
        "  extension: com.google.protobuf.ExtensionLite<$message$, "
        "kotlin.collections.List<E>>\n"
        "): com.google.protobuf.kotlin.ExtensionList<E, $message$> {\n"
        "  return com.google.protobuf.kotlin.ExtensionList(extension, "
        "_builder.getExtension(extension))\n"
        "}\n\n");
  }

  printer->Print(
      "$jvm_synthetic$"
      "public operator fun contains(extension: "
      "com.google.protobuf.ExtensionLite<$message$, *>): "
      "Boolean {\n"
      "  return _builder.hasExtension(extension)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "public fun clear(extension: "
      "com.google.protobuf.ExtensionLite<$message$, *>) "
      "{\n"
      "  _builder.clearExtension(extension)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "public fun <T : kotlin.Any> setExtension(extension: "
      "com.google.protobuf.ExtensionLite<$message$, T>, "
      "value: T) {\n"
      "  _builder.setExtension(extension, value)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "@Suppress(\"NOTHING_TO_INLINE\")\n"
      "public inline operator fun <T : Comparable<T>> set(\n"
      "  extension: com.google.protobuf.ExtensionLite<$message$, T>,\n"
      "  value: T\n"
      ") {\n"
      "  setExtension(extension, value)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "@Suppress(\"NOTHING_TO_INLINE\")\n"
      "public inline operator fun set(\n"
      "  extension: com.google.protobuf.ExtensionLite<$message$, "
      "com.google.protobuf.ByteString>,\n"
      "  value: com.google.protobuf.ByteString\n"
      ") {\n"
      "  setExtension(extension, value)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "@Suppress(\"NOTHING_TO_INLINE\")\n"
      "public inline operator fun <T : com.google.protobuf.MessageLite> set(\n"
      "  extension: com.google.protobuf.ExtensionLite<$message$, T>,\n"
      "  value: T\n"
      ") {\n"
      "  setExtension(extension, value)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  // TODO: generate these methods on Kotlin Native once the
  // [Mutable]List issue is resolved.
  if (!context_->options().jvm_dsl) return;

  printer->Print(
      "$jvm_synthetic$"
      "public fun<E : kotlin.Any> com.google.protobuf.kotlin.ExtensionList<E, "
      "$message$>.add(value: E) {\n"
      "  _builder.addExtension(this.extension, value)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "@Suppress(\"NOTHING_TO_INLINE\")\n"
      "public inline operator fun <E : kotlin.Any> "
      "com.google.protobuf.kotlin.ExtensionList<E, "
      "$message$>.plusAssign"
      "(value: E) {\n"
      "  add(value)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "public fun<E : kotlin.Any> com.google.protobuf.kotlin.ExtensionList<E, "
      "$message$>.addAll(values: Iterable<E>) {\n"
      "  for (value in values) {\n"
      "    add(value)\n"
      "  }\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "@Suppress(\"NOTHING_TO_INLINE\")\n"
      "public inline operator fun <E : kotlin.Any> "
      "com.google.protobuf.kotlin.ExtensionList<E, "
      "$message$>.plusAssign(values: "
      "Iterable<E>) {\n"
      "  addAll(values)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "public operator fun <E : kotlin.Any> "
      "com.google.protobuf.kotlin.ExtensionList<E, "
      "$message$>.set(index: Int, value: "
      "E) {\n"
      "  _builder.setExtension(this.extension, index, value)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);

  printer->Print(
      "$jvm_synthetic$"
      "@Suppress(\"NOTHING_TO_INLINE\")\n"
      "public inline fun com.google.protobuf.kotlin.ExtensionList<*, "
      "$message$>.clear() {\n"
      "  clear(extension)\n"
      "}\n\n",
      "jvm_synthetic", JvmSynthetic(context_->options()), "message",
      message_name);
}

}  // namespace kotlin
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"
