    ROOTFLAGS     = $(shell root-config --cflags)
    ROOTLIBS      = $(shell root-config --glibs)
    TARGET        = drsspectrum
    CC            = gcc
    CXX           = g++
    LINK          = g++
    DELFILE       = @rm -f
    OBJDIR        = ./obj
    BINDIR        = ./bin
    SODIR         = ./SO
    OBJS          = $(OBJDIR)/main.o $(OBJDIR)/drsread.o $(OBJDIR)/drssignalproc.o $(OBJDIR)/drsspectrumproc.o

    TARGETLOC     = $(BINDIR)/$(TARGET)
    CXXFLAGS      = -MD -DDEBUG $(ROOTFLAGS)
    LIBS          = $(ROOTLIBS)
#    CXXFLAGS      = $(ROOTFLAGS) -MD
#    CXXLIBS       = $(ROOTLIBS)
#    ROOTSO        = totdataread_cpp.so

first: all

all: $(TARGETLOC)

$(TARGETLOC): $(OBJS)
	@if ! [ -d $(BINDIR) ] ; then $(MKDIR) $(BINDIR) ; fi
	$(LINK) $(OBJS) $(LIBS) -o $(TARGETLOC)
	
####### compile
$(OBJDIR)/%.o: %.cpp
	@if ! [ -d $(OBJDIR) ] ; then $(MKDIR) $(OBJDIR) ; fi
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean: FORCE
	$(DELFILE) ./SO/*.so ./SO/*.so* ./bin/*so ./obj/*.o *.h~ *.cpp~ *.d

FORCE:
