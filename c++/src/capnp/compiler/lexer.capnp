# Copyright (c) 2013, Kenton Varda <temporal@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@0xa73956d2621fc3ee;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("capnp::compiler");

struct Token {
  union {
    identifier @0 :Text;
    stringLiteral @1 :Text;
    integerLiteral @2 :UInt64;
    floatLiteral @3 :Float64;
    operator @4 :Text;
    parenthesizedList @5 :List(List(Token));
    bracketedList @6 :List(List(Token));
  }

  startByte @7 :UInt32;
  endByte @8 :UInt32;
}

struct Statement {
  tokens @0 :List(Token);
  union {
    line @1 :Void;
    block @2 :List(Statement);
  }

  docComment @3 :Text;

  startByte @4 :UInt32;
  endByte @5 :UInt32;
}

struct LexedTokens {
  # Lexer output when asked to parse tokens that don't form statements.

  tokens @0 :List(Token);
}

struct LexedStatements {
  # Lexer output when asked to parse statements.

  statements @0 :List(Statement);
}
