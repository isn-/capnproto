// Copyright (c) 2013, Kenton Varda <temporal@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "json.h"
#include <gtest/gtest.h>
#include <capnp/message.h>
#include <capnp/pretty-print.h>
#include <capnp/test.capnp.h>

namespace capnp {
namespace _ {  // private
namespace {

using capnproto_test::capnp::test::TestAllTypes;

TEST(Json, Smoke) {
  capnp::MallocMessageBuilder message;
  auto builder = message.initRoot<TestAllTypes>();
  auto parsed = capnp::fromJson("{\"textField\":\"0000000\"  ,\t\"int32Field\":\n  123}", builder);
  EXPECT_TRUE(parsed);
  //std::cout << "parsed: " << parsed << "\nmessage:\n"
  //  << capnp::prettyPrint(message.getRoot<TestAllTypes>()).flatten().cStr() << std::endl;
}


}  // namespace
}  // namespace _ (private)
}  // namespace capnp
