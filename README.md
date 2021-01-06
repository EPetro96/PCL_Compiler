# PCL_compiler

Evgenios Petropoulos  
Alexandros Sampanis  

(Important Note: Some PCL features are not supported by this compiler. Check "Known Issues" below for further explanation)  

A compiler for PCL, a simple imperative language. This was developed as a project for Compilers' class in NTUA.  

## Prequisities
The PCL compiler was developed and tested using:  
C++11  
bison (GNU Bison) 3.0.4  
flex 2.6.0  
llvm 7.0.1  
python 3.7  

## Installation Instructions:
1) in root directory: make  
2) make install (superuser privileges)  

## To Uninstall:
make uninstall

## Usage:
Compiler's executable is called "pclc". Running "pclc -h" yields the following usage instructions:  

usage: pclc [-h] [-O] [-x] [-i] [-f] [-o OUTNAME] [infile]  

PCLC - the PCL Somewhat Complete ( :( ) Compiler  

positional arguments:  
  infile         (if -f or -i not given) the PCL source code to compile  

optional arguments:  
  -h, --help     show this help message and exit  
  -O             optimize IR and final (assembly) code  
  -x, --no-dump  do not emit IR and assembly code in two seperate files  
                 (switched on by default)  
  -i             read source code from stdin, print IR code to stdout, then  
                 exit (no executable is produced)  
  -f             read source code from stdin, print final code to stdout, then  
                 exit (no executable is produced)  
  -o OUTNAME     (if -f or -i not given) the name of the produced executable  
                 (default: a.out)  
                 
## Known Issues
1) There is no support for real numbers (this was optional in the projects instructions)  
2) Function "readString" is not working  
3) Forward Declarations of Functions/Procedures do not work  
