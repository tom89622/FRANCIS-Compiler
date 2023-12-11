# FRANCIS-Compiler

In developing a FANCIS compiler with C, I processed a revised Fortran language into intermediate code with quadruples form. Using nested For loops, I implemented a scanner and parser for syntax verification. Upon confirming the correct syntax, I stored input in tables based on type. Specifically, I recorded array names and subroutines in an identifier table for arrays, capturing essential details like addresses, real array status, dimensionality, and dimension sizes in an information table. Additionally, when handling assignments, I employed Reverse Polish Notation with a stack. For instance, the expression 'X = Y + U * V' was streamlined into 'XYUV*+=.' The compiler then generated intermediate code, referencing the stored variables and expression form. In conclusion, the FANCIS compiler, crafted in C, processes a modified Fortran language into intermediate code with quadruples form, showcasing organized input storage through tables and introducing an approach to assignment handling using Reverse Polish Notation.

Design handbook: 
[FRANCIS COMPILER.pdf](https://github.com/tom89622/FRANCIS-Compiler/files/13638288/FRANCIS.COMPILER.pdf)
