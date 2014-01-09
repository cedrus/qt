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

#include "disassembler.h"
#include "disasm.h"
#include "jsregexp.h"
#include "objects-visiting.h"

namespace v8 {
namespace internal {

#ifdef OBJECT_PRINT

static const char* TypeToString(InstanceType type);


void MaybeObject::Print(FILE* out) {
  Object* this_as_object;
  if (ToObject(&this_as_object)) {
    if (this_as_object->IsSmi()) {
      Smi::cast(this_as_object)->SmiPrint(out);
    } else {
      HeapObject::cast(this_as_object)->HeapObjectPrint(out);
    }
  } else {
    Failure::cast(this)->FailurePrint(out);
  }
  Flush(out);
}


void MaybeObject::PrintLn(FILE* out) {
  Print(out);
  FPrintF(out, "\n");
}


void HeapObject::PrintHeader(FILE* out, const char* id) {
  FPrintF(out, "%p: [%s]\n", reinterpret_cast<void*>(this), id);
}


void HeapObject::HeapObjectPrint(FILE* out) {
  InstanceType instance_type = map()->instance_type();

  HandleScope scope;
  if (instance_type < FIRST_NONSTRING_TYPE) {
    String::cast(this)->StringPrint(out);
    return;
  }

  switch (instance_type) {
    case MAP_TYPE:
      Map::cast(this)->MapPrint(out);
      break;
    case HEAP_NUMBER_TYPE:
      HeapNumber::cast(this)->HeapNumberPrint(out);
      break;
    case FIXED_DOUBLE_ARRAY_TYPE:
      FixedDoubleArray::cast(this)->FixedDoubleArrayPrint(out);
      break;
    case FIXED_ARRAY_TYPE:
      FixedArray::cast(this)->FixedArrayPrint(out);
      break;
    case BYTE_ARRAY_TYPE:
      ByteArray::cast(this)->ByteArrayPrint(out);
      break;
    case FREE_SPACE_TYPE:
      FreeSpace::cast(this)->FreeSpacePrint(out);
      break;
    case EXTERNAL_PIXEL_ARRAY_TYPE:
      ExternalPixelArray::cast(this)->ExternalPixelArrayPrint(out);
      break;
    case EXTERNAL_BYTE_ARRAY_TYPE:
      ExternalByteArray::cast(this)->ExternalByteArrayPrint(out);
      break;
    case EXTERNAL_UNSIGNED_BYTE_ARRAY_TYPE:
      ExternalUnsignedByteArray::cast(this)
          ->ExternalUnsignedByteArrayPrint(out);
      break;
    case EXTERNAL_SHORT_ARRAY_TYPE:
      ExternalShortArray::cast(this)->ExternalShortArrayPrint(out);
      break;
    case EXTERNAL_UNSIGNED_SHORT_ARRAY_TYPE:
      ExternalUnsignedShortArray::cast(this)
          ->ExternalUnsignedShortArrayPrint(out);
      break;
    case EXTERNAL_INT_ARRAY_TYPE:
      ExternalIntArray::cast(this)->ExternalIntArrayPrint(out);
      break;
    case EXTERNAL_UNSIGNED_INT_ARRAY_TYPE:
      ExternalUnsignedIntArray::cast(this)->ExternalUnsignedIntArrayPrint(out);
      break;
    case EXTERNAL_FLOAT_ARRAY_TYPE:
      ExternalFloatArray::cast(this)->ExternalFloatArrayPrint(out);
      break;
    case EXTERNAL_DOUBLE_ARRAY_TYPE:
      ExternalDoubleArray::cast(this)->ExternalDoubleArrayPrint(out);
      break;
    case FILLER_TYPE:
      FPrintF(out, "filler");
      break;
    case JS_OBJECT_TYPE:  // fall through
    case JS_CONTEXT_EXTENSION_OBJECT_TYPE:
    case JS_ARRAY_TYPE:
    case JS_REGEXP_TYPE:
      JSObject::cast(this)->JSObjectPrint(out);
      break;
    case ODDBALL_TYPE:
      Oddball::cast(this)->to_string()->Print(out);
      break;
    case JS_MODULE_TYPE:
      JSModule::cast(this)->JSModulePrint(out);
      break;
    case JS_FUNCTION_TYPE:
      JSFunction::cast(this)->JSFunctionPrint(out);
      break;
    case JS_GLOBAL_PROXY_TYPE:
      JSGlobalProxy::cast(this)->JSGlobalProxyPrint(out);
      break;
    case JS_GLOBAL_OBJECT_TYPE:
      JSGlobalObject::cast(this)->JSGlobalObjectPrint(out);
      break;
    case JS_BUILTINS_OBJECT_TYPE:
      JSBuiltinsObject::cast(this)->JSBuiltinsObjectPrint(out);
      break;
    case JS_VALUE_TYPE:
      FPrintF(out, "Value wrapper around:");
      JSValue::cast(this)->value()->Print(out);
      break;
    case JS_DATE_TYPE:
      JSDate::cast(this)->JSDatePrint(out);
      break;
    case CODE_TYPE:
      Code::cast(this)->CodePrint(out);
      break;
    case JS_PROXY_TYPE:
      JSProxy::cast(this)->JSProxyPrint(out);
      break;
    case JS_FUNCTION_PROXY_TYPE:
      JSFunctionProxy::cast(this)->JSFunctionProxyPrint(out);
      break;
    case JS_WEAK_MAP_TYPE:
      JSWeakMap::cast(this)->JSWeakMapPrint(out);
      break;
    case FOREIGN_TYPE:
      Foreign::cast(this)->ForeignPrint(out);
      break;
    case SHARED_FUNCTION_INFO_TYPE:
      SharedFunctionInfo::cast(this)->SharedFunctionInfoPrint(out);
      break;
    case JS_MESSAGE_OBJECT_TYPE:
      JSMessageObject::cast(this)->JSMessageObjectPrint(out);
      break;
    case JS_GLOBAL_PROPERTY_CELL_TYPE:
      JSGlobalPropertyCell::cast(this)->JSGlobalPropertyCellPrint(out);
      break;
#define MAKE_STRUCT_CASE(NAME, Name, name) \
  case NAME##_TYPE:                        \
    Name::cast(this)->Name##Print(out);    \
    break;
  STRUCT_LIST(MAKE_STRUCT_CASE)
#undef MAKE_STRUCT_CASE

    default:
      FPrintF(out, "UNKNOWN TYPE %d", map()->instance_type());
      UNREACHABLE();
      break;
  }
}


void ByteArray::ByteArrayPrint(FILE* out) {
  FPrintF(out, "byte array, data starts at %p", GetDataStartAddress());
}


void FreeSpace::FreeSpacePrint(FILE* out) {
  FPrintF(out, "free space, size %d", Size());
}


void ExternalPixelArray::ExternalPixelArrayPrint(FILE* out) {
  FPrintF(out, "external pixel array");
}


void ExternalByteArray::ExternalByteArrayPrint(FILE* out) {
  FPrintF(out, "external byte array");
}


void ExternalUnsignedByteArray::ExternalUnsignedByteArrayPrint(FILE* out) {
  FPrintF(out, "external unsigned byte array");
}


void ExternalShortArray::ExternalShortArrayPrint(FILE* out) {
  FPrintF(out, "external short array");
}


void ExternalUnsignedShortArray::ExternalUnsignedShortArrayPrint(FILE* out) {
  FPrintF(out, "external unsigned short array");
}


void ExternalIntArray::ExternalIntArrayPrint(FILE* out) {
  FPrintF(out, "external int array");
}


void ExternalUnsignedIntArray::ExternalUnsignedIntArrayPrint(FILE* out) {
  FPrintF(out, "external unsigned int array");
}


void ExternalFloatArray::ExternalFloatArrayPrint(FILE* out) {
  FPrintF(out, "external float array");
}


void ExternalDoubleArray::ExternalDoubleArrayPrint(FILE* out) {
  FPrintF(out, "external double array");
}


void JSObject::PrintProperties(FILE* out) {
  if (HasFastProperties()) {
    DescriptorArray* descs = map()->instance_descriptors();
    for (int i = 0; i < map()->NumberOfOwnDescriptors(); i++) {
      FPrintF(out, "   ");
      descs->GetKey(i)->StringPrint(out);
      FPrintF(out, ": ");
      switch (descs->GetType(i)) {
        case FIELD: {
          int index = descs->GetFieldIndex(i);
          FastPropertyAt(index)->ShortPrint(out);
          FPrintF(out, " (field at offset %d)\n", index);
          break;
        }
        case CONSTANT_FUNCTION:
          descs->GetConstantFunction(i)->ShortPrint(out);
          FPrintF(out, " (constant function)\n");
          break;
        case CALLBACKS:
          descs->GetCallbacksObject(i)->ShortPrint(out);
          FPrintF(out, " (callback)\n");
          break;
        case NORMAL:  // only in slow mode
        case HANDLER:  // only in lookup results, not in descriptors
        case INTERCEPTOR:  // only in lookup results, not in descriptors
        // There are no transitions in the descriptor array.
        case TRANSITION:
        case NONEXISTENT:
          UNREACHABLE();
          break;
      }
    }
  } else {
    property_dictionary()->Print(out);
  }
}


void JSObject::PrintElements(FILE* out) {
  // Don't call GetElementsKind, its validation code can cause the printer to
  // fail when debugging.
  switch (map()->elements_kind()) {
    case FAST_HOLEY_SMI_ELEMENTS:
    case FAST_SMI_ELEMENTS:
    case FAST_HOLEY_ELEMENTS:
    case FAST_ELEMENTS: {
      // Print in array notation for non-sparse arrays.
      FixedArray* p = FixedArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: ", i);
        p->get(i)->ShortPrint(out);
        FPrintF(out, "\n");
      }
      break;
    }
    case FAST_HOLEY_DOUBLE_ELEMENTS:
    case FAST_DOUBLE_ELEMENTS: {
      // Print in array notation for non-sparse arrays.
      if (elements()->length() > 0) {
        FixedDoubleArray* p = FixedDoubleArray::cast(elements());
        for (int i = 0; i < p->length(); i++) {
          if (p->is_the_hole(i)) {
            FPrintF(out, "   %d: <the hole>", i);
          } else {
            FPrintF(out, "   %d: %g", i, p->get_scalar(i));
          }
          FPrintF(out, "\n");
        }
      }
      break;
    }
    case EXTERNAL_PIXEL_ELEMENTS: {
      ExternalPixelArray* p = ExternalPixelArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: %d\n", i, p->get_scalar(i));
      }
      break;
    }
    case EXTERNAL_BYTE_ELEMENTS: {
      ExternalByteArray* p = ExternalByteArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: %d\n", i, static_cast<int>(p->get_scalar(i)));
      }
      break;
    }
    case EXTERNAL_UNSIGNED_BYTE_ELEMENTS: {
      ExternalUnsignedByteArray* p =
          ExternalUnsignedByteArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: %d\n", i, static_cast<int>(p->get_scalar(i)));
      }
      break;
    }
    case EXTERNAL_SHORT_ELEMENTS: {
      ExternalShortArray* p = ExternalShortArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: %d\n", i, static_cast<int>(p->get_scalar(i)));
      }
      break;
    }
    case EXTERNAL_UNSIGNED_SHORT_ELEMENTS: {
      ExternalUnsignedShortArray* p =
          ExternalUnsignedShortArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: %d\n", i, static_cast<int>(p->get_scalar(i)));
      }
      break;
    }
    case EXTERNAL_INT_ELEMENTS: {
      ExternalIntArray* p = ExternalIntArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: %d\n", i, static_cast<int>(p->get_scalar(i)));
      }
      break;
    }
    case EXTERNAL_UNSIGNED_INT_ELEMENTS: {
      ExternalUnsignedIntArray* p =
          ExternalUnsignedIntArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: %d\n", i, static_cast<int>(p->get_scalar(i)));
      }
      break;
    }
    case EXTERNAL_FLOAT_ELEMENTS: {
      ExternalFloatArray* p = ExternalFloatArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "   %d: %f\n", i, p->get_scalar(i));
      }
      break;
    }
    case EXTERNAL_DOUBLE_ELEMENTS: {
      ExternalDoubleArray* p = ExternalDoubleArray::cast(elements());
      for (int i = 0; i < p->length(); i++) {
        FPrintF(out, "  %d: %f\n", i, p->get_scalar(i));
      }
      break;
    }
    case DICTIONARY_ELEMENTS:
      elements()->Print(out);
      break;
    case NON_STRICT_ARGUMENTS_ELEMENTS: {
      FixedArray* p = FixedArray::cast(elements());
      for (int i = 2; i < p->length(); i++) {
        FPrintF(out, "   %d: ", i);
        p->get(i)->ShortPrint(out);
        FPrintF(out, "\n");
      }
      break;
    }
  }
}


void JSObject::PrintTransitions(FILE* out) {
  if (!map()->HasTransitionArray()) return;
  TransitionArray* transitions = map()->transitions();
  for (int i = 0; i < transitions->number_of_transitions(); i++) {
    FPrintF(out, "   ");
    transitions->GetKey(i)->StringPrint(out);
    FPrintF(out, ": ");
    switch (transitions->GetTargetDetails(i).type()) {
      case FIELD: {
        FPrintF(out, " (transition to field)\n");
        break;
      }
      case CONSTANT_FUNCTION:
        FPrintF(out, " (transition to constant function)\n");
        break;
      case CALLBACKS:
        FPrintF(out, " (transition to callback)\n");
        break;
      // Values below are never in the target descriptor array.
      case NORMAL:
      case HANDLER:
      case INTERCEPTOR:
      case TRANSITION:
      case NONEXISTENT:
        UNREACHABLE();
        break;
    }
  }
}


void JSObject::JSObjectPrint(FILE* out) {
  FPrintF(out, "%p: [JSObject]\n", reinterpret_cast<void*>(this));
  FPrintF(out, " - map = %p [", reinterpret_cast<void*>(map()));
  // Don't call GetElementsKind, its validation code can cause the printer to
  // fail when debugging.
  PrintElementsKind(out, this->map()->elements_kind());
  FPrintF(out,
         "]\n - prototype = %p\n",
         reinterpret_cast<void*>(GetPrototype()));
  FPrintF(out, " {\n");
  PrintProperties(out);
  PrintTransitions(out);
  PrintElements(out);
  FPrintF(out, " }\n");
}


void JSModule::JSModulePrint(FILE* out) {
  HeapObject::PrintHeader(out, "JSModule");
  FPrintF(out, " - map = 0x%p\n", reinterpret_cast<void*>(map()));
  FPrintF(out, " - context = ");
  context()->Print(out);
  FPrintF(out, " - scope_info = ");
  scope_info()->ShortPrint(out);
  PrintElementsKind(out, this->map()->elements_kind());
  FPrintF(out, " {\n");
  PrintProperties(out);
  PrintElements(out);
  FPrintF(out, " }\n");
}


static const char* TypeToString(InstanceType type) {
  switch (type) {
    case INVALID_TYPE: return "INVALID";
    case MAP_TYPE: return "MAP";
    case HEAP_NUMBER_TYPE: return "HEAP_NUMBER";
    case SYMBOL_TYPE: return "SYMBOL";
    case ASCII_SYMBOL_TYPE: return "ASCII_SYMBOL";
    case CONS_SYMBOL_TYPE: return "CONS_SYMBOL";
    case CONS_ASCII_SYMBOL_TYPE: return "CONS_ASCII_SYMBOL";
    case EXTERNAL_ASCII_SYMBOL_TYPE:
    case EXTERNAL_SYMBOL_WITH_ASCII_DATA_TYPE:
    case EXTERNAL_SYMBOL_TYPE: return "EXTERNAL_SYMBOL";
    case SHORT_EXTERNAL_ASCII_SYMBOL_TYPE:
    case SHORT_EXTERNAL_SYMBOL_WITH_ASCII_DATA_TYPE:
    case SHORT_EXTERNAL_SYMBOL_TYPE: return "SHORT_EXTERNAL_SYMBOL";
    case ASCII_STRING_TYPE: return "ASCII_STRING";
    case STRING_TYPE: return "TWO_BYTE_STRING";
    case CONS_STRING_TYPE:
    case CONS_ASCII_STRING_TYPE: return "CONS_STRING";
    case EXTERNAL_ASCII_STRING_TYPE:
    case EXTERNAL_STRING_WITH_ASCII_DATA_TYPE:
    case EXTERNAL_STRING_TYPE: return "EXTERNAL_STRING";
    case SHORT_EXTERNAL_ASCII_STRING_TYPE:
    case SHORT_EXTERNAL_STRING_WITH_ASCII_DATA_TYPE:
    case SHORT_EXTERNAL_STRING_TYPE: return "SHORT_EXTERNAL_STRING";
    case FIXED_ARRAY_TYPE: return "FIXED_ARRAY";
    case BYTE_ARRAY_TYPE: return "BYTE_ARRAY";
    case FREE_SPACE_TYPE: return "FREE_SPACE";
    case EXTERNAL_PIXEL_ARRAY_TYPE: return "EXTERNAL_PIXEL_ARRAY";
    case EXTERNAL_BYTE_ARRAY_TYPE: return "EXTERNAL_BYTE_ARRAY";
    case EXTERNAL_UNSIGNED_BYTE_ARRAY_TYPE:
      return "EXTERNAL_UNSIGNED_BYTE_ARRAY";
    case EXTERNAL_SHORT_ARRAY_TYPE: return "EXTERNAL_SHORT_ARRAY";
    case EXTERNAL_UNSIGNED_SHORT_ARRAY_TYPE:
      return "EXTERNAL_UNSIGNED_SHORT_ARRAY";
    case EXTERNAL_INT_ARRAY_TYPE: return "EXTERNAL_INT_ARRAY";
    case EXTERNAL_UNSIGNED_INT_ARRAY_TYPE:
      return "EXTERNAL_UNSIGNED_INT_ARRAY";
    case EXTERNAL_FLOAT_ARRAY_TYPE: return "EXTERNAL_FLOAT_ARRAY";
    case EXTERNAL_DOUBLE_ARRAY_TYPE: return "EXTERNAL_DOUBLE_ARRAY";
    case FILLER_TYPE: return "FILLER";
    case JS_OBJECT_TYPE: return "JS_OBJECT";
    case JS_CONTEXT_EXTENSION_OBJECT_TYPE: return "JS_CONTEXT_EXTENSION_OBJECT";
    case ODDBALL_TYPE: return "ODDBALL";
    case JS_GLOBAL_PROPERTY_CELL_TYPE: return "JS_GLOBAL_PROPERTY_CELL";
    case SHARED_FUNCTION_INFO_TYPE: return "SHARED_FUNCTION_INFO";
    case JS_MODULE_TYPE: return "JS_MODULE";
    case JS_FUNCTION_TYPE: return "JS_FUNCTION";
    case CODE_TYPE: return "CODE";
    case JS_ARRAY_TYPE: return "JS_ARRAY";
    case JS_PROXY_TYPE: return "JS_PROXY";
    case JS_WEAK_MAP_TYPE: return "JS_WEAK_MAP";
    case JS_REGEXP_TYPE: return "JS_REGEXP";
    case JS_VALUE_TYPE: return "JS_VALUE";
    case JS_GLOBAL_OBJECT_TYPE: return "JS_GLOBAL_OBJECT";
    case JS_BUILTINS_OBJECT_TYPE: return "JS_BUILTINS_OBJECT";
    case JS_GLOBAL_PROXY_TYPE: return "JS_GLOBAL_PROXY";
    case FOREIGN_TYPE: return "FOREIGN";
    case JS_MESSAGE_OBJECT_TYPE: return "JS_MESSAGE_OBJECT_TYPE";
#define MAKE_STRUCT_CASE(NAME, Name, name) case NAME##_TYPE: return #NAME;
  STRUCT_LIST(MAKE_STRUCT_CASE)
#undef MAKE_STRUCT_CASE
    default: return "UNKNOWN";
  }
}


void Map::MapPrint(FILE* out) {
  HeapObject::PrintHeader(out, "Map");
  FPrintF(out, " - type: %s\n", TypeToString(instance_type()));
  FPrintF(out, " - instance size: %d\n", instance_size());
  FPrintF(out, " - inobject properties: %d\n", inobject_properties());
  FPrintF(out, " - elements kind: ");
  PrintElementsKind(out, elements_kind());
  FPrintF(out, "\n - pre-allocated property fields: %d\n",
      pre_allocated_property_fields());
  FPrintF(out, " - unused property fields: %d\n", unused_property_fields());
  if (is_hidden_prototype()) {
    FPrintF(out, " - hidden_prototype\n");
  }
  if (has_named_interceptor()) {
    FPrintF(out, " - named_interceptor\n");
  }
  if (has_indexed_interceptor()) {
    FPrintF(out, " - indexed_interceptor\n");
  }
  if (is_undetectable()) {
    FPrintF(out, " - undetectable\n");
  }
  if (has_instance_call_handler()) {
    FPrintF(out, " - instance_call_handler\n");
  }
  if (is_access_check_needed()) {
    FPrintF(out, " - access_check_needed\n");
  }
  FPrintF(out, " - back pointer: ");
  GetBackPointer()->ShortPrint(out);
  FPrintF(out, "\n - instance descriptors %i #%i: ",
         owns_descriptors(),
         NumberOfOwnDescriptors());
  instance_descriptors()->ShortPrint(out);
  if (HasTransitionArray()) {
    FPrintF(out, "\n - transitions: ");
    transitions()->ShortPrint(out);
  }
  FPrintF(out, "\n - prototype: ");
  prototype()->ShortPrint(out);
  FPrintF(out, "\n - constructor: ");
  constructor()->ShortPrint(out);
  FPrintF(out, "\n - code cache: ");
  code_cache()->ShortPrint(out);
  FPrintF(out, "\n");
}


void CodeCache::CodeCachePrint(FILE* out) {
  HeapObject::PrintHeader(out, "CodeCache");
  FPrintF(out, "\n - default_cache: ");
  default_cache()->ShortPrint(out);
  FPrintF(out, "\n - normal_type_cache: ");
  normal_type_cache()->ShortPrint(out);
}


void PolymorphicCodeCache::PolymorphicCodeCachePrint(FILE* out) {
  HeapObject::PrintHeader(out, "PolymorphicCodeCache");
  FPrintF(out, "\n - cache: ");
  cache()->ShortPrint(out);
}


void TypeFeedbackInfo::TypeFeedbackInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "TypeFeedbackInfo");
  FPrintF(out, " - ic_total_count: %d, ic_with_type_info_count: %d\n",
         ic_total_count(), ic_with_type_info_count());
  FPrintF(out, " - type_feedback_cells: ");
  type_feedback_cells()->FixedArrayPrint(out);
}


void AliasedArgumentsEntry::AliasedArgumentsEntryPrint(FILE* out) {
  HeapObject::PrintHeader(out, "AliasedArgumentsEntry");
  FPrintF(out, "\n - aliased_context_slot: %d", aliased_context_slot());
}


void FixedArray::FixedArrayPrint(FILE* out) {
  HeapObject::PrintHeader(out, "FixedArray");
  FPrintF(out, " - length: %d", length());
  for (int i = 0; i < length(); i++) {
    FPrintF(out, "\n  [%d]: ", i);
    get(i)->ShortPrint(out);
  }
  FPrintF(out, "\n");
}


void FixedDoubleArray::FixedDoubleArrayPrint(FILE* out) {
  HeapObject::PrintHeader(out, "FixedDoubleArray");
  FPrintF(out, " - length: %d", length());
  for (int i = 0; i < length(); i++) {
    if (is_the_hole(i)) {
      FPrintF(out, "\n  [%d]: <the hole>", i);
    } else {
      FPrintF(out, "\n  [%d]: %g", i, get_scalar(i));
    }
  }
  FPrintF(out, "\n");
}


void JSValue::JSValuePrint(FILE* out) {
  HeapObject::PrintHeader(out, "ValueObject");
  value()->Print(out);
}


void JSMessageObject::JSMessageObjectPrint(FILE* out) {
  HeapObject::PrintHeader(out, "JSMessageObject");
  FPrintF(out, " - type: ");
  type()->ShortPrint(out);
  FPrintF(out, "\n - arguments: ");
  arguments()->ShortPrint(out);
  FPrintF(out, "\n - start_position: %d", start_position());
  FPrintF(out, "\n - end_position: %d", end_position());
  FPrintF(out, "\n - script: ");
  script()->ShortPrint(out);
  FPrintF(out, "\n - stack_trace: ");
  stack_trace()->ShortPrint(out);
  FPrintF(out, "\n - stack_frames: ");
  stack_frames()->ShortPrint(out);
  FPrintF(out, "\n");
}


void String::StringPrint(FILE* out) {
  if (StringShape(this).IsSymbol()) {
    FPrintF(out, "#");
  } else if (StringShape(this).IsCons()) {
    FPrintF(out, "c\"");
  } else {
    FPrintF(out, "\"");
  }

  const char truncated_epilogue[] = "...<truncated>";
  int len = length();
  if (!FLAG_use_verbose_printer) {
    if (len > 100) {
      len = 100 - sizeof(truncated_epilogue);
    }
  }
  for (int i = 0; i < len; i++) {
    FPrintF(out, "%c", Get(i));
  }
  if (len != length()) {
    FPrintF(out, "%s", truncated_epilogue);
  }

  if (!StringShape(this).IsSymbol()) FPrintF(out, "\"");
}


// This method is only meant to be called from gdb for debugging purposes.
// Since the string can also be in two-byte encoding, non-ASCII characters
// will be ignored in the output.
char* String::ToAsciiArray() {
  // Static so that subsequent calls frees previously allocated space.
  // This also means that previous results will be overwritten.
  static char* buffer = NULL;
  if (buffer != NULL) free(buffer);
  buffer = new char[length()+1];
  WriteToFlat(this, buffer, 0, length());
  buffer[length()] = 0;
  return buffer;
}


static const char* const weekdays[] = {
  "???", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

void JSDate::JSDatePrint(FILE* out) {
  HeapObject::PrintHeader(out, "JSDate");
  FPrintF(out, " - map = 0x%p\n", reinterpret_cast<void*>(map()));
  FPrintF(out, " - value = ");
  value()->Print(out);
  if (!year()->IsSmi()) {
    FPrintF(out, " - time = NaN\n");
  } else {
    FPrintF(out, " - time = %s %04d/%02d/%02d %02d:%02d:%02d\n",
           weekdays[weekday()->IsSmi() ? Smi::cast(weekday())->value() + 1 : 0],
           year()->IsSmi() ? Smi::cast(year())->value() : -1,
           month()->IsSmi() ? Smi::cast(month())->value() : -1,
           day()->IsSmi() ? Smi::cast(day())->value() : -1,
           hour()->IsSmi() ? Smi::cast(hour())->value() : -1,
           min()->IsSmi() ? Smi::cast(min())->value() : -1,
           sec()->IsSmi() ? Smi::cast(sec())->value() : -1);
  }
}


void JSProxy::JSProxyPrint(FILE* out) {
  HeapObject::PrintHeader(out, "JSProxy");
  FPrintF(out, " - map = 0x%p\n", reinterpret_cast<void*>(map()));
  FPrintF(out, " - handler = ");
  handler()->Print(out);
  FPrintF(out, " - hash = ");
  hash()->Print(out);
  FPrintF(out, "\n");
}


void JSFunctionProxy::JSFunctionProxyPrint(FILE* out) {
  HeapObject::PrintHeader(out, "JSFunctionProxy");
  FPrintF(out, " - map = 0x%p\n", reinterpret_cast<void*>(map()));
  FPrintF(out, " - handler = ");
  handler()->Print(out);
  FPrintF(out, " - call_trap = ");
  call_trap()->Print(out);
  FPrintF(out, " - construct_trap = ");
  construct_trap()->Print(out);
  FPrintF(out, "\n");
}


void JSWeakMap::JSWeakMapPrint(FILE* out) {
  HeapObject::PrintHeader(out, "JSWeakMap");
  FPrintF(out, " - map = 0x%p\n", reinterpret_cast<void*>(map()));
  FPrintF(out, " - table = ");
  table()->ShortPrint(out);
  FPrintF(out, "\n");
}


void JSFunction::JSFunctionPrint(FILE* out) {
  HeapObject::PrintHeader(out, "Function");
  FPrintF(out, " - map = 0x%p\n", reinterpret_cast<void*>(map()));
  FPrintF(out, " - initial_map = ");
  if (has_initial_map()) {
    initial_map()->ShortPrint(out);
  }
  FPrintF(out, "\n - shared_info = ");
  shared()->ShortPrint(out);
  FPrintF(out, "\n   - name = ");
  shared()->name()->Print(out);
  FPrintF(out, "\n - context = ");
  unchecked_context()->ShortPrint(out);
  FPrintF(out, "\n - literals = ");
  literals()->ShortPrint(out);
  FPrintF(out, "\n - code = ");
  code()->ShortPrint(out);
  FPrintF(out, "\n");

  PrintProperties(out);
  PrintElements(out);

  FPrintF(out, "\n");
}


void SharedFunctionInfo::SharedFunctionInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "SharedFunctionInfo");
  FPrintF(out, " - name: ");
  name()->ShortPrint(out);
  FPrintF(out, "\n - expected_nof_properties: %d", expected_nof_properties());
  FPrintF(out, "\n - instance class name = ");
  instance_class_name()->Print(out);
  FPrintF(out, "\n - code = ");
  code()->ShortPrint(out);
  if (HasSourceCode()) {
    FPrintF(out, "\n - source code = ");
    String* source = String::cast(Script::cast(script())->source());
    int start = start_position();
    int length = end_position() - start;
    SmartArrayPointer<char> source_string =
        source->ToCString(DISALLOW_NULLS,
                          FAST_STRING_TRAVERSAL,
                          start, length, NULL);
    FPrintF(out, "%s", *source_string);
  }
  // Script files are often large, hard to read.
  // FPrintF(out, "\n - script =");
  // script()->Print(out);
  FPrintF(out, "\n - function token position = %d", function_token_position());
  FPrintF(out, "\n - start position = %d", start_position());
  FPrintF(out, "\n - end position = %d", end_position());
  FPrintF(out, "\n - is expression = %d", is_expression());
  FPrintF(out, "\n - debug info = ");
  debug_info()->ShortPrint(out);
  FPrintF(out, "\n - length = %d", length());
  FPrintF(out, "\n - has_only_simple_this_property_assignments = %d",
         has_only_simple_this_property_assignments());
  FPrintF(out, "\n - this_property_assignments = ");
  this_property_assignments()->ShortPrint(out);
  FPrintF(out, "\n");
}


void JSGlobalProxy::JSGlobalProxyPrint(FILE* out) {
  FPrintF(out, "global_proxy ");
  JSObjectPrint(out);
  FPrintF(out, "native context : ");
  native_context()->ShortPrint(out);
  FPrintF(out, "\n");
}


void JSGlobalObject::JSGlobalObjectPrint(FILE* out) {
  FPrintF(out, "global ");
  JSObjectPrint(out);
  FPrintF(out, "native context : ");
  native_context()->ShortPrint(out);
  FPrintF(out, "\n");
}


void JSBuiltinsObject::JSBuiltinsObjectPrint(FILE* out) {
  FPrintF(out, "builtins ");
  JSObjectPrint(out);
}


void JSGlobalPropertyCell::JSGlobalPropertyCellPrint(FILE* out) {
  HeapObject::PrintHeader(out, "JSGlobalPropertyCell");
}


void Code::CodePrint(FILE* out) {
  HeapObject::PrintHeader(out, "Code");
#ifdef ENABLE_DISASSEMBLER
  if (FLAG_use_verbose_printer) {
    Disassemble(NULL, out);
  }
#endif
}


void Foreign::ForeignPrint(FILE* out) {
  FPrintF(out, "foreign address : %p", foreign_address());
}


void AccessorInfo::AccessorInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "AccessorInfo");
  FPrintF(out, "\n - getter: ");
  getter()->ShortPrint(out);
  FPrintF(out, "\n - setter: ");
  setter()->ShortPrint(out);
  FPrintF(out, "\n - name: ");
  name()->ShortPrint(out);
  FPrintF(out, "\n - data: ");
  data()->ShortPrint(out);
  FPrintF(out, "\n - flag: ");
  flag()->ShortPrint(out);
}


void AccessorPair::AccessorPairPrint(FILE* out) {
  HeapObject::PrintHeader(out, "AccessorPair");
  FPrintF(out, "\n - getter: ");
  getter()->ShortPrint(out);
  FPrintF(out, "\n - setter: ");
  setter()->ShortPrint(out);
}


void AccessCheckInfo::AccessCheckInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "AccessCheckInfo");
  FPrintF(out, "\n - named_callback: ");
  named_callback()->ShortPrint(out);
  FPrintF(out, "\n - indexed_callback: ");
  indexed_callback()->ShortPrint(out);
  FPrintF(out, "\n - data: ");
  data()->ShortPrint(out);
}


void InterceptorInfo::InterceptorInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "InterceptorInfo");
  FPrintF(out, "\n - getter: ");
  getter()->ShortPrint(out);
  FPrintF(out, "\n - setter: ");
  setter()->ShortPrint(out);
  FPrintF(out, "\n - query: ");
  query()->ShortPrint(out);
  FPrintF(out, "\n - deleter: ");
  deleter()->ShortPrint(out);
  FPrintF(out, "\n - enumerator: ");
  enumerator()->ShortPrint(out);
  FPrintF(out, "\n - data: ");
  data()->ShortPrint(out);
}


void CallHandlerInfo::CallHandlerInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "CallHandlerInfo");
  FPrintF(out, "\n - callback: ");
  callback()->ShortPrint(out);
  FPrintF(out, "\n - data: ");
  data()->ShortPrint(out);
  FPrintF(out, "\n - call_stub_cache: ");
}


void FunctionTemplateInfo::FunctionTemplateInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "FunctionTemplateInfo");
  FPrintF(out, "\n - class name: ");
  class_name()->ShortPrint(out);
  FPrintF(out, "\n - tag: ");
  tag()->ShortPrint(out);
  FPrintF(out, "\n - property_list: ");
  property_list()->ShortPrint(out);
  FPrintF(out, "\n - serial_number: ");
  serial_number()->ShortPrint(out);
  FPrintF(out, "\n - call_code: ");
  call_code()->ShortPrint(out);
  FPrintF(out, "\n - property_accessors: ");
  property_accessors()->ShortPrint(out);
  FPrintF(out, "\n - prototype_template: ");
  prototype_template()->ShortPrint(out);
  FPrintF(out, "\n - parent_template: ");
  parent_template()->ShortPrint(out);
  FPrintF(out, "\n - named_property_handler: ");
  named_property_handler()->ShortPrint(out);
  FPrintF(out, "\n - indexed_property_handler: ");
  indexed_property_handler()->ShortPrint(out);
  FPrintF(out, "\n - instance_template: ");
  instance_template()->ShortPrint(out);
  FPrintF(out, "\n - signature: ");
  signature()->ShortPrint(out);
  FPrintF(out, "\n - access_check_info: ");
  access_check_info()->ShortPrint(out);
  FPrintF(out, "\n - hidden_prototype: %s",
         hidden_prototype() ? "true" : "false");
  FPrintF(out, "\n - undetectable: %s", undetectable() ? "true" : "false");
  FPrintF(out, "\n - need_access_check: %s",
         needs_access_check() ? "true" : "false");
}


void ObjectTemplateInfo::ObjectTemplateInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "ObjectTemplateInfo");
  FPrintF(out, " - tag: ");
  tag()->ShortPrint(out);
  FPrintF(out, "\n - property_list: ");
  property_list()->ShortPrint(out);
  FPrintF(out, "\n - constructor: ");
  constructor()->ShortPrint(out);
  FPrintF(out, "\n - internal_field_count: ");
  internal_field_count()->ShortPrint(out);
  FPrintF(out, "\n");
}


void SignatureInfo::SignatureInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "SignatureInfo");
  FPrintF(out, "\n - receiver: ");
  receiver()->ShortPrint(out);
  FPrintF(out, "\n - args: ");
  args()->ShortPrint(out);
}


void TypeSwitchInfo::TypeSwitchInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "TypeSwitchInfo");
  FPrintF(out, "\n - types: ");
  types()->ShortPrint(out);
}


void Script::ScriptPrint(FILE* out) {
  HeapObject::PrintHeader(out, "Script");
  FPrintF(out, "\n - source: ");
  source()->ShortPrint(out);
  FPrintF(out, "\n - name: ");
  name()->ShortPrint(out);
  FPrintF(out, "\n - line_offset: ");
  line_offset()->ShortPrint(out);
  FPrintF(out, "\n - column_offset: ");
  column_offset()->ShortPrint(out);
  FPrintF(out, "\n - type: ");
  type()->ShortPrint(out);
  FPrintF(out, "\n - id: ");
  id()->ShortPrint(out);
  FPrintF(out, "\n - data: ");
  data()->ShortPrint(out);
  FPrintF(out, "\n - context data: ");
  context_data()->ShortPrint(out);
  FPrintF(out, "\n - wrapper: ");
  wrapper()->ShortPrint(out);
  FPrintF(out, "\n - compilation type: ");
  compilation_type()->ShortPrint(out);
  FPrintF(out, "\n - line ends: ");
  line_ends()->ShortPrint(out);
  FPrintF(out, "\n - eval from shared: ");
  eval_from_shared()->ShortPrint(out);
  FPrintF(out, "\n - eval from instructions offset: ");
  eval_from_instructions_offset()->ShortPrint(out);
  FPrintF(out, "\n");
}


#ifdef ENABLE_DEBUGGER_SUPPORT
void DebugInfo::DebugInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "DebugInfo");
  FPrintF(out, "\n - shared: ");
  shared()->ShortPrint(out);
  FPrintF(out, "\n - original_code: ");
  original_code()->ShortPrint(out);
  FPrintF(out, "\n - code: ");
  code()->ShortPrint(out);
  FPrintF(out, "\n - break_points: ");
  break_points()->Print(out);
}


void BreakPointInfo::BreakPointInfoPrint(FILE* out) {
  HeapObject::PrintHeader(out, "BreakPointInfo");
  FPrintF(out, "\n - code_position: %d", code_position()->value());
  FPrintF(out, "\n - source_position: %d", source_position()->value());
  FPrintF(out, "\n - statement_position: %d", statement_position()->value());
  FPrintF(out, "\n - break_point_objects: ");
  break_point_objects()->ShortPrint(out);
}
#endif  // ENABLE_DEBUGGER_SUPPORT


void DescriptorArray::PrintDescriptors(FILE* out) {
  FPrintF(out, "Descriptor array  %d\n", number_of_descriptors());
  for (int i = 0; i < number_of_descriptors(); i++) {
    FPrintF(out, " %d: ", i);
    Descriptor desc;
    Get(i, &desc);
    desc.Print(out);
  }
  FPrintF(out, "\n");
}


void TransitionArray::PrintTransitions(FILE* out) {
  FPrintF(out, "Transition array  %d\n", number_of_transitions());
  for (int i = 0; i < number_of_transitions(); i++) {
    FPrintF(out, " %d: ", i);
    GetKey(i)->StringPrint(out);
    FPrintF(out, ": ");
    switch (GetTargetDetails(i).type()) {
      case FIELD: {
        FPrintF(out, " (transition to field)\n");
        break;
      }
      case CONSTANT_FUNCTION:
        FPrintF(out, " (transition to constant function)\n");
        break;
      case CALLBACKS:
        FPrintF(out, " (transition to callback)\n");
        break;
      // Values below are never in the target descriptor array.
      case NORMAL:
      case HANDLER:
      case INTERCEPTOR:
      case TRANSITION:
      case NONEXISTENT:
        UNREACHABLE();
        break;
    }
  }
  FPrintF(out, "\n");
}


#endif  // OBJECT_PRINT


} }  // namespace v8::internal
