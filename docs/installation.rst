Installation
============

alice is a header-only C++-14 library.  Just add the include directory
of alice to your include directories, and you can integrate alice into
your source files using

.. code-block:: c++

   #include <alice/alice.hpp>

Compile with readline
---------------------

Alice can use the *readline* library to enable command completition and history.
If one integrates alice through *CMake*, then *readline* is enabled by default.
Otherwise, make sure to define ``READLINE_USE_READLINE`` and link against
*readline*.

Building examples
-----------------

In order to build the examples, you need to enable them. Run the following from
the base directory of alice::

  git submodule update --init --recursive
  mkdir build
  cd build
  cmake -DALICE_EXAMPLES=ON ..
  make

Building tests
--------------

In order to run the tests and the micro benchmarks, you need to enable tests in
CMake::

  git submodule update --init --recursive
  mkdir build
  cd build
  cmake -DALICE_TEST=ON ..
  make
  ./test/run_tests
