Example 1
=========

[TOC]

# Introduction {#Introduction}

This is an example of a basic module using the O2 CMake macros to generate a library and an executable.

## Macros {#Macros}

- O2_SETUP : necessary to register the module in O2.
- O2_GENERATE_LIBRARY : use once and only once per module to generate a library.
- O2_GENERATE_EXECUTABLE : generate executables.

# Documentation {#Documentation}

The documentation is in markdown with special markers for doxygen such as `[TOC]`.
Note the that the TOC will work only if no levels are skip (don't create a subsection without a section
above it).
