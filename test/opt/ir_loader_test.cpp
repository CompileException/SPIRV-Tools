// Copyright (c) 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>
#include <algorithm>

#include "message.h"
#include "opt/build_module.h"
#include "spirv-tools/libspirv.hpp"

namespace {

using namespace spvtools;

void DoRoundTripCheck(const std::string& text) {
  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  std::unique_ptr<ir::Module> module =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text);
  ASSERT_NE(nullptr, module) << "Failed to assemble\n" << text;

  std::vector<uint32_t> binary;
  module->ToBinary(&binary, /* skip_nop = */ false);

  std::string disassembled_text;
  EXPECT_TRUE(t.Disassemble(binary, &disassembled_text));
  EXPECT_EQ(text, disassembled_text);
}

TEST(IrBuilder, RoundTrip) {
  // #version 310 es
  // int add(int a, int b) { return a + b; }
  // void main() { add(1, 2); }
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
          "%1 = OpExtInstImport \"GLSL.std.450\"\n"
               "OpMemoryModel Logical GLSL450\n"
               "OpEntryPoint Vertex %main \"main\"\n"
               "OpSource ESSL 310\n"
               "OpSourceExtension \"GL_GOOGLE_cpp_style_line_directive\"\n"
               "OpSourceExtension \"GL_GOOGLE_include_directive\"\n"
               "OpName %main \"main\"\n"
               "OpName %add_i1_i1_ \"add(i1;i1;\"\n"
               "OpName %a \"a\"\n"
               "OpName %b \"b\"\n"
               "OpName %param \"param\"\n"
               "OpName %param_0 \"param\"\n"
       "%void = OpTypeVoid\n"
          "%9 = OpTypeFunction %void\n"
        "%int = OpTypeInt 32 1\n"
 "%_ptr_Function_int = OpTypePointer Function %int\n"
         "%12 = OpTypeFunction %int %_ptr_Function_int %_ptr_Function_int\n"
      "%int_1 = OpConstant %int 1\n"
      "%int_2 = OpConstant %int 2\n"
       "%main = OpFunction %void None %9\n"
         "%15 = OpLabel\n"
      "%param = OpVariable %_ptr_Function_int Function\n"
    "%param_0 = OpVariable %_ptr_Function_int Function\n"
               "OpStore %param %int_1\n"
               "OpStore %param_0 %int_2\n"
         "%16 = OpFunctionCall %int %add_i1_i1_ %param %param_0\n"
               "OpReturn\n"
               "OpFunctionEnd\n"
 "%add_i1_i1_ = OpFunction %int None %12\n"
          "%a = OpFunctionParameter %_ptr_Function_int\n"
          "%b = OpFunctionParameter %_ptr_Function_int\n"
         "%17 = OpLabel\n"
         "%18 = OpLoad %int %a\n"
         "%19 = OpLoad %int %b\n"
         "%20 = OpIAdd %int %18 %19\n"
               "OpReturnValue %20\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, RoundTripIncompleteBasicBlock) {
  DoRoundTripCheck(
      "%2 = OpFunction %1 None %3\n"
      "%4 = OpLabel\n"
      "OpNop\n");
}

TEST(IrBuilder, RoundTripIncompleteFunction) {
  DoRoundTripCheck("%2 = OpFunction %1 None %3\n");
}

TEST(IrBuilder, KeepLineDebugInfo) {
  // #version 310 es
  // void main() {}
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
          "%1 = OpExtInstImport \"GLSL.std.450\"\n"
               "OpMemoryModel Logical GLSL450\n"
               "OpEntryPoint Vertex %main \"main\"\n"
          "%3 = OpString \"minimal.vert\"\n"
               "OpSource ESSL 310\n"
               "OpName %main \"main\"\n"
               "OpLine %3 10 10\n"
       "%void = OpTypeVoid\n"
               "OpLine %3 100 100\n"
          "%5 = OpTypeFunction %void\n"
       "%main = OpFunction %void None %5\n"
               "OpLine %3 1 1\n"
               "OpNoLine\n"
               "OpLine %3 2 2\n"
               "OpLine %3 3 3\n"
          "%6 = OpLabel\n"
               "OpLine %3 4 4\n"
               "OpNoLine\n"
               "OpReturn\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, LocalGlobalVariables) {
  // #version 310 es
  //
  // float gv1 = 10.;
  // float gv2 = 100.;
  //
  // float f() {
  //   float lv1 = gv1 + gv2;
  //   float lv2 = gv1 * gv2;
  //   return lv1 / lv2;
  // }
  //
  // void main() {
  //   float lv1 = gv1 - gv2;
  // }
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
          "%1 = OpExtInstImport \"GLSL.std.450\"\n"
               "OpMemoryModel Logical GLSL450\n"
               "OpEntryPoint Vertex %main \"main\"\n"
               "OpSource ESSL 310\n"
               "OpName %main \"main\"\n"
               "OpName %f_ \"f(\"\n"
               "OpName %gv1 \"gv1\"\n"
               "OpName %gv2 \"gv2\"\n"
               "OpName %lv1 \"lv1\"\n"
               "OpName %lv2 \"lv2\"\n"
               "OpName %lv1_0 \"lv1\"\n"
       "%void = OpTypeVoid\n"
         "%10 = OpTypeFunction %void\n"
      "%float = OpTypeFloat 32\n"
         "%12 = OpTypeFunction %float\n"
 "%_ptr_Private_float = OpTypePointer Private %float\n"
        "%gv1 = OpVariable %_ptr_Private_float Private\n"
   "%float_10 = OpConstant %float 10\n"
        "%gv2 = OpVariable %_ptr_Private_float Private\n"
  "%float_100 = OpConstant %float 100\n"
 "%_ptr_Function_float = OpTypePointer Function %float\n"
       "%main = OpFunction %void None %10\n"
         "%17 = OpLabel\n"
      "%lv1_0 = OpVariable %_ptr_Function_float Function\n"
               "OpStore %gv1 %float_10\n"
               "OpStore %gv2 %float_100\n"
         "%18 = OpLoad %float %gv1\n"
         "%19 = OpLoad %float %gv2\n"
         "%20 = OpFSub %float %18 %19\n"
               "OpStore %lv1_0 %20\n"
               "OpReturn\n"
               "OpFunctionEnd\n"
         "%f_ = OpFunction %float None %12\n"
         "%21 = OpLabel\n"
        "%lv1 = OpVariable %_ptr_Function_float Function\n"
        "%lv2 = OpVariable %_ptr_Function_float Function\n"
         "%22 = OpLoad %float %gv1\n"
         "%23 = OpLoad %float %gv2\n"
         "%24 = OpFAdd %float %22 %23\n"
               "OpStore %lv1 %24\n"
         "%25 = OpLoad %float %gv1\n"
         "%26 = OpLoad %float %gv2\n"
         "%27 = OpFMul %float %25 %26\n"
               "OpStore %lv2 %27\n"
         "%28 = OpLoad %float %lv1\n"
         "%29 = OpLoad %float %lv2\n"
         "%30 = OpFDiv %float %28 %29\n"
               "OpReturnValue %30\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, OpUndefOutsideFunction) {
  // #version 310 es
  // void main() {}
  const std::string text =
      // clang-format off
               "OpMemoryModel Logical GLSL450\n"
        "%int = OpTypeInt 32 1\n"
       "%uint = OpTypeInt 32 0\n"
      "%float = OpTypeFloat 32\n"
          "%4 = OpUndef %int\n"
     "%int_10 = OpConstant %int 10\n"
          "%6 = OpUndef %uint\n"
       "%bool = OpTypeBool\n"
          "%8 = OpUndef %float\n"
     "%double = OpTypeFloat 64\n";
  // clang-format on

  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  std::unique_ptr<ir::Module> module =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text);
  ASSERT_NE(nullptr, module);

  const auto opundef_count = std::count_if(
      module->types_values_begin(), module->types_values_end(),
      [](const ir::Instruction& inst) { return inst.opcode() == SpvOpUndef; });
  EXPECT_EQ(3, opundef_count);

  std::vector<uint32_t> binary;
  module->ToBinary(&binary, /* skip_nop = */ false);

  std::string disassembled_text;
  EXPECT_TRUE(t.Disassemble(binary, &disassembled_text));
  EXPECT_EQ(text, disassembled_text);
}

TEST(IrBuilder, OpUndefInBasicBlock) {
  DoRoundTripCheck(
      // clang-format off
               "OpMemoryModel Logical GLSL450\n"
               "OpName %main \"main\"\n"
       "%void = OpTypeVoid\n"
       "%uint = OpTypeInt 32 0\n"
     "%double = OpTypeFloat 64\n"
          "%5 = OpTypeFunction %void\n"
       "%main = OpFunction %void None %5\n"
          "%6 = OpLabel\n"
          "%7 = OpUndef %uint\n"
          "%8 = OpUndef %double\n"
               "OpReturn\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, KeepLineDebugInfoBeforeType) {
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
               "OpMemoryModel Logical GLSL450\n"
          "%1 = OpString \"minimal.vert\"\n"
               "OpLine %1 1 1\n"
               "OpNoLine\n"
       "%void = OpTypeVoid\n"
               "OpLine %1 2 2\n"
          "%3 = OpTypeFunction %void\n");
  // clang-format on
}

TEST(IrBuilder, KeepLineDebugInfoBeforeLabel) {
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
               "OpMemoryModel Logical GLSL450\n"
          "%1 = OpString \"minimal.vert\"\n"
       "%void = OpTypeVoid\n"
          "%3 = OpTypeFunction %void\n"
       "%4 = OpFunction %void None %3\n"
          "%5 = OpLabel\n"
   "OpBranch %6\n"
               "OpLine %1 1 1\n"
               "OpLine %1 2 2\n"
          "%6 = OpLabel\n"
               "OpBranch %7\n"
               "OpLine %1 100 100\n"
          "%7 = OpLabel\n"
               "OpReturn\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, KeepLineDebugInfoBeforeFunctionEnd) {
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
               "OpMemoryModel Logical GLSL450\n"
          "%1 = OpString \"minimal.vert\"\n"
       "%void = OpTypeVoid\n"
          "%3 = OpTypeFunction %void\n"
       "%4 = OpFunction %void None %3\n"
               "OpLine %1 1 1\n"
               "OpLine %1 2 2\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, KeepModuleProcessedInRightPlace) {
  DoRoundTripCheck(
   // clang-format off
               "OpCapability Shader\n"
               "OpMemoryModel Logical GLSL450\n"
          "%1 = OpString \"minimal.vert\"\n"
               "OpName %void \"void\"\n"
               "OpModuleProcessed \"Made it faster\"\n"
               "OpModuleProcessed \".. and smaller\"\n"
       "%void = OpTypeVoid\n");
  // clang-format on
}

// Checks the given |error_message| is reported when trying to build a module
// from the given |assembly|.
void DoErrorMessageCheck(const std::string& assembly,
                         const std::string& error_message) {
  auto consumer = [error_message](spv_message_level_t level, const char* source,
                                  const spv_position_t& position,
                                  const char* m) {
    EXPECT_EQ(error_message, StringifyMessage(level, source, position, m));
  };

  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  std::unique_ptr<ir::Module> module =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, std::move(consumer), assembly);
  EXPECT_EQ(nullptr, module);
}

TEST(IrBuilder, FunctionInsideFunction) {
  DoErrorMessageCheck("%2 = OpFunction %1 None %3\n%5 = OpFunction %4 None %6",
                      "error: <instruction>:2:0:0: function inside function");
}

TEST(IrBuilder, MismatchOpFunctionEnd) {
  DoErrorMessageCheck("OpFunctionEnd",
                      "error: <instruction>:1:0:0: OpFunctionEnd without "
                      "corresponding OpFunction");
}

TEST(IrBuilder, OpFunctionEndInsideBasicBlock) {
  DoErrorMessageCheck(
      "%2 = OpFunction %1 None %3\n"
      "%4 = OpLabel\n"
      "OpFunctionEnd",
      "error: <instruction>:3:0:0: OpFunctionEnd inside basic block");
}

TEST(IrBuilder, BasicBlockOutsideFunction) {
  DoErrorMessageCheck("OpCapability Shader\n%1 = OpLabel",
                      "error: <instruction>:2:0:0: OpLabel outside function");
}

TEST(IrBuilder, OpLabelInsideBasicBlock) {
  DoErrorMessageCheck(
      "%2 = OpFunction %1 None %3\n"
      "%4 = OpLabel\n"
      "%5 = OpLabel",
      "error: <instruction>:3:0:0: OpLabel inside basic block");
}

TEST(IrBuilder, TerminatorOutsideFunction) {
  DoErrorMessageCheck(
      "OpReturn",
      "error: <instruction>:1:0:0: terminator instruction outside function");
}

TEST(IrBuilder, TerminatorOutsideBasicBlock) {
  DoErrorMessageCheck("%2 = OpFunction %1 None %3\nOpReturn",
                      "error: <instruction>:2:0:0: terminator instruction "
                      "outside basic block");
}

TEST(IrBuilder, NotAllowedInstAppearingInFunction) {
  DoErrorMessageCheck("%2 = OpFunction %1 None %3\n%5 = OpVariable %4 Function",
                      "error: <instruction>:2:0:0: Non-OpFunctionParameter "
                      "(opcode: 59) found inside function but outside basic "
                      "block");
}

}  // anonymous namespace
