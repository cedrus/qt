// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "v8.h"

namespace v8 {
namespace internal {


void LookupResult::Iterate(ObjectVisitor* visitor) {
  LookupResult* current = this;  // Could be NULL.
  while (current != NULL) {
    visitor->VisitPointer(BitCast<Object**>(&current->holder_));
    current = current->next_;
  }
}


#ifdef OBJECT_PRINT
void LookupResult::Print(FILE* out) {
  if (!IsFound()) {
    FPrintF(out, "Not Found\n");
    return;
  }

  FPrintF(out, "LookupResult:\n");
  FPrintF(out, " -cacheable = %s\n", IsCacheable() ? "true" : "false");
  FPrintF(out, " -attributes = %x\n", GetAttributes());
  switch (type()) {
    case NORMAL:
      FPrintF(out, " -type = normal\n");
      FPrintF(out, " -entry = %d", GetDictionaryEntry());
      break;
    case CONSTANT_FUNCTION:
      FPrintF(out, " -type = constant function\n");
      FPrintF(out, " -function:\n");
      GetConstantFunction()->Print(out);
      FPrintF(out, "\n");
      break;
    case FIELD:
      FPrintF(out, " -type = field\n");
      FPrintF(out, " -index = %d", GetFieldIndex());
      FPrintF(out, "\n");
      break;
    case CALLBACKS:
      FPrintF(out, " -type = call backs\n");
      FPrintF(out, " -callback object:\n");
      GetCallbackObject()->Print(out);
      break;
    case HANDLER:
      FPrintF(out, " -type = lookup proxy\n");
      break;
    case INTERCEPTOR:
      FPrintF(out, " -type = lookup interceptor\n");
      break;
    case TRANSITION:
      switch (GetTransitionDetails().type()) {
        case FIELD:
          FPrintF(out, " -type = map transition\n");
          FPrintF(out, " -map:\n");
          GetTransitionMap()->Print(out);
          FPrintF(out, "\n");
          return;
        case CONSTANT_FUNCTION:
          FPrintF(out, " -type = constant property transition\n");
          FPrintF(out, " -map:\n");
          GetTransitionMap()->Print(out);
          FPrintF(out, "\n");
          return;
        case CALLBACKS:
          FPrintF(out, " -type = callbacks transition\n");
          FPrintF(out, " -callback object:\n");
          GetCallbackObject()->Print(out);
          return;
        default:
          UNREACHABLE();
          return;
      }
    case NONEXISTENT:
      UNREACHABLE();
      break;
  }
}


void Descriptor::Print(FILE* out) {
  FPrintF(out, "Descriptor ");
  GetKey()->ShortPrint(out);
  FPrintF(out, " @ ");
  GetValue()->ShortPrint(out);
  FPrintF(out, " %d\n", GetDetails().descriptor_index());
}


#endif


} }  // namespace v8::internal
