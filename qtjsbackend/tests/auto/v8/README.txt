The v8 tests are actually implemented in v8test.[h|cpp].  There are also QtTest
(tst_v8.cpp) and non-Qt (v8main.cpp) stubs provided to run these tests.  This
is done to allow the tests to be run both in the Qt CI system, and manually
without a build of Qt.  The latter is necessary to run them against more exotic
build of V8, like the ARM simulator.

To build the non-Qt version of the tests, first build a debug or release V8
library under src/3rdparty/v8 using gyp build.

For example:
make x64.release

NOTE: If you get error message like "/bin/sh: build/gyp/gyp: not found" first use:
make dependencies

For detailed description about gyp build, see:
http://code.google.com/p/v8/wiki/BuildingWithGYP

After the successful build use Makefile.nonqt under tests/auto/v8 for testing.

For example:
make -f Makefile.nonqt x64.release
./v8test_x64_release

The following targets are available for non-Qt testing:
   ia32-release: Build the tests with -O2 -m32 and link against libv8_base and
                 libv8_snapshot.
   ia32-debug: Build the tests with -g -m32 and link against libv8_base and
               libv8_snapshot.
   x64-release: Build the tests with -O2 and link against libv8_base and
                libv8_snapshot.
   x64-debug: Build the tests with -g and link against libv8_base and
              libv8_snapshot.
   mips-release: Build the tests with -O2 -m32 and link against libv8_base and
                 libv8_nosnapshot.
   mips-debug: Build the tests with -g and link against libv8_base and
               libv8_nosnapshot.
   arm-release: Build the tests with -O2 -m32 and link against libv8_base and
                libv8_nosnapshot.
   arm-debug: Build the tests with -g and link against libv8_base and
              libv8_nosnapshot.

NOTE: It is necessary to build the corresponding V8 libraries for each target. For
example the arm simulator testing in release mode (arm-release) needs:
make arm.release
