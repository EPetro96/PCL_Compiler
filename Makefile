.PHONY: default clean distclean install uninstall

SRCDIR=src
INCDIR=include
LIBDIR=lib
BUILDDIR=build
BINDIR=bin
INC=-I./$(INCDIR)
INSTALLDIR=/usr/local/bin

CXX=g++
CXXFLAGS=-Wall -std=c++11 `llvm-config --cxxflags` $(INC)
CFLAGS=-w $(INC)
LDFLAGS=`llvm-config --ldflags --system-libs --libs all`
COMPILER=pclc

default: $(BINDIR)/pcl

$(BUILDDIR)/lexer.cpp: $(SRCDIR)/lexer.l
	mkdir -p $(BUILDDIR)
	flex -s -o $(BUILDDIR)/lexer.cpp $(SRCDIR)/lexer.l

$(BUILDDIR)/lexer.o: $(BUILDDIR)/lexer.cpp $(BUILDDIR)/parser.hpp $(INCDIR)/ast.hpp
	mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -w -o $@ -c $<

$(BUILDDIR)/parser.hpp $(BUILDDIR)/parser.cpp: $(SRCDIR)/parser.y
	mkdir -p $(BUILDDIR)
	bison -dv -o $(BUILDDIR)/parser.cpp $(SRCDIR)/parser.y

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<	

$(BINDIR)/pcl: $(BUILDDIR)/lexer.o $(BUILDDIR)/parser.o
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/pcl $^ $(LDFLAGS)

clean:
	$(RM) -rf $(BUILDDIR)

distclean: clean
	$(RM) -rf $(BINDIR)

install: $(BINDIR)/pcl $(LIBDIR)/lib.a
	mkdir -p $(INSTALLDIR)/bin
	mkdir -p $(INSTALLDIR)/lib
	cp $(BINDIR)/pcl $(INSTALLDIR)/bin
	cp $(LIBDIR)/lib.a $(INSTALLDIR)/lib
	cp $(COMPILER) $(INSTALLDIR)

uninstall:
	rm -f $(INSTALLDIR)/bin/pcl
	rm -f $(INSTALLDIR)/$(COMPILER)
	rmdir --ignore-fail-on-non-empty $(INSTALLDIR)/bin