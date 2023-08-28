CXX   := g++
MKDIR := mkdir
RM    := rm
BINDIR   := bin
BUILDDIR := build

CFLAGS  += -Isrc/include -std=c++2a
LDFLAGS += -lpthread

all: $(BUILDDIR) $(BINDIR) $(BINDIR)/libsoundbag-httpserver.a $(BINDIR)/sample_server$(EXT)

$(BINDIR)/sample_server$(EXT): $(addprefix $(BUILDDIR)/,sample_main.o) $(BINDIR)/libsoundbag-httpserver.a
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/libsoundbag-httpserver.a: $(addprefix $(BUILDDIR)/,HttpServer.o HttpRequest.o HttpResponse.o HttpUtil.o)
	$(AR) r $@ $^

$(BUILDDIR):
	$(MKDIR) $@

$(BINDIR):
	$(MKDIR) $@

$(BUILDDIR)/%.o: src/%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $^

clean:
	$(RM) -r $(BUILDDIR) $(BINDIR)

.PHONY: clean
