    ROOTFLAGS     = $(shell root-config --cflags)
    ROOTLIBS      = $(shell root-config --glibs)
    TARGET        = drsspectrum
    CC            = gcc
    CXX           = g++
    LINK          = g++
    DELFILE       = @rm -f
    MKDIR         = mkdir
    OBJDIR        = ./obj
    BINDIR        = ./bin
    SODIR         = ./SO
    OBJS          = $(OBJDIR)/drsread.o $(OBJDIR)/drssignalproc.o $(OBJDIR)/drsspectrumproc.o

    TARGETLOC     = $(BINDIR)/$(TARGET)
    
    LIBS          = $(ROOTLIBS)
    RELEASE_FLAGS = -MD $(ROOTFLAGS) -O2
    DEBUG_FLAGS   = -MD $(ROOTFLAGS) -O0 -g -DDEBUG -Wall

#    CXXFLAGS      = $(ROOTFLAGS) -MD
#    CXXLIBS       = $(ROOTLIBS)
#    ROOTSO        = totdataread_cpp.so

first: release

release: CXXFLAGS=  $(RELEASE_FLAGS)
release: all

debug: CXXFLAGS= $(DEBUG_FLAGS)
debug: all

all: $(TARGETLOC)

$(TARGETLOC): $(OBJS) $(OBJDIR)/main.o
	@if ! [ -d $(BINDIR) ] ; then $(MKDIR) $(BINDIR) ; fi
	$(LINK) $(OBJDIR)/main.o $(OBJS)  $(LIBS) -o $(TARGETLOC)
	
####### compile
$(OBJDIR)/%.o: %.cpp %.h
	@if ! [ -d $(OBJDIR) ] ; then $(MKDIR) $(OBJDIR) ; fi
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/main.o: main.cpp
	@if ! [ -d $(OBJDIR) ] ; then $(MKDIR) $(OBJDIR) ; fi
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean: FORCE
	$(DELFILE) ./SO/*.so ./SO/*.so* ./bin/*so ./obj/*.o *.h~ *.cpp~ *.d
	
remake: clean first
remake_debug: clean debug

FORCE:
