load("@bazel_skylib//rules:build_test.bzl", "build_test")

def runtime_conformance(gencode_version, tags = []):
  minor = gencode_version[gencode_version.find(".") + 1:]

  protoc = Label("@com_google_protobuf_v%s//:protoc" % minor)
  gencode = [
    "v%s/protobuf_unittest/UnittestProto.java" % gencode_version,
    "v%s/com/google/protobuf/test/UnittestImport.java" % gencode_version,
    "v%s/com/google/protobuf/test/UnittestImportPublic.java" % gencode_version,
  ]
  native.genrule(
    name = "unittest_proto_gencode_v" + gencode_version,
    srcs = [
      "//src/google/protobuf:unittest_proto_srcs",
    ],
    outs = gencode,
    cmd = "$(location %s) " % protoc +
    "$(locations //src/google/protobuf:unittest_proto_srcs) " +
    " -Isrc/ --java_out=$(@D)/v%s" % gencode_version,
    tools = [protoc],
  )

  conformance_name = "conformance_v" + gencode_version
  conformance_lib_name = conformance_name + "_lib"
  native.java_library(
    name =  conformance_lib_name,
    srcs = gencode,
    deps = ["//java/core"],
  )

  build_test(
    name = conformance_name,
    targets = [":" + conformance_lib_name],
    tags = tags,
  )
