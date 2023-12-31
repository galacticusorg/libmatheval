Copyright (C) 1999, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011,
2012, 2013 Free Software Foundation, Inc.

Copying and distribution of this file, with or without modification, are
permitted in any medium without royalty provided the copyright notice
and this notice are preserved.


### WHAT IS IT?

GNU libmatheval is a library which contains several procedures that make
it possible to create an in-memory tree from the string representation
of a mathematical function over single or multiple variables. This tree
can be used later to evaluate a function for specified variable values,
to create a corresponding tree for the function derivative over a
specified variable, or to write a textual tree representation to a
specified string. The library exposes C and Fortran 77 interfaces.


### REQUIREMENTS

GNU autotools are employed for this project, so library should be
compilable on any UNIX variant.  Still, compilation is verified on
NetBSD 1.6 and Slackware GNU/Linux 9.1 only.  Library (actually, test
suite) requires Guile to be present.


### INSTALLATION

For detailed GNU libmatheval installation instructions, see INSTALL
document.  For impatient, simple sequence of:
```
  ./configure
  make
  make install
```
will do the job.


### DOCUMENTATION

Documentation for GNU libmatheval is available as a Texinfo manual.
After installation, issue command:
```
  info libmatheval
```
to access this document.


### BUGS

Please report bugs via GitHub [issues](https://github.com/galacticusorg/libmatheval/issues). Patches should be submitted as GitHub pull requests.
