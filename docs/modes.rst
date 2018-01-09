Console vs. Python mode
=======================

Alice supports both a console mode and a Python mode.

- **Console mode:** the source code is compiled as **executable** and a
  **stand-alone program** is offering a console shell interface, which accepts
  commands.

- **Python mode:** the source code is compiled as **Python module** which offers
  an API according to the commands.  Commands become function names, and command
  arguments become function arguments.

Which mode is taken is determined by compilation.  No changes in the source code
are necessary, when making use of the :ref:`macroapi`.

Let's say we have a source file `shell.cpp`, which defines an Alice CLI as
explained in the tutorials of this manual.  Then add the following lines into a
CMakeLists.txt file in order to compile it as an executable in console mode::

    add_executable(shell shell.cpp)
    target_link_libraries(shell alice)

The same file can be compiled as a Python module with the following command::

    add_alice_python_module(shell shell.cpp)

.. note::

    The name of the Python module *must* equal the name of the prefix that was
    used in the :c:macro:`ALICE_MAIN` macro.  Our example file program.cpp must
    finish with ``ALICE_MAIN(program)``.

Shell commands as Python functions
----------------------------------

If the ALICE shell has the prefix ``shell``, then the corresponding Python
module has the name `shell` and can be imported as follows:

.. code-block:: python

   import shell

Commands are mapped into Python functions with the same name.  Assume there is a
command called ``command``, then one can call it from Python as follows:

.. code-block:: python

   import shell
   shell.command()

Long option and flag names are mapped into keyword arguments of the
corresponding Python command.  Assume that the command ``command`` has the
following synopsis::

    shell> command -h
    A test command
    Usage: command [OPTIONS]

    Options:
      -h,--help                   Print this help message and exit
      -s,--sopt TEXT              A string option
      -n,--nopt INT               An integer option
      -f,--flag                   A flag

Then the individual arguments in this command can be called in Python mode as
follows:

.. code-block:: python

   import shell
   shell.command(sopt = "Some text", nopt = 42, flag = True)

The order in which the keyword arguments are passed does not matter; also, not
all of them need to be provided.  Note again, that the short option and flag
names cannot be used in Python mode.  Also flags must be assigned a Boolean
value.  Assigning ``False`` to a flag argument is as omitting it.

The return value of a Python function corresponds to the logging output of the
corresponding command.  Each command can contribute to the log by implementing
the ``log()`` function.  It returns a JSON object.  The return value of the
function in Python mode can be considered as a Python ``dict``, in which the
entries correspond to the JSON object.

Assume that the example command ``command`` implements the following ``log()``
function:

.. code-block:: c++

   nlohmann::json log() const
   {
     return nlohmann::json({
       {"str", "Some string"},
       {"number", 42}
     });
   }

Then one can access these values from the return value of the Python function:

.. code-block:: python

   import shell
   r = shell.command()
   print(r["number"])    # Prints 42

