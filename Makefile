#path var + dir commands
ROOT=`root-config --cflags --glibs`
CXX=g++
CXXFLAGS=-Wall -O2 -Wextra -Werror -Wno-unused-local-typedefs  -Werror -Wno-deprecated-declarations -std=c++11

ifeq "$(GCCVERSION)" "1"
  CXXFLAGS += -Wno-error=misleading-indentation
endif

MKDIR_BIN=mkdir -p $(PWD)/bin
MKDIR_PDF=mkdir -p $(PWD)/pdfDir

#programs to make

all: mkdirBin mkdirPdf bin/runForestDQM.exe bin/checkEventSync.exe bin/checkAllTreesReadable.exe bin/checkForestEventSelection.exe bin/photonTreeCountCheck.exe

mkdirBin:
	$(MKDIR_BIN)
mkdirPdf:
	$(MKDIR_PDF)
bin/runForestDQM.exe: src/runForestDQM.C
	$(CXX) $(CXXFLAGS) src/runForestDQM.C $(ROOT) -I $(PWD) -o bin/runForestDQM.exe 

bin/checkEventSync.exe: src/checkEventSync.C
	$(CXX) $(CXXFLAGS) src/checkEventSync.C $(ROOT) -I $(PWD) -o bin/checkEventSync.exe

bin/checkAllTreesReadable.exe: src/checkAllTreesReadable.C
	$(CXX) $(CXXFLAGS) src/checkAllTreesReadable.C $(ROOT) -I $(PWD) -o bin/checkAllTreesReadable.exe

bin/checkForestEventSelection.exe: src/checkForestEventSelection.C
	$(CXX) $(CXXFLAGS) src/checkForestEventSelection.C $(ROOT) -I $(PWD) -o bin/checkForestEventSelection.exe

bin/photonTreeCountCheck.exe: src/photonTreeCountCheck.C
	$(CXX) $(CXXFLAGS) src/photonTreeCountCheck.C $(ROOT) -I $(PWD) -o bin/photonTreeCountCheck.exe

clean:
	rm -f *~
	rm -f \#*.*#
	rm -f $(PWD)/include/#*.*#
	rm -f $(PWD)/include/*~
	rm -f $(PWD)/src/#*.*#
	rm -f $(PWD)/src/*~
	rm -f $(PWD)/bin/*.exe
	rmdir bin
.PHONY: all

