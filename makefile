    ROOTFLAGS     = $(shell root-config --cflags)
    #ROOTLIBS      = $(shell root-config --glibs)
    ROOTLIBS     = -L/usr/lib/x86_64-linux-gnu -lCore -lRIO -lHist -lGpad -lTree -lm -ldl
    TARGET        = bindrsspectrum
    CC            = gcc
    CXX           = g++
    LINK          = g++
    DELFILE       = @rm -f
    MKDIR         = mkdir
    OBJDIR        = ./obj
    BINDIR        = ./bin
    SODIR         = ./SO
    OBJS          = $(OBJDIR)/drsreadn.o $(OBJDIR)/drs4read.o $(OBJDIR)/drssignalprocn.o $(OBJDIR)/drs450read.o

    TARGETLOC     = $(BINDIR)/$(TARGET)
    
    LIBS          = $(ROOTLIBS)
    COM_FLAGS     = -std=c++11
    RELEASE_FLAGS = -MD $(ROOTFLAGS) -O2 $(COM_FLAGS)
    DEBUG_FLAGS   = -MD $(ROOTFLAGS) -O0 -g -DDEBUG -Wall $(COM_FLAGS)
    

first: release
#first: debug

release: CXXFLAGS=  $(RELEASE_FLAGS)
release: all

debug: CXXFLAGS= $(DEBUG_FLAGS)
debug: all

all: $(TARGETLOC)

$(TARGETLOC): $(OBJS) $(OBJDIR)/main.o
	@if ! [ -d $(BINDIR) ] ; then $(MKDIR) $(BINDIR) ; fi
	$(LINK) $(OBJDIR)/main.o $(OBJS)  $(LIBS) -o $(TARGETLOC)
	
####### compile
$(OBJDIR)/%.o: %.cpp %.h common.h
	@if ! [ -d $(OBJDIR) ] ; then $(MKDIR) $(OBJDIR) ; fi
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/drs4read.o: drs4read.cpp drs4read.h common.h $(OBJDIR)/drsreadn.o
	@if ! [ -d $(OBJDIR) ] ; then $(MKDIR) $(OBJDIR) ; fi
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/drs450read.o: drs450read.cpp drs450read.h common.h $(OBJDIR)/drsreadn.o
	@if ! [ -d $(OBJDIR) ] ; then $(MKDIR) $(OBJDIR) ; fi
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/main.o: main.cpp $(OBJS) common.h drstype.h
	@if ! [ -d $(OBJDIR) ] ; then $(MKDIR) $(OBJDIR) ; fi
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean: FORCE
	$(DELFILE) ./SO/*.so ./SO/*.so* ./bin/*so ./obj/*.o *.h~ *.cpp~ ./obj/*.d ./bin/$(TARGET)
	
install: insthomebin

insthomebin: first
	@cp $(BINDIR)/$(TARGET) $(HOME)/bin
	
remake: clean first
remake_debug: clean debug

FORCE:
