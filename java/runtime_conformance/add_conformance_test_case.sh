#!/bin/bash

temp=$(mktemp -d)

if [[ $# -gt 0 ]]; then
    ver="$1"
else
    read -p "Enter version string (e.g., 3.25.4): " ver
fi

new_case_folder="java/runtime_conformance/v${ver}"
if [[ -d "${new_case_folder}" ]]; then
  echo "${ver} is already a test case. See java/runtime_conformance/ for all test cases."
  exit 0
fi

IFS='.' read -r unused minor <<< "$ver"

# Download and extract Protobuf
url="https://github.com/protocolbuffers/protobuf/releases/download/v${minor}/protobuf-${minor}.zip"
wget -O "$temp/protobuf.zip" "$url" || { echo "Download failed." >&2; exit 1; }
unzip -d "$temp" "$temp/protobuf.zip" || { echo "Unzip failed." >&2; rm -rf "$temp"; exit 1; }

# Build protoc
pushd "$temp/protobuf-${minor}"
bazel build :protoc || { echo "Failed to build protoc." >&2; popd; rm -rf "$temp"; exit 1; }
popd

# Run protoc
mkdir -p java/runtime_conformance/v${ver}
"$temp/protobuf-${minor}/bazel-bin/protoc" -Isrc/ \
  google/protobuf/unittest.proto \
  google/protobuf/unittest_import.proto \
  google/protobuf/unittest_import_public.proto \
  --java_out=java/runtime_conformance/v${ver} \
  || { echo "Failed to invoke protoc." >&2; rm -rf "$temp"; exit 1; }

# Update BUILD.bazel
build_content="
java_library(
    name = \"v${ver}\",
    srcs = [
        \"v${ver}/protobuf_unittest/UnittestProto.java\",
        \"v${ver}/com/google/protobuf/test/UnittestImport.java\",
        \"v${ver}/com/google/protobuf/test/UnittestImportPublic.java\",
    ],
    deps = [\"//java/core\"],
)
"
make_file="java/runtime_conformance/BUILD.bazel"

echo "$build_content" >> "$make_file"  # Append

# Cleanup
rm -rf "$temp"
