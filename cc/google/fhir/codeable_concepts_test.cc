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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "google/fhir/test_helper.h"
#include "google/fhir/testutil/proto_matchers.h"
#include "proto/google/fhir/proto/r4/core/datatypes.pb.h"
#include "proto/google/fhir/proto/stu3/datatypes.pb.h"
#include "testdata/r4/profiles/test.pb.h"
#include "testdata/stu3/profiles/test.pb.h"

namespace google {
namespace fhir {

namespace {

using Stu3TestObservation = stu3::testing::TestObservation;
using R4TestObservation = r4::testing::TestObservation;
using ::testing::ElementsAre;

const Stu3TestObservation::CodeableConceptForCode GetConceptStu3() {
  return ReadProto<Stu3TestObservation::CodeableConceptForCode>(
      "testdata/stu3/profiles/testobservation_codeableconceptforcode.prototxt");
}

TEST(CodeableConceptsTest, ForEachSystemCodeStringPair) {
  const auto codeable_concept = GetConceptStu3();
  std::string sys_accum = "";
  std::string code_accum = "";
  ForEachSystemCodeStringPair(
      codeable_concept, [&sys_accum, &code_accum](const std::string& sys,
                                                  const std::string& code) {
        absl::StrAppend(&sys_accum, sys, ",");
        absl::StrAppend(&code_accum, code, ",");
      });
  EXPECT_EQ(
      sys_accum,
      "http://sysf.org,http://sysg.org,http://sysg.org,http://sysa.org,http://"
      "sysb.org,http://sysb.org,http://sysc.org,http://sysd.org,");
  EXPECT_EQ(code_accum,
            "fcode,gcode1,gcode2,acode,bcode1,bcode2,8472,8675329,");
}

TEST(CodeableConceptsTest, GetCodesWithSystemUnprofiled) {
  const auto codeable_concept = GetConceptStu3();
  ASSERT_THAT(GetCodesWithSystem(codeable_concept, "http://sysg.org"),
              ElementsAre("gcode1", "gcode2"));
}

TEST(CodeableConceptsTest, GetCodesWithSystemFixedSystem) {
  const auto codeable_concept = GetConceptStu3();
  ASSERT_THAT(GetCodesWithSystem(codeable_concept, "http://sysb.org"),
              ElementsAre("bcode1", "bcode2"));
}

TEST(CodeableConceptsTest, GetCodesWithSystemFixedCode) {
  const auto codeable_concept = GetConceptStu3();
  ASSERT_THAT(GetCodesWithSystem(codeable_concept, "http://sysc.org"),
              ElementsAre("8472"));
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemUnprofiled) {
  const auto codeable_concept = GetConceptStu3();
  EXPECT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysf.org").value(),
            "fcode");
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemFixedSystem) {
  const auto codeable_concept = GetConceptStu3();
  EXPECT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysa.org").value(),
            "acode");
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemFixedCode) {
  const auto codeable_concept = GetConceptStu3();
  EXPECT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysc.org").value(),
            "8472");
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemUnprofiledTooMany) {
  const auto codeable_concept = GetConceptStu3();
  ASSERT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysg.org")
                .status()
                .code(),
            ::absl::StatusCode::kAlreadyExists);
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemProfiledTooMany) {
  const auto codeable_concept = GetConceptStu3();
  ASSERT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysb.org")
                .status()
                .code(),
            ::absl::StatusCode::kAlreadyExists);
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemNone) {
  const auto codeable_concept = GetConceptStu3();
  ASSERT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysq.org")
                .status()
                .code(),
            ::absl::StatusCode::kNotFound);
}

TEST(CodeableConceptsTest, ClearAllCodingsWithSystemFixedCode) {
  auto codeable_concept = GetConceptStu3();
  EXPECT_FALSE(
      ClearAllCodingsWithSystem(&codeable_concept, "http://sysc.org").ok());
}

TEST(CodeableConceptsTest, CopyCodeableConcept) {
  stu3::proto::CodeableConcept codeable_concept = PARSE_STU3_PROTO(R"pb(
    coding {
      system { value: "foo" },
      code { value: "bar" }
    },
    coding {
      system { value: "http://catA.org" },
      code { value: "bar" }
    }
    coding {
      system { value: "http://sysa.org" }
      code { value: "acode" }
      display { value: "A Display" }
    },
    coding {
      system { value: "http://sysc.org" },
      code { value: "8472" }
    },
    text { value: "some text to copy" }
    id { value: "2134" }
    extension {
      url { value: "foo" }
      value { string_value { value: "bar" } }
    }
    extension {
      url { value: "baz" }
      value { integer { value: 5 } }
    }
  )pb");
  Stu3TestObservation::CodeableConceptForCode concept_for_code =
      PARSE_STU3_PROTO(R"proto(
        # inlined system
        sys_a {
          code { value: "acode" },
          display { value: "A Display" }
        }
        # inlined system and code
        sys_c {}
        coding {
          system { value: "foo" },
          code { value: "bar" }
        }
        coding {
          system { value: "http://catA.org" },
          code { value: "bar" }
        }
        text { value: "some text to copy" }
        id { value: "2134" }
        extension {
          url { value: "foo" }
          value { string_value { value: "bar" } }
        }
        extension {
          url { value: "baz" }
          value { integer { value: 5 } }
        }
      )proto");
  Stu3TestObservation::CodeableConceptForCategory concept_for_cat =
      PARSE_STU3_PROTO(R"proto(
        coding {
          system { value: "http://sysa.org" }
          code { value: "acode" }
          display { value: "A Display" }
        },
        coding {
          system { value: "http://sysc.org" },
          code { value: "8472" }
        },
        coding {
          system { value: "foo" },
          code { value: "bar" }
        },
        # inlined system
        cat_a { code { value: "bar" } }
        text { value: "some text to copy" }
        id { value: "2134" }
        extension {
          url { value: "foo" }
          value { string_value { value: "bar" } }
        }
        extension {
          url { value: "baz" }
          value { integer { value: 5 } }
        }
      )proto");

  stu3::proto::CodeableConcept profiled_to_unprofiled;
  FHIR_ASSERT_OK(
      CopyCodeableConcept(concept_for_code, &profiled_to_unprofiled));
  ASSERT_THAT(codeable_concept,
              testutil::EqualsProtoIgnoringReordering(profiled_to_unprofiled));

  Stu3TestObservation::CodeableConceptForCode unprofiled_to_profiled;
  FHIR_ASSERT_OK(
      CopyCodeableConcept(codeable_concept, &unprofiled_to_profiled));
  ASSERT_THAT(concept_for_code,
              testutil::EqualsProtoIgnoringReordering(unprofiled_to_profiled));

  Stu3TestObservation::CodeableConceptForCategory profiled_to_profiled;
  FHIR_ASSERT_OK(CopyCodeableConcept(concept_for_code, &profiled_to_profiled));
  ASSERT_THAT(concept_for_cat,
              testutil::EqualsProtoIgnoringReordering(profiled_to_profiled));
}

TEST(CodeableConceptsTest, AddCodingFromStringsSTU3) {
  stu3::proto::CodeableConcept codeable_concept;

  FHIR_CHECK_OK(AddCoding(&codeable_concept, "http://sysq.org", "qcode1"));
  FHIR_CHECK_OK(AddCoding(&codeable_concept, "http://sysq.org", "qcode2"));
  FHIR_CHECK_OK(AddCoding(&codeable_concept, "http://sysr.org", "rcode"));

  EXPECT_EQ(codeable_concept.coding_size(), 3);
  std::string code_accum = "";
  ForEachSystemCodeStringPair(
      codeable_concept,
      [&code_accum](const std::string& code, const std::string& system) {
        absl::StrAppend(&code_accum, code, ",");
      });
  EXPECT_EQ(code_accum, "http://sysq.org,http://sysq.org,http://sysr.org,");
}

TEST(CodeableConceptsTest, AddCodingFromStringsR4) {
  r4::core::CodeableConcept codeable_concept;

  FHIR_CHECK_OK(AddCoding(&codeable_concept, "http://sysq.org", "qcode1"));
  FHIR_CHECK_OK(AddCoding(&codeable_concept, "http://sysq.org", "qcode2"));
  FHIR_CHECK_OK(AddCoding(&codeable_concept, "http://sysr.org", "rcode"));

  EXPECT_EQ(codeable_concept.coding_size(), 3);
  std::string code_accum = "";
  ForEachSystemCodeStringPair(
      codeable_concept,
      [&code_accum](const std::string& code, const std::string& system) {
        absl::StrAppend(&code_accum, code, ",");
      });
  EXPECT_EQ(code_accum, "http://sysq.org,http://sysq.org,http://sysr.org,");
}

const R4TestObservation::CodeableConceptForCode GetConceptR4() {
  return ReadProto<R4TestObservation::CodeableConceptForCode>(
      "testdata/r4/profiles/testobservation_codeableconceptforcode.prototxt");
}

TEST(CodeableConceptsTest, FindSystemCodeStringPairUnprofiledR4) {
  std::string found_system;
  std::string found_code;
  const auto codeable_concept = GetConceptR4();
  EXPECT_TRUE(FindSystemCodeStringPair(
      codeable_concept,
      [](const std::string& system, const std::string& code) {
        return system == "http://sysg.org" && code == "gcode1";
      },
      &found_system, &found_code));
  EXPECT_EQ(found_system, "http://sysg.org");
  EXPECT_EQ(found_code, "gcode1");
}

TEST(CodeableConceptsTest, FindSystemCodeStringPairFixedSystemR4) {
  std::string found_system;
  std::string found_code;
  const auto codeable_concept = GetConceptR4();
  EXPECT_TRUE(FindSystemCodeStringPair(
      codeable_concept,
      [](const std::string& system, const std::string& code) {
        return system == "http://sysb.org" && code == "bcode2";
      },
      &found_system, &found_code));

  EXPECT_EQ(found_system, "http://sysb.org");
  EXPECT_EQ(found_code, "bcode2");
}

TEST(CodeableConceptsTest, FindSystemCodeStringPairFixedCodeR4) {
  std::string found_system;
  std::string found_code;
  const auto codeable_concept = GetConceptR4();
  EXPECT_TRUE(FindSystemCodeStringPair(
      codeable_concept,
      [](const std::string& system, const std::string& code) {
        return system == "http://sysd.org" && code == "8675329";
      },
      &found_system, &found_code));

  EXPECT_EQ(found_system, "http://sysd.org");
  EXPECT_EQ(found_code, "8675329");
}

TEST(CodeableConceptsTest, FindSystemCodeStringPairNotFoundR4) {
  std::string found_system;
  std::string found_code;
  const auto codeable_concept = GetConceptR4();
  EXPECT_FALSE(FindSystemCodeStringPair(
      codeable_concept,
      [](const std::string& system, const std::string& code) { return false; },
      &found_system, &found_code));
}

TEST(CodeableConceptsTest, ForEachSystemCodeStringPairR4) {
  const auto codeable_concept = GetConceptR4();
  std::string sys_accum = "";
  std::string code_accum = "";
  ForEachSystemCodeStringPair(
      codeable_concept, [&sys_accum, &code_accum](const std::string& sys,
                                                  const std::string& code) {
        absl::StrAppend(&sys_accum, sys, ",");
        absl::StrAppend(&code_accum, code, ",");
      });
  EXPECT_EQ(
      sys_accum,
      "http://sysf.org,http://sysg.org,http://sysg.org,http://sysa.org,http://"
      "sysb.org,http://sysb.org,http://sysc.org,http://sysd.org,");
  EXPECT_EQ(code_accum,
            "fcode,gcode1,gcode2,acode,bcode1,bcode2,8472,8675329,");
}

TEST(CodeableConceptsTest, GetCodesWithSystemUnprofiledR4) {
  const auto codeable_concept = GetConceptR4();
  ASSERT_THAT(GetCodesWithSystem(codeable_concept, "http://sysg.org"),
              ElementsAre("gcode1", "gcode2"));
}

TEST(CodeableConceptsTest, GetCodesWithSystemFixedSystemR4) {
  const auto codeable_concept = GetConceptR4();
  ASSERT_THAT(GetCodesWithSystem(codeable_concept, "http://sysb.org"),
              ElementsAre("bcode1", "bcode2"));
}

TEST(CodeableConceptsTest, GetCodesWithSystemFixedCodeR4) {
  const auto codeable_concept = GetConceptR4();
  ASSERT_THAT(GetCodesWithSystem(codeable_concept, "http://sysc.org"),
              ElementsAre("8472"));
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemUnprofiledR4) {
  const auto codeable_concept = GetConceptR4();
  EXPECT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysf.org").value(),
            "fcode");
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemFixedSystemR4) {
  const auto codeable_concept = GetConceptR4();
  EXPECT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysa.org").value(),
            "acode");
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemFixedCodeR4) {
  const auto codeable_concept = GetConceptR4();
  EXPECT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysc.org").value(),
            "8472");
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemUnprofiledTooManyR4) {
  const auto codeable_concept = GetConceptR4();
  ASSERT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysg.org")
                .status()
                .code(),
            ::absl::StatusCode::kAlreadyExists);
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemProfiledTooManyR4) {
  const auto codeable_concept = GetConceptR4();
  ASSERT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysb.org")
                .status()
                .code(),
            ::absl::StatusCode::kAlreadyExists);
}

TEST(CodeableConceptsTest, GetOnlyCodeWithSystemNoneR4) {
  const auto codeable_concept = GetConceptR4();
  ASSERT_EQ(GetOnlyCodeWithSystem(codeable_concept, "http://sysq.org")
                .status()
                .code(),
            ::absl::StatusCode::kNotFound);
}

TEST(CodeableConceptsTest, ClearAllCodingsWithSystemUnprofiledR4) {
  auto codeable_concept = GetConceptR4();
  FHIR_CHECK_OK(
      ClearAllCodingsWithSystem(&codeable_concept, "http://sysg.org"));
  std::string display_accum = "";
  ForEachSystemCodeStringPair(
      codeable_concept,
      [&display_accum](const std::string& code, const std::string& system) {
        absl::StrAppend(&display_accum, code, ",");
      });
  EXPECT_EQ(display_accum,
            "http://sysf.org,http://sysa.org,http://sysb.org,http://"
            "sysb.org,http://sysc.org,http://sysd.org,");
}

TEST(CodeableConceptsTest, ClearAllCodingsWithSystemFixedSystemR4) {
  auto codeable_concept = GetConceptR4();
  FHIR_CHECK_OK(
      ClearAllCodingsWithSystem(&codeable_concept, "http://sysb.org"));
  std::string display_accum = "";
  ForEachSystemCodeStringPair(
      codeable_concept,
      [&display_accum](const std::string& code, const std::string& system) {
        absl::StrAppend(&display_accum, code, ",");
      });
  EXPECT_EQ(display_accum,
            "http://sysf.org,http://sysg.org,http://sysg.org,http://"
            "sysa.org,http://sysc.org,http://sysd.org,");
}

TEST(CodeableConceptsTest, ClearAllCodingsWithSystemFixedCodeR4) {
  auto codeable_concept = GetConceptR4();
  EXPECT_FALSE(
      ClearAllCodingsWithSystem(&codeable_concept, "http://sysc.org").ok());
}

TEST(CodeableConceptsTest, CopyCodeableConceptR4) {
  r4::core::CodeableConcept codeable_concept = PARSE_STU3_PROTO(R"pb(
    coding {
      system { value: "foo" },
      code { value: "bar" }
    },
    coding {
      system { value: "http://catA.org" },
      code { value: "bar" }
    }
    coding {
      system { value: "http://sysa.org" }
      code { value: "acode" }
      display { value: "A Display" }
    },
    coding {
      system { value: "http://sysc.org" },
      code { value: "8472" }
    },
    text { value: "some text to copy" }
    id { value: "2134" }
    extension {
      url { value: "foo" }
      value { string_value { value: "bar" } }
    }
    extension {
      url { value: "baz" }
      value { integer { value: 5 } }
    }
  )pb");
  R4TestObservation::CodeableConceptForCode concept_for_code =
      PARSE_STU3_PROTO(R"proto(
        # inlined system
        sys_a {
          code { value: "acode" },
          display { value: "A Display" }
        }
        # inlined system and code
        sys_c {}
        coding {
          system { value: "foo" },
          code { value: "bar" }
        }
        coding {
          system { value: "http://catA.org" },
          code { value: "bar" }
        }
        text { value: "some text to copy" }
        id { value: "2134" }
        extension {
          url { value: "foo" }
          value { string_value { value: "bar" } }
        }
        extension {
          url { value: "baz" }
          value { integer { value: 5 } }
        }
      )proto");
  R4TestObservation::CodeableConceptForCategory concept_for_cat =
      PARSE_STU3_PROTO(R"proto(
        coding {
          system { value: "http://sysa.org" }
          code { value: "acode" }
          display { value: "A Display" }
        },
        coding {
          system { value: "http://sysc.org" },
          code { value: "8472" }
        },
        coding {
          system { value: "foo" },
          code { value: "bar" }
        },
        # inlined system
        cat_a { code { value: "bar" } }
        text { value: "some text to copy" }
        id { value: "2134" }
        extension {
          url { value: "foo" }
          value { string_value { value: "bar" } }
        }
        extension {
          url { value: "baz" }
          value { integer { value: 5 } }
        }
      )proto");

  r4::core::CodeableConcept profiled_to_unprofiled;
  FHIR_ASSERT_OK(
      CopyCodeableConcept(concept_for_code, &profiled_to_unprofiled));
  ASSERT_THAT(codeable_concept,
              testutil::EqualsProtoIgnoringReordering(profiled_to_unprofiled));

  R4TestObservation::CodeableConceptForCode unprofiled_to_profiled;
  FHIR_ASSERT_OK(
      CopyCodeableConcept(codeable_concept, &unprofiled_to_profiled));
  ASSERT_THAT(concept_for_code,
              testutil::EqualsProtoIgnoringReordering(unprofiled_to_profiled));

  R4TestObservation::CodeableConceptForCategory profiled_to_profiled;
  FHIR_ASSERT_OK(CopyCodeableConcept(concept_for_code, &profiled_to_profiled));
  ASSERT_THAT(concept_for_cat,
              testutil::EqualsProtoIgnoringReordering(profiled_to_profiled));
}

}  // namespace

}  // namespace fhir
}  // namespace google
