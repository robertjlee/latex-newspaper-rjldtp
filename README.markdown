LaTeX-Based Newspaper Typesetting System
========================================

Author: Robert J Lee

Status: Work in progress, multiple limitations still exist.


License
-------

The majority of this project is released under the GNU Affero GPL. See
the file LICENSE.

The exception to this are the files rjlnewsp4.dtx, rjlnewsp4.ins, and
the derived file rjlnewsp4.cls, which are distributed under the LPPL,
version 1.3c. See the file lppl-1-3c.tex.


Installation & Usage
--------------------

You will need:

* GNU Make
* LaTeX 2e
* A C++1y compiler

Running "make default" will generate documentation.


Limitations
-----------

This currently only generates single-page newspapers containing
rectangular articles.


Architecture
------------

The C++ program and the LaTeX document class work together.

The C++ program can act as a front-end, generates a .lay (layout) file
for your document, then typesets the newspaper. LaTeX is used twice;
once to size the articles without producing any output; and again to
produce the final document.

The C++ program has been designed to be easily extensible, to allow
experimentation with different page layout algorithms.

Once you have a .lay file, either from the C++ program, hand crafted,
or produced in some other way, you can then use the LaTeX document
class stand-alone to produce the document.
