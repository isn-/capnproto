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
#include <numeric>
#include <kj/parse/common.h>
#include <kj/parse/char.h>
#include <kj/arena.h>
#include <capnp/orphan.h>

namespace p = kj::parse;

namespace capnp {

namespace _ {  // private

using None = kj::Tuple<>;
static constexpr None none = None();

template <typename SubParser>
struct IgnoreReturnValue {
  explicit constexpr IgnoreReturnValue(SubParser&& subParser)
      : subParser(kj::fwd<SubParser>(subParser)) {}

  template <typename Input>
  kj::Maybe<None> operator()(Input& input) const {
    return subParser(input) == nullptr ? kj::Maybe<None>(nullptr) : kj::Maybe<None>(none);
  }

private:
  SubParser subParser;
};

template <typename SubParser>
constexpr IgnoreReturnValue<SubParser> ignoreReturnValue(SubParser&& subParser) {
  return IgnoreReturnValue<SubParser>(kj::fwd<SubParser>(subParser));
}

class JsonReader {
public:
  using Input = p::IteratorInput<kj::StringPtr, const char*>;
  using Parser = kj::parse::ParserRef<Input, None>;

  JsonReader(DynamicStruct::Builder destParam)
    : dest(destParam)
    , orphanage(Orphanage::getForMessageContaining(dest)) {

    auto& ws = arena.copy(p::many(p::anyOfChars(" \t\r\n")));

    auto& object = parsers.object;

    auto& value = arena.copy(p::transform(
        p::choice(
          p::doubleQuotedString,
          p::number,
          p::integer),
        [this](kj::Maybe<kj::String> s, kj::Maybe<double> d, kj::Maybe<uint64_t> i)
        -> None {
          KJ_IF_MAYBE(value, s) {
            dest.set(field, Text::Reader(*value));
          } else KJ_IF_MAYBE(value, d) {
            dest.set(field, DynamicValue::Reader(*value));
          } else KJ_IF_MAYBE(value, i) {
            dest.set(field, DynamicValue::Reader(*value));
          }
          return none;
        }));


    auto& keyValue = arena.copy(p::sequence(
        p::transformOrReject(p::doubleQuotedString,
          [this](const kj::String& key)
          -> kj::Maybe<None> {
            return dest.getSchema().findFieldByName(key).map(
              [this](StructSchema::Field fieldParam) -> None {
                field = fieldParam;
                return none;
              });
        }),
        ws,
        p::exactChar<':'>(),
        ws,
        value));

    parsers.object = arena.copy(ignoreReturnValue(p::sequence(
        p::exactChar<'{'>(),
        p::optional(p::sequence(
          keyValue,
          ws,
          p::many(p::sequence(
            p::exactChar<','>(),
            ws,
            keyValue)))),
        p::exactChar<'}'>())));

    auto& message = arena.copy(ignoreReturnValue(p::sequence(
        object,
        ws,
        p::endOfInput)));

    parsers.value = value;
    parsers.message = message;
  }

  struct Parsers {
    Parser ws;
    Parser value;
    Parser object;
    Parser message;
  };

  Parsers& getParsers() { return parsers; }

private:
  Parsers parsers;
  kj::Arena arena;
  DynamicStruct::Builder dest;
  Orphanage orphanage;
  StructSchema::Field field;
};

}  // namespace _ (private)

// =======================================================================================

kj::String toJson(DynamicStruct::Reader source)
{
  return nullptr;
}

bool fromJson(kj::StringPtr jsonText, DynamicStruct::Builder dest)
{
  _::JsonReader reader(dest);
  _::JsonReader::Input input(jsonText.begin(), jsonText.end());
  return reader.getParsers().message(input) != nullptr;
}

} // namespace capnp
