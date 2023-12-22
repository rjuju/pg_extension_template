PostgreSQL Extension Template
=============================

This repository contains a template to quickly start writing PostgreSQL
extensions.

You will find:

- a template C source file
- declarations for the most useful hooks, with compatibility macros down to
  PostgreSQL 10
- a template SQL C SRF
- a template SQL extension
- a template Makefile with some useful rules
- a template META.json for releasing on pgxn
- a template for basic regression tests

All you need to do is:
-  rename and modify any occurence of "pg_extension_template" with your
   extension name
- modify any occurence of "pget" with a suitable short name
- modify the FIXME and other boilerplate parts
- remove any part you don't need
- and start hacking!
