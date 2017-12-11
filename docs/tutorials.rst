Tutorials
=========

Tutorial 1: A minimalistic example
----------------------------------

This tutorial shows a minimal example, the barely minimum what needs to be
written in order to get an Alice shell.  The source files for this tutorial are
located in ``examples/tutorial1``.

.. literalinclude:: ../examples/tutorial1/tutorial1.cpp
    :language: c++
    :lines: 26-28

That's all!   Two lines of code suffice.  The first line includes the Alice
header ``alice/alice.hpp``.  In all use cases, this will be the only header that
needs to be included.  The second line calls ``ALICE_MAIN``, which takes as
argument a name for the shell.  Besides acting as the prompt, it will also be
used as a name for the Python library, if it is build.

Compile ``tutorial1.cpp`` and link it to the ``alice`` interface library; have a
look into ``examples/CMakeLists.txt`` to check the details.  Even though we only
wrote two lines of code, we already can do several things with the program. When
executing the program (it will be in ``build/examples/tutorial1``), we can enter
some commands to the prompt::

    tutorial1> help
    General commands:
     alias            help             quit

It shows that the shell has 3 commands: ``alias``, ``help``, and ``quit``.
Further information about each commands can be obtained by calling it with the
``-h`` flag. We'll get to ``alias`` later.  Command ``help`` lists all available
commands, and it also allows to search through the help texts of all commands.
Command ``quit`` quits the program.

Tutorial 2: Adding a store and writing a simple command
-------------------------------------------------------

We extend on the previous example and add a store to the shell.  A shell can
have several stores, each is indexed by its type.

.. literalinclude:: ../examples/tutorial2/tutorial2.cpp
    :language: c++
    :lines: 26-42

The macro ``ALICE_ADD_STORE`` registers a store for strings (using type
``std::string``).  The type is the first argument to the macro.  The other four
are used to build commands.  The values ``str`` and ``s`` are long and short
flag names, respectively, and will be used to select this type in several
store-related commands, e.g., ``print --str`` or ``print -s`` to print a string
to the terminal.  The last two arguments are a singular and plural name that is
used to generate help strings in store-related commands.  Let's have a look what
``help`` shows for this tutorial::

    tutorial2> help
    Generation commands:
     hello
    
    General commands:
     alias            convert          current          help
     print            ps               quit             store

First, we see two categories of commands, the first one (`Generation commands`)
listing the custom command ``hello``.  We'll get to that one in a bit.  There
are also several other general commands compared to the previous tutorial.
These are called store-related commands are as follows:

+-------------+---------------------------------------------------+
| ``convert`` | Converts a store element of one type into another |
+-------------+---------------------------------------------------+
| ``current`` | Changes the current store element                 |
+-------------+---------------------------------------------------+
| ``print``   | Prints the current store element                  |
+-------------+---------------------------------------------------+
| ``ps``      | Prints statistics about the current store element |
+-------------+---------------------------------------------------+
| ``store``   | Shows a summary of store elements                 |
+-------------+---------------------------------------------------+

