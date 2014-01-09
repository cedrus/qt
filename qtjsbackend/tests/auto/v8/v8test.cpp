/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "v8test.h"
#ifndef NONQT_TESTING
#include <private/qcalculatehash_p.h>
#endif

using namespace v8;

#define BEGINTEST() bool _testPassed = true;
#define ENDTEST() return _testPassed;

#define VERIFY(expr) { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL: %s:%d %s\n", __FILE__, __LINE__, # expr); \
        _testPassed = false; \
        goto cleanup; \
    }  \
}

static inline Local<Value> CompileRun(const char* source)
{
  return Script::Compile(String::New(source))->Run();
}

struct MyStringResource : public String::ExternalAsciiStringResource
{
    static bool wasDestroyed;
    virtual ~MyStringResource() { wasDestroyed = true; }
    virtual const char* data() const { return "v8test"; }
    virtual size_t length() const { return 6; }
};
bool MyStringResource::wasDestroyed = false;

struct MyResource : public Object::ExternalResource
{
    static bool wasDestroyed;
    virtual ~MyResource() { wasDestroyed = true; }
};
bool MyResource::wasDestroyed = false;

bool v8test_externalteardown()
{
    BEGINTEST();

    Isolate *isolate = v8::Isolate::New();
    isolate->Enter();

    {
        HandleScope handle_scope;
        Persistent<Context> context = Context::New();
        Context::Scope context_scope(context);

        String::NewExternal(new MyStringResource);

        Local<FunctionTemplate> ft = FunctionTemplate::New();
        ft->InstanceTemplate()->SetHasExternalResource(true);

        Local<Object> obj = ft->GetFunction()->NewInstance();
        obj->SetExternalResource(new MyResource);

        context.Dispose();
    }

    // while (!v8::V8::IdleNotification()) ;
    isolate->Exit();
    isolate->Dispose();

    // ExternalString resources aren't guaranteed to be freed by v8 at this
    // point. Uncommenting the IdleNotification() line above helps.
//    VERIFY(MyStringResource::wasDestroyed);

    VERIFY(MyResource::wasDestroyed);

cleanup:

    ENDTEST();
}

bool v8test_eval()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<Object> qmlglobal = Object::New();
    qmlglobal->Set(String::New("a"), Integer::New(1922));

    Local<Script> script = Script::Compile(String::New("eval(\"a\")"), NULL, NULL,
                                           Handle<String>(), Script::QmlMode);

    TryCatch tc;
    Local<Value> result = script->Run(qmlglobal);

    VERIFY(!tc.HasCaught());
    VERIFY(result->Int32Value() == 1922);

cleanup:
    context.Dispose();

    ENDTEST();
}

bool v8test_globalcall()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<Object> qmlglobal = Object::New();

#define SOURCE "function func1() { return 1; }\n" \
               "function func2() { var sum = 0; for (var ii = 0; ii < 10000000; ++ii) { sum += func1(); } return sum; }\n" \
               "func2();"

    Local<Script> script = Script::Compile(String::New(SOURCE), NULL, NULL,
                                           Handle<String>(), Script::QmlMode);
    Local<Value> result = script->Run(qmlglobal);
    VERIFY(!result.IsEmpty());
    VERIFY(result->IsInt32());
    VERIFY(result->Int32Value() == 10000000);

#undef SOURCE

cleanup:
    context.Dispose();

    ENDTEST();
}

bool v8test_evalwithinwith()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<Object> qmlglobal = Object::New();
    qmlglobal->Set(String::New("a"), Integer::New(1922));
    // There was a bug that the "eval" lookup would incorrectly resolve
    // to the QML global object
    qmlglobal->Set(String::New("eval"), Integer::New(1922));

#define SOURCE \
    "(function() { " \
    "    var b = { c: 10 }; " \
    "    with (b) { " \
    "        return eval(\"a\"); " \
    "    } " \
    "})"
    Local<Script> script = Script::Compile(String::New(SOURCE), NULL, NULL, 
                                           Handle<String>(), Script::QmlMode);
#undef SOURCE

    TryCatch tc;
    Local<Value> result = script->Run(qmlglobal);

    VERIFY(!tc.HasCaught());
    VERIFY(result->IsFunction());

    {
    Local<Value> fresult = Handle<Function>::Cast(result)->Call(context->Global(), 0, 0);
    VERIFY(!tc.HasCaught());
    VERIFY(fresult->Int32Value() == 1922);
    }

cleanup:
    context.Dispose();

    ENDTEST();
}

static int userObjectComparisonCalled = 0;
static bool userObjectComparisonReturn = false;
static Local<Object> expectedLhs;
static Local<Object> expectedRhs;
static bool expectedObjectsCompared = false;

#define SET_EXPECTED(lhs, rhs) { \
    expectedObjectsCompared = false; \
    expectedLhs = lhs; \
    expectedRhs = rhs; \
}

static bool UserObjectComparison(Local<Object> lhs, Local<Object> rhs)
{
    userObjectComparisonCalled++;

    expectedObjectsCompared = (lhs == expectedLhs && rhs == expectedRhs);

    return userObjectComparisonReturn;
}

inline bool runscript(const char *source) {
    Local<Script> script = Script::Compile(String::New(source));
    Local<Value> result = script->Run();
    return result->BooleanValue();
}

bool v8test_userobjectcompare()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    V8::SetUserObjectComparisonCallbackFunction(UserObjectComparison);

    Local<ObjectTemplate> ot = ObjectTemplate::New();
    ot->MarkAsUseUserObjectComparison();

    Local<Object> uoc1 = ot->NewInstance();
    Local<Object> uoc2 = ot->NewInstance();
    context->Global()->Set(String::New("uoc1a"), uoc1);
    context->Global()->Set(String::New("uoc1b"), uoc1);
    context->Global()->Set(String::New("uoc2"), uoc2);
    Local<Object> obj1 = Object::New();
    context->Global()->Set(String::New("obj1a"), obj1);
    context->Global()->Set(String::New("obj1b"), obj1);
    context->Global()->Set(String::New("obj2"), Object::New());
    Local<String> string1 = String::New("Hello World");
    context->Global()->Set(String::New("string1a"), string1);
    context->Global()->Set(String::New("string1b"), string1);
    context->Global()->Set(String::New("string2"), v8::String::New("Goodbye World"));

    // XXX Opportunity for optimization - don't invoke user callback if objects are
    // equal.
#if 0
    userObjectComparisonCalled = 0; userObjectComparisonReturn = false;
    VERIFY(true == runscript("uoc1a == uoc1b"));
    VERIFY(userObjectComparisonCalled == 0);
#endif

    // Comparing two uoc objects invokes uoc
    userObjectComparisonCalled = 0;
    userObjectComparisonReturn = false;
    VERIFY(false == runscript("uoc1a == uoc2"));
    VERIFY(userObjectComparisonCalled == 1);

    VERIFY(false == runscript("uoc2 == uoc1a"));
    VERIFY(userObjectComparisonCalled == 2);
    userObjectComparisonReturn = true;
    VERIFY(true == runscript("uoc1a == uoc2"));
    VERIFY(userObjectComparisonCalled == 3);
    VERIFY(true == runscript("uoc2 == uoc1a"));
    VERIFY(userObjectComparisonCalled == 4);

    // != on two uoc object invokes uoc
    userObjectComparisonCalled = 0;
    userObjectComparisonReturn = false;
    VERIFY(true == runscript("uoc1a != uoc2"));
    VERIFY(userObjectComparisonCalled == 1);
    VERIFY(true == runscript("uoc2 != uoc1a"));
    VERIFY(userObjectComparisonCalled == 2);
    userObjectComparisonReturn = true;
    VERIFY(false == runscript("uoc1a != uoc2"));
    VERIFY(userObjectComparisonCalled == 3);
    VERIFY(false == runscript("uoc2 != uoc1a"));
    VERIFY(userObjectComparisonCalled == 4);

    // Comparison against a non-object doesn't invoke uoc
    userObjectComparisonCalled = 0;
    userObjectComparisonReturn = false;
    VERIFY(false == runscript("uoc1a == string1a"));
    VERIFY(userObjectComparisonCalled == 0);
    VERIFY(false == runscript("string1a == uoc1a"));
    VERIFY(userObjectComparisonCalled == 0);
    VERIFY(false == runscript("2 == uoc1a"));
    VERIFY(userObjectComparisonCalled == 0);
    VERIFY(true == runscript("uoc1a != string1a"));
    VERIFY(userObjectComparisonCalled == 0);
    VERIFY(true == runscript("string1a != uoc1a"));
    VERIFY(userObjectComparisonCalled == 0);
    VERIFY(true == runscript("2 != uoc1a"));
    VERIFY(userObjectComparisonCalled == 0);

    // Comparison against a non-uoc-object still invokes uoc
    userObjectComparisonCalled = 0;
    userObjectComparisonReturn = false;
    VERIFY(false == runscript("uoc1a == obj1a"));
    VERIFY(userObjectComparisonCalled == 1);
    VERIFY(false == runscript("obj1a == uoc1a"));
    VERIFY(userObjectComparisonCalled == 2);
    userObjectComparisonReturn = true;
    VERIFY(true == runscript("uoc1a == obj1a"));
    VERIFY(userObjectComparisonCalled == 3);
    VERIFY(true == runscript("obj1a == uoc1a"));
    VERIFY(userObjectComparisonCalled == 4);

    // != comparison against a non-uoc-object still invokes uoc
    userObjectComparisonCalled = 0;
    userObjectComparisonReturn = false;
    VERIFY(true == runscript("uoc1a != obj1a"));
    VERIFY(userObjectComparisonCalled == 1);
    VERIFY(true == runscript("obj1a != uoc1a"));
    VERIFY(userObjectComparisonCalled == 2);
    userObjectComparisonReturn = true;
    VERIFY(false == runscript("uoc1a != obj1a"));
    VERIFY(userObjectComparisonCalled == 3);
    VERIFY(false == runscript("obj1a != uoc1a"));
    VERIFY(userObjectComparisonCalled == 4);

    // Comparing two non-uoc objects does not invoke uoc
    userObjectComparisonCalled = 0;
    userObjectComparisonReturn = false;
    VERIFY(true == runscript("obj1a == obj1a"));
    VERIFY(true == runscript("obj1a == obj1b"));
    VERIFY(false == runscript("obj1a == obj2"));
    VERIFY(false == runscript("obj1a == string1a"));
    VERIFY(true == runscript("string1a == string1a"));
    VERIFY(true == runscript("string1a == string1b"));
    VERIFY(false == runscript("string1a == string2"));
    VERIFY(userObjectComparisonCalled == 0);

    // Correct lhs and rhs passed to uoc
    userObjectComparisonCalled = 0;
    userObjectComparisonReturn = false;
    SET_EXPECTED(uoc1, uoc2);
    VERIFY(false == runscript("uoc1a == uoc2"));
    VERIFY(true == expectedObjectsCompared);
    SET_EXPECTED(uoc2, uoc1);
    VERIFY(false == runscript("uoc2 == uoc1a"));
    VERIFY(true == expectedObjectsCompared);
    SET_EXPECTED(uoc1, uoc2);
    VERIFY(true == runscript("uoc1a != uoc2"));
    VERIFY(true == expectedObjectsCompared);
    SET_EXPECTED(uoc2, uoc1);
    VERIFY(true == runscript("uoc2 != uoc1a"));
    VERIFY(true == expectedObjectsCompared);
    SET_EXPECTED(uoc1, obj1);
    VERIFY(false == runscript("uoc1a == obj1a"));
    VERIFY(true == expectedObjectsCompared);
    SET_EXPECTED(obj1, uoc1);
    VERIFY(false == runscript("obj1a == uoc1a"));
    VERIFY(true == expectedObjectsCompared);

cleanup:
    V8::SetUserObjectComparisonCallbackFunction(0);
    context.Dispose();

    ENDTEST();
}

#define VARNAME "tipli"
#define VARVALUE 28

Handle<Value> CheckQMLGlobal(const Arguments&)
{
  HandleScope handle_scope;

  Local<String> key = String::New(VARNAME);
  Local<Object> qmlglobal = Context::GetCallingQmlGlobal();

  if (qmlglobal.IsEmpty()) return Integer::New(0);

  int hash = qmlglobal->GetIdentityHash();
  int value = qmlglobal->Get(key)->Int32Value();

  return Integer::New(hash + value);
}

bool v8test_getcallingqmlglobal()
{
    BEGINTEST();

    HandleScope handle_scope;

    Local<ObjectTemplate> global = ObjectTemplate::New();
    global->Set(String::New("checkQMLGlobal"), FunctionTemplate::New(CheckQMLGlobal));

    Persistent<Context> context = Context::New(NULL, global);
    Context::Scope global_scope(context);

    Local<String> key = String::New(VARNAME);
    Local<Object> qmlglobal = Object::New();

    qmlglobal->Set(key, Integer::New(VARVALUE));
    int hash1 = qmlglobal->GetIdentityHash();

    Local<String> source = String::New("(function test() { return checkQMLGlobal(); })");
    Local<Script> script = Script::Compile(source, NULL, NULL, v8::Handle<v8::String>(), v8::Script::QmlMode);
    Local<Value> result = script->Run(qmlglobal);

    Local<Function> v8function = Local<Function>::Cast(result);
    int hash2 = v8function->Call(v8function, 0, 0)->Int32Value();
    VERIFY(hash2);
    VERIFY(hash1 == (hash2 - VARVALUE));

    qmlglobal = Context::GetCallingQmlGlobal();
    VERIFY(qmlglobal.IsEmpty());

cleanup:
    context.Dispose();

    ENDTEST();
}

#undef VARNAME
#undef VARVALUE

bool v8test_typeof()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<Object> qmlglobal = Object::New();
    qmlglobal->Set(String::New("a"), Integer::New(123));

    Local<Script> script = Script::Compile(String::New("["
                                                       "typeof a === 'number', "
                                                       "typeof b === 'undefined', "
                                                       "(function() { return typeof c === 'undefined'; })()"
                                                       "]"), NULL, NULL,
                                           Handle<String>(), Script::QmlMode);

    TryCatch tc;
    Local<Value> result = script->Run(qmlglobal);

    VERIFY(!tc.HasCaught());
    VERIFY(result->IsArray());
    VERIFY(v8::Array::Cast(*result)->Length() == 3);
    VERIFY(v8::Array::Cast(*result)->Get(0)->IsTrue());
    VERIFY(v8::Array::Cast(*result)->Get(1)->IsTrue());
    VERIFY(v8::Array::Cast(*result)->Get(2)->IsTrue());

cleanup:
    context.Dispose();

    ENDTEST();
}

bool v8test_referenceerror()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<Object> qmlglobal = Object::New();

    Local<Script> script = Script::Compile(String::New("a"), NULL, NULL,
                                           Handle<String>(), Script::QmlMode);

    TryCatch tc;
    Local<Value> result = script->Run(qmlglobal);

    VERIFY(tc.HasCaught());
    VERIFY(result.IsEmpty());
    VERIFY(tc.Exception()->IsError());
    VERIFY(tc.Exception()->ToString()->Equals(v8::String::New("ReferenceError: a is not defined")));

cleanup:
    context.Dispose();

    ENDTEST();
}

bool v8test_qtbug_24871()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<Object> qmlglobal = Object::New();

    Local<Script> script = Script::Compile(String::New(
                                               // Create a bunch of properties to exceed kMaxFastProperties
                                               "var a1, a2, a3, a4, a5, a6, a7, a8;\n"
                                               "var b1, b2, b3, b4, b5, b6, b7, b8;\n"
                                               "var c1, c2, c3, c4, c5, c6, c7, c8;\n"
                                               "var d1, d2, d3, d4, d5, d6, d7, d8;\n"
                                               "function index(a) { return a + 1; }\n"
                                               "function init() {\n"
                                               "  for (var i = 0; i < 300; ++i)\n"
                                               "    index(i);\n"
                                               "}\n"
                                               "init();"), NULL, NULL,
                                           Handle<String>(), Script::QmlMode);

    TryCatch tc;
    Local<Value> result = script->Run(qmlglobal);

    VERIFY(!tc.HasCaught());
    VERIFY(result->IsUndefined());

cleanup:
    context.Dispose();

    ENDTEST();
}

#define DATA "fallbackpropertyhandler callbacks test"

namespace CallbackType {
    enum {
        Error,
        Getter,
        Setter,
        Query,
        Deleter,
        Enumerator
    };
}

bool checkInterceptorCalled(Local<Object> obj, int index)
{
    if (obj->GetInternalField(index)->IsTrue()) {
        obj->SetInternalField(index, False());
        return true;
    }

    return false;
}

Handle<Value> EmptyInterceptorGetter(Local<String> name,
                                     const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<String> data = String::New(DATA);

    if (name->IsUndefined()) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Value>();
    }

    if (!data->Equals(info.Data())) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Value>();
    }

    self->SetInternalField(CallbackType::Getter, True());
    return Handle<Value>();
}

Handle<Value> EmptyInterceptorSetter(Local<String> name,
                                     Local<Value> value,
                                     const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<String> data = String::New(DATA);

    if (name->IsUndefined() || value->IsUndefined()) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Value>();
    }

    if (!data->Equals(info.Data())) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Value>();
    }

    self->SetInternalField(CallbackType::Setter, True());
    return Handle<Value>();
}

Handle<Integer> EmptyInterceptorQuery(Local<String> name,
                                      const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<String> data = String::New(DATA);

    if (name->IsUndefined()) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Integer>();
    }

    if (!data->Equals(info.Data())) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Integer>();
    }

    self->SetInternalField(CallbackType::Query, True());
    return Handle<Integer>();
}

Handle<Boolean> EmptyInterceptorDeleter(Local<String> name,
                                        const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<String> data = String::New(DATA);

    if (name->IsUndefined()) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Boolean>();
    }

    if (!data->Equals(info.Data())) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Boolean>();
    }

    self->SetInternalField(CallbackType::Deleter, True());
    return Handle<Boolean>();
}

Handle<Array> EmptyInterceptorEnumerator(const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<String> data = String::New(DATA);

    if (!data->Equals(info.Data())) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Array>();
    }

    self->SetInternalField(CallbackType::Enumerator, True());
    return Handle<Array>();
}

// Check whether the callbacks of fallback-named-property interceptors
// called adequately.
bool v8test_fallbackpropertyhandler_callbacks()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Handle<ObjectTemplate> templ = ObjectTemplate::New();
    templ->SetFallbackPropertyHandler(EmptyInterceptorGetter,
                                      EmptyInterceptorSetter,
                                      EmptyInterceptorQuery,
                                      EmptyInterceptorDeleter,
                                      EmptyInterceptorEnumerator,
                                      String::New(DATA));
    templ->SetInternalFieldCount(6);
    Local<Object> obj = templ->NewInstance();
    obj->SetInternalField(CallbackType::Error, False());
    obj->SetInternalField(CallbackType::Getter, False());
    obj->SetInternalField(CallbackType::Setter, False());
    obj->SetInternalField(CallbackType::Query, False());
    obj->SetInternalField(CallbackType::Deleter, False());
    obj->SetInternalField(CallbackType::Enumerator, False());
    Local<Value> result;

    context->Global()->Set(String::New("obj"), obj);

    // Check Getter/Setter callbacks
    obj->Set(String::New("a"), Integer::New(28));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(checkInterceptorCalled(obj, CallbackType::Setter));

    result = obj->Get(String::New("a"));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Setter));
    VERIFY(28 == result->Int32Value());

    obj->Set(String::New("a"), Integer::New(28));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Setter));

    result = obj->Get(String::New("b"));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Setter));
    VERIFY(result->IsUndefined());

    obj->ForceSet(String::New("b"), Integer::New(42));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Setter));

    result = obj->Get(String::New("b"));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Setter));
    VERIFY(42 == result->Int32Value());

    result = CompileRun("obj.c = 47");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(checkInterceptorCalled(obj, CallbackType::Setter));
    VERIFY(47 == result->Int32Value());

    result = CompileRun("obj.d");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Setter));
    VERIFY(result->IsUndefined());

    result = CompileRun("obj.a");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Setter));
    VERIFY(28 == result->Int32Value());

    result = CompileRun("obj.b = 5");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Getter));
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Setter));
    VERIFY(5 == result->Int32Value());

    // Check query callback
    obj->HasOwnProperty(String::New("a"));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Query));

    result = CompileRun("'a' in obj");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Query));

    obj->HasOwnProperty(String::New("x"));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(checkInterceptorCalled(obj, CallbackType::Query));

    result = CompileRun("'x' in obj");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(checkInterceptorCalled(obj, CallbackType::Query));

    // Check deleter callback
    obj->Delete(String::New("a"));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Deleter));

    result = CompileRun("delete obj.b");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(!checkInterceptorCalled(obj, CallbackType::Deleter));

    obj->Delete(String::New("x"));
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(checkInterceptorCalled(obj, CallbackType::Deleter));

    result = CompileRun("delete obj.x");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(checkInterceptorCalled(obj, CallbackType::Deleter));

    // Check enumerator callback
    result = CompileRun("for (var p in obj) ;");
    VERIFY(obj->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(checkInterceptorCalled(obj, CallbackType::Enumerator));

cleanup:
    context.Dispose();

    ENDTEST();
}

#undef DATA

Handle<Object> bottom;
Handle<Object> middle;
Handle<Object> top;

Handle<Value> CheckThisFallbackPropertyHandler(Local<String> name,
                                               const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Object> holder = info.Holder();

    if (name->IsUndefined()) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Value>();
    }

    bool check = self->Equals(bottom);
    bottom->SetInternalField(CallbackType::Error, Boolean::New(!check));

    if (holder->Equals(bottom)) {
        bottom->SetInternalField(1, String::New("bottom"));
    } else if (holder->Equals(middle)) {
        bottom->SetInternalField(1, String::New("middle"));
    } else if (holder->Equals(top)) {
        bottom->SetInternalField(1, String::New("top"));
    } else {
        bottom->SetInternalField(1, String::New(""));
    }

    return Handle<Value>();
}

Handle<Value> CheckThisFallbackPropertySetter(Local<String> name,
                                              Local<Value> value,
                                              const AccessorInfo& info)
{
    Local<Object> self = info.This();
    Local<Object> holder = info.Holder();

    if (name->IsUndefined() || value->IsUndefined()) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Value>();
    }

    bool check = self->Equals(bottom);
    bottom->SetInternalField(CallbackType::Error, Boolean::New(!check));

    if (holder->Equals(bottom)) {
        bottom->SetInternalField(1, String::New("bottom"));
    } else if (holder->Equals(middle)) {
        bottom->SetInternalField(1, String::New("middle"));
    } else if (holder->Equals(top)) {
        bottom->SetInternalField(1, String::New("top"));
    } else {
        bottom->SetInternalField(1, String::New(""));
    }

    return Handle<Value>();
}

Handle<Integer> CheckThisFallbackPropertyQuery(Local<String> name,
                                               const AccessorInfo& info)
{
    Local<Object> self = info.This();

    if (name->IsUndefined()) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Integer>();
    }

    bool check = self->Equals(bottom);
    bottom->SetInternalField(CallbackType::Error, Boolean::New(!check));

    return Handle<Integer>();
}

Handle<Boolean> CheckThisFallbackPropertyDeleter(Local<String> name,
                                                 const AccessorInfo& info)
{
    Local<Object> self = info.This();

    if (name->IsUndefined()) {
        self->SetInternalField(CallbackType::Error, True());
        return Handle<Boolean>();
    }

    bool check = self->Equals(bottom);
    bottom->SetInternalField(CallbackType::Error, Boolean::New(!check));

    return Handle<Boolean>();
}

Handle<Array> CheckThisFallbackPropertyEnumerator(const AccessorInfo& info)
{
    bool check = info.This()->Equals(bottom);
    // Use field index 1 here due to query fallback interceptor can be called
    // by enumerator
    bottom->SetInternalField(1, Boolean::New(!check));

    return Handle<Array>();
}

bool v8test_fallbackpropertyhandler_in_prototype()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<String> bottom_name = String::New("bottom");
    Local<String> middle_name = String::New("middle");
    Local<String> top_name = String::New("top");

    // Set up a prototype chain with three interceptors.
    Handle<FunctionTemplate> templ = FunctionTemplate::New();
    templ->InstanceTemplate()->SetInternalFieldCount(2);
    templ->InstanceTemplate()->SetNamedPropertyHandler(
            CheckThisFallbackPropertyHandler,
            CheckThisFallbackPropertySetter,
            CheckThisFallbackPropertyQuery,
            CheckThisFallbackPropertyDeleter,
            CheckThisFallbackPropertyEnumerator);

    bottom = templ->GetFunction()->NewInstance();
    top = templ->GetFunction()->NewInstance();
    middle = templ->GetFunction()->NewInstance();

    bottom->Set(bottom_name, Integer::New(0));
    middle->Set(middle_name, Integer::New(0));
    top->Set(top_name, Integer::New(0));

    bottom->Set(String::New("__proto__"), middle);
    middle->Set(String::New("__proto__"), top);
    context->Global()->Set(String::New("obj"), bottom);

    // Set Error field to true in every case to check whether the interceptor is called.
    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("obj.x");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());

    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("obj.y = 42");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());

    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("'x' in obj");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());

    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("delete obj.x");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());

    bottom->SetInternalField(1, True());
    CompileRun("for (var p in obj) ;");
    // Check field index 1 here due to query fallback interceptor can be called
    // by enumerator
    VERIFY(bottom->GetInternalField(1)->IsFalse());

    bottom->SetInternalField(1, String::New(""));
    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("obj.bottom");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(bottom->GetInternalField(1)->ToString()->Equals(bottom_name));

    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("obj.middle");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(bottom->GetInternalField(1)->ToString()->Equals(middle_name));

    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("obj.top");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(bottom->GetInternalField(1)->ToString()->Equals(top_name));

    bottom->SetInternalField(1, String::New(""));
    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("obj.bottom = 1");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(bottom->GetInternalField(1)->ToString()->Equals(bottom_name));

    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("obj.cica = 1");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(bottom->GetInternalField(1)->ToString()->Equals(bottom_name));

    bottom->SetInternalField(CallbackType::Error, True());
    CompileRun("obj.top = 1");
    VERIFY(bottom->GetInternalField(CallbackType::Error)->IsFalse());
    VERIFY(bottom->GetInternalField(1)->ToString()->Equals(bottom_name));

cleanup:
    context.Dispose();

    ENDTEST();
}

Handle<Value> NonEmptyInterceptorGetter(Local<String> name,
                                        const AccessorInfo& info)
{
    (void)info;

    return name;
}

Handle<Value> NonEmptyInterceptorSetter(Local<String> name,
                                        Local<Value> value,
                                        const AccessorInfo& info)
{
    Local<Object> self = info.This();
    self->ForceSet(name, String::Concat(name, value->ToString()));

    return value;
}

bool v8test_fallbackpropertyhandler_nonempty()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Handle<ObjectTemplate> templ = ObjectTemplate::New();
    templ->SetFallbackPropertyHandler(NonEmptyInterceptorGetter,
                                      NonEmptyInterceptorSetter);
    Local<Object> obj = templ->NewInstance();
    Local<Value> result;

    context->Global()->Set(String::New("obj"), obj);

    result = obj->Get(String::New("a"));
    VERIFY(result->IsString());
    VERIFY(result->ToString()->Equals(String::New("a")));
    result = CompileRun("obj.a");
    VERIFY(result->IsString());
    VERIFY(result->ToString()->Equals(String::New("a")));

    obj->Set(String::New("a"), Integer::New(28));
    result = obj->Get(String::New("a"));
    VERIFY(result->IsString());
    VERIFY(result->ToString()->Equals(String::New("a28")));

    obj->Set(String::New("a"), Integer::New(42));
    result = obj->Get(String::New("a"));
    VERIFY(result->IsNumber());
    VERIFY(42 == result->Int32Value());

cleanup:
    context.Dispose();

    ENDTEST();
}

bool v8test_completehash()
{
#define HASH_EQUALS(str1, str2) \
{ \
    String::CompleteHashData hash1 = str1->CompleteHash(); \
    String::CompleteHashData hash2 = str2->CompleteHash(); \
    VERIFY(hash1.length == hash2.length); \
    VERIFY(hash1.symbol_id == hash2.symbol_id || (hash1.symbol_id == 0 || hash2.symbol_id == 0)); \
    VERIFY(hash1.hash == hash2.hash); \
};

    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<String> str;
    Local<String> str2;
    uint16_t input[] = { 'I', 'n', 'p', 'u', 't', 0 };

    str = String::New("Input");
    str2 = String::New("Input");
    HASH_EQUALS(str, str2);

    str = String::New(input);
    str2 = String::New(input);
    HASH_EQUALS(str, str2);

    str = String::NewSymbol("input");
    str2 = String::NewSymbol("input");
    HASH_EQUALS(str, str2);

    str = CompileRun("var input = \"{ \\\"abc\\\": \\\"value\\\" }\"; input")->ToString();
    str2 = CompileRun("var input2 = \"{ \\\"abc\\\": \\\"value\\\" }\"; input2")->ToString();
    HASH_EQUALS(str, str2);

    // SubStringAsciiSymbolKey is created via the built-in JSON parser for
    // property names.
    str = CompileRun("JSON.parse(input)")->ToObject()->GetOwnPropertyNames()->Get(0)->ToString();
    str2 = CompileRun("JSON.parse(input2)")->ToObject()->GetOwnPropertyNames()->Get(0)->ToString();
    HASH_EQUALS(str, str2);
cleanup:
    context.Dispose();

    ENDTEST();
}

#ifndef NONQT_TESTING
bool v8test_stringhashcomparison()
{
    BEGINTEST();

    // Initialize V8 random seed for string hashing
    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    quint32 hash1;
    uint32_t hash2;
    int length, rand;

    const char* text;
    QString qtext;

    char textRand[HashedString::kMaxHashCalcLength + 1];
    QString qtextRand;

    text = "tipli";
    qtext = QString(text);
    length = strlen(text);

    hash1 = calculateHash((uint8_t*)text, length) >> HashedString::kHashShift;
    hash2 = String::ComputeHash((char*)text, length);
    VERIFY(hash1 == hash2);

    hash1 = calculateHash<quint16>((quint16*)qtext.constData(), length) >> HashedString::kHashShift;
    hash2 = String::ComputeHash((uint16_t*)qtext.constData(), length);
    VERIFY(hash1 == hash2);

    // Check V8 trivial hash
    length = HashedString::kMaxHashCalcLength + 1;
    for (int i = 0; i < length; i++) {
        rand = qrand() % 255 + 1;
        textRand[i] = (char)rand;
    }
    qtextRand = QString(textRand);

    hash1 = calculateHash((uint8_t*)textRand, length) >> HashedString::kHashShift;
    hash2 = String::ComputeHash((char*)textRand, length);
    VERIFY(hash1 == hash2);

    hash1 = calculateHash<quint16>((quint16*)qtextRand.constData(), length) >> HashedString::kHashShift;
    hash2 = String::ComputeHash((uint16_t*)qtextRand.constData(), length);
    VERIFY(hash1 == hash2);

cleanup:
    context.Dispose();

    ENDTEST();
}
#endif

// Test whether the variables are declared in the appropriate scope
// when script is compiled in QML compilation mode.
bool v8test_qmlmodevariables()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<Object> global = context->Global();
    Local<Object> qmlglobal = Object::New();

    qmlglobal->Set(String::New("eval"), Integer::New(1922));
    qmlglobal->Set(String::New("b"), Integer::New(28));
    global->Set(String::New("x"), Integer::New(32));
    global->Set(String::New("y"), Integer::New(40));

    // Different declarations regarding to the binding kind.
    Local<String> source = String::New(
        "function f() { return 28; }" // function is bound at parse-time
        "var a = 42;"                 // bound variable declared in qmlglobal scope
        "eval(\"b\");"                // unbound variable declared in qmlglobal scope
        "const c = 28;"               // constant is bound at parse-time
        "var x = 2;"                  // bound variable declared in global scope
        "eval(\"y\");"                // unbound variable declared in global scope
      );
    Local<Script> script = Script::Compile(source, NULL, NULL, Handle<String>(), Script::QmlMode);

    TryCatch tc;
    script->Run(qmlglobal);
    VERIFY(!tc.HasCaught());

    // Check redeclaration of a global JS function.
    VERIFY(global->HasOwnProperty(String::New("eval")));
    VERIFY(qmlglobal->HasOwnProperty(String::New("eval")));

    // The following variables should be declared in the qmlglobal scope.
    VERIFY(!global->HasOwnProperty(String::New("f")));
    VERIFY(qmlglobal->HasOwnProperty(String::New("f")));

    VERIFY(!global->HasOwnProperty(String::New("a")));
    VERIFY(qmlglobal->HasOwnProperty(String::New("a")));

    VERIFY(!global->HasOwnProperty(String::New("b")));
    VERIFY(qmlglobal->HasOwnProperty(String::New("b")));

    VERIFY(!global->HasOwnProperty(String::New("c")));
    VERIFY(qmlglobal->HasOwnProperty(String::New("c")));

    // The following variables should be declared in the global scope.
    VERIFY(global->HasOwnProperty(String::New("x")));
    VERIFY(!qmlglobal->HasOwnProperty(String::New("x")));

    VERIFY(global->HasOwnProperty(String::New("y")));
    VERIFY(!qmlglobal->HasOwnProperty(String::New("y")));

cleanup:
    context.Dispose();

    ENDTEST();
}

// test for https://bugreports.qt-project.org/browse/QTBUG-31366
// assert/crash when inlining local functions in qml mode
bool v8test_qmlmodeinlinelocal()
{
    BEGINTEST();

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Local<Object> qmlglobal = Object::New();

    Local<String> source = String::New(
        "function func() {"
            "function local_function () {"
            "}"
            // high enough to get it to opt; 10000 seems to be too low
            "for (var i = 0; i < 100000; ++i) local_function();"
        "}"
        "func();"
      );

    Local<Script> script = Script::Compile(source, NULL, NULL, Handle<String>(), Script::QmlMode);

    TryCatch tc;
    script->Run(qmlglobal);
    VERIFY(!tc.HasCaught());

cleanup:
    context.Dispose();

    ENDTEST();
}
