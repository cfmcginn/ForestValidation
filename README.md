# ForestValidation
Validation code for updates to forest to reduce possible introduction of bugs

To build do "make"
To clean do "make clean"

Dependencies: ROOT and gcc

Built with ROOT version 6.10/09 and gcc 6.3.0

If working with CMSSW, guarenteed to build with cmsenv in CMSSW_10_0_5

To run, after build do:

./bin/runForestDQM.exe <inFileName1> <inFileName2>

Output will be to directory "pdfDir", set of date-tagged pdf's and two tex files, main and plot file.

Build tex with preferred method, with all corresponding pdf files in same directory. Will produce set of Tex slides for validation.