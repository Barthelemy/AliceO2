## Doxygen - Instructions

### Generating Doxygen documentation

This directory contains the necessary files to automatically generate documentation for the
AliceO2 project using Doxygen. To create the documentation, set the flag -DBUILD_DOXYGEN=ON
when calling cmake; the doxygen documentation will then be generated when calling make.
The generated html files can be found in the "[build]/doc/doxygen/html" subdirectory of the build directory.
It is installed in share/o2/html.

<!---
Doxygen documantation is also available online [here](http://aliceo2group.github.io/AliceO2/)
-->

###Guidelines

- In your modules, you can create markdown files (mydoc.md) to present your code.
They will appear in github.
Set a meaningful title in the document as it will also be used by doxygen. Do it this way :
`# My big title at the top of file` (avoid "README"...)
- One can also use doxygen files. Its syntax allows more complex features.
- Keep documentation files and images in the "doc" subdirectory of the modules.
- The output should be automatically pushed to the branch gh-pages.

### TODO

- Move all documentation (READMEs in particular) to the doc directory ?
Or if there is only README we keep it at the top of the module ?
- Commit the generated documentation to branch gh-pages
- Link to gh-pages from top README