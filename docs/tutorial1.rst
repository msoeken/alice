Tutorial 1: A minimalistic example
==================================

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
