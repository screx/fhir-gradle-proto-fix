// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/fhir/codeable_concepts.h"

#include <string>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "google/fhir/annotations.h"
#include "google/fhir/proto_util.h"
#include "google/fhir/r4/codeable_concepts.h"
#include "google/fhir/status/statusor.h"
#include "google/fhir/stu3/codeable_concepts.h"
#include "proto/google/fhir/proto/annotations.pb.h"

#define CODEABLE_CONCEPTS_VERSION_DISPATCH(default_return_val, function_name, \
                                           first_arg, args...)                \
  switch (google::fhir::GetFhirVersion(first_arg)) {                          \
    case google::fhir::proto::STU3:                                           \
      return stu3::function_name(first_arg, args);                            \
    case google::fhir::proto::R4: {                                           \
      return r4::function_name(first_arg, args);                              \
    }                                                                         \
    default:                                                                  \
      return default_return_val;                                              \
  }

#define CODEABLE_CONCEPTS_VERSION_DISPATCH_WITH_STATUS(function_name,         \
                                                       first_arg, args...)    \
  CODEABLE_CONCEPTS_VERSION_DISPATCH(                                         \
      InvalidArgumentError(                                                   \
          absl::StrCat("FHIR version not supported by codeable_concepts.h: ", \
                       google::fhir::proto::FhirVersion_Name(                 \
                           google::fhir::GetFhirVersion(first_arg)))),        \
      function_name, first_arg, args);

#define CODEABLE_CONCEPTS_VERSION_DISPATCH_VOID(function_name, first_arg, \
                                                args...)                  \
  CODEABLE_CONCEPTS_VERSION_DISPATCH(, function_name, first_arg, args);

namespace google {
namespace fhir {

using ::absl::InvalidArgumentError;

const bool FindSystemCodeStringPair(const ::google::protobuf::Message& concept_proto,
                                    const CodeBoolFunc& func,
                                    std::string* found_system,
                                    std::string* found_code) {
  CODEABLE_CONCEPTS_VERSION_DISPATCH(false, FindSystemCodeStringPair,
                                     concept_proto, func, found_system,
                                     found_code);
}

const bool FindSystemCodeStringPair(const ::google::protobuf::Message& concept_proto,
                                    const CodeBoolFunc& func) {
  CODEABLE_CONCEPTS_VERSION_DISPATCH(false, FindSystemCodeStringPair,
                                     concept_proto, func);
}

void ForEachSystemCodeStringPair(const ::google::protobuf::Message& concept_proto,
                                 const CodeFunc& func) {
  CODEABLE_CONCEPTS_VERSION_DISPATCH_VOID(ForEachSystemCodeStringPair,
                                          concept_proto, func);
}

const std::vector<std::string> GetCodesWithSystem(
    const ::google::protobuf::Message& concept_proto,
    const absl::string_view target_system) {
  CODEABLE_CONCEPTS_VERSION_DISPATCH(std::vector<std::string>(),
                                     GetCodesWithSystem, concept_proto,
                                     target_system);
}

absl::StatusOr<const std::string> GetOnlyCodeWithSystem(
    const ::google::protobuf::Message& concept_proto, const absl::string_view system) {
  CODEABLE_CONCEPTS_VERSION_DISPATCH_WITH_STATUS(GetOnlyCodeWithSystem,
                                                 concept_proto, system);
}

absl::StatusOr<const std::string> ExtractCodeBySystem(
    const ::google::protobuf::Message& concept_proto, const absl::string_view system) {
  CODEABLE_CONCEPTS_VERSION_DISPATCH_WITH_STATUS(ExtractCodeBySystem,
                                                 concept_proto, system);
}

absl::Status AddCoding(::google::protobuf::Message* concept_proto,
                       const std::string& system, const std::string& code) {
  switch (google::fhir::GetFhirVersion(*concept_proto)) {
    case google::fhir::proto::STU3:
      return stu3::AddCoding(concept_proto, system, code);
    case google::fhir::proto::R4: {
      return r4::AddCoding(concept_proto, system, code);
    }
    default:
      return InvalidArgumentError(
          absl::StrCat("FHIR version not supported by codeable_concepts.h: ",
                       google::fhir::proto::FhirVersion_Name(
                           google::fhir::GetFhirVersion(*concept_proto))));
  }
}

absl::Status CopyCodeableConcept(const ::google::protobuf::Message& source,
                                 ::google::protobuf::Message* target) {
  CODEABLE_CONCEPTS_VERSION_DISPATCH_WITH_STATUS(CopyCodeableConcept, source,
                                                 target);
}

int CodingSize(const ::google::protobuf::Message& concept_proto) {
  switch (google::fhir::GetFhirVersion(concept_proto)) {
    case google::fhir::proto::STU3:
      return stu3::CodingSize(concept_proto);
    case google::fhir::proto::R4: {
      return r4::CodingSize(concept_proto);
    }
    default:
      return 0;
  }
}

}  // namespace fhir
}  // namespace google
