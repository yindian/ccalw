CFLAGS = -Wall
LDFLAGS =
ifeq ($(DEBUG),)
CFLAGS += -O3
else
CFLAGS += -g
endif
ifneq ($(CROSS_COMPILE),)
CC  = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar
else
ifeq ($(shell uname -m 2>&1),x86_64)
CFLAGS += -m64
LDFLAGS += -m64
endif
endif

CC_V := $(shell LANG=C $(CC) -v 2>&1)

TARGETS = $(addprefix $(BIN_DIR)/,$(libccal_TARGETS))
SRC = $(libccal_SOURCES)
GENERATED =

libccal_BIN = ccal
libccal_TARGETS = libccal.a $(libccal_BIN)
libccal_CXXFLAGS = -DUSE_YEARCACHE
ccal_SRCS = lunaryear.cpp mphases.cpp htmlmonth.cpp psmonth.cpp moonphase.cpp \
	    misc.cpp solarterm.cpp tt2ut.cpp novas.cpp novascon.cpp nutation.cpp \
	    solsys3.cpp yearcache.cpp
libccal_a_SRC = $(addprefix ccal/, $(ccal_SRCS))
$(foreach prog,$(libccal_BIN),$(eval $(prog)_SRC = ccal/$(prog).cpp libccal.a))
libccal_SOURCES = $(filter-out %.a, $(libccal_a_SRC) $(foreach prog,$(libccal_BIN),$(value $(prog)_SRC)))

TARGETS += $(addprefix $(BIN_DIR)/,$(ccalw_TARGETS))
SRC += $(ccalw_SOURCES)
GENERATED += $(ccalw_GEN_HDR)

ccalw_TARGETS = ccalw
ccalw_CFLAGS = -Iwebview
ccalw_CXXFLAGS = -std=c++11 $(ccalw_CFLAGS)
ccalw_LDFLAGS =
ifneq ($(findstring mingw,$(CC_V)),)
ccalw_CFLAGS += -DWEBVIEW_WINAPI=1
ccalw_LDFLAGS += -lole32 -lcomctl32 -loleaut32 -luuid -mwindows
ccalw_LDFLAGS += -static
else ifeq ($(shell uname -s),Linux)
ccalw_CFLAGS += -DWEBVIEW_GTK=1 $(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.0)
ccalw_LDFLAGS += $(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.0)
else ifeq ($(shell uname -s),Darwin)
ccalw_CFLAGS += -DWEBVIEW_COCOA=1 -x objective-c
ccalw_LDFLAGS += -framework Cocoa -framework WebKit
endif
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
ccalw_SOURCES = $(call uniq,$(filter-out %.a, $(foreach prog,$(ccalw_TARGETS),$(value $(prog)_SRC))))
ccalw_SRC = ccalw.cpp webview.cpp libccal.a
ccalw_GEN_HDR = polyfill.h

DEP_DIR = deps
OBJ_DIR = objs
BIN_DIR = bin
src2obj = $(addprefix $(OBJ_DIR)/,$(filter %.o,$(patsubst %.c,%.c.o,$(1:.cpp=.cpp.o)))) $(addprefix $(BIN_DIR)/,$(filter %.a,$1)) $(filter-out %.c %.cpp %.a,$1)

ifneq ($(shell which gsed 2>/dev/null),)
SED = gsed
endif
SED ?= sed

ifeq ($(findstring mingw,$(CC_V)),)
DOT_EXE =
else
DOT_EXE = .exe

ORIG_TARGETS := $(TARGETS)
TARGETS = $(addsuffix $(DOT_EXE),$(filter-out %.a,$(ORIG_TARGETS))) $(filter %.a,$(ORIG_TARGETS))
endif

all: $(TARGETS)

.PHONY: clean
clean:
	rm -rf $(TARGETS) $(DEP_DIR) $(OBJ_DIR) $(GENERATED)

$(libccal_SOURCES:%.cpp=$(DEP_DIR)/%.cpp.d): CXXFLAGS += $(libccal_CXXFLAGS)
$(libccal_SOURCES:%.cpp=$(OBJ_DIR)/%.cpp.o): CXXFLAGS += $(libccal_CXXFLAGS)

$(filter %.d,$(ccalw_SOURCES:%.c=$(DEP_DIR)/%.c.d)): CFLAGS += $(ccalw_CFLAGS)
$(filter %.d,$(ccalw_SOURCES:%.cpp=$(DEP_DIR)/%.cpp.d)): CXXFLAGS += $(ccalw_CXXFLAGS)
$(filter %.d,$(ccalw_SOURCES:%.cpp=$(DEP_DIR)/%.cpp.d)): $(ccalw_GEN_HDR)
$(filter %.o,$(ccalw_SOURCES:%.c=$(OBJ_DIR)/%.c.o)): Makefile
$(filter %.o,$(ccalw_SOURCES:%.cpp=$(OBJ_DIR)/%.cpp.o)): Makefile
$(filter %.o,$(ccalw_SOURCES:%.c=$(OBJ_DIR)/%.c.o)): CFLAGS += $(ccalw_CFLAGS)
$(filter %.o,$(ccalw_SOURCES:%.cpp=$(OBJ_DIR)/%.cpp.o)): CXXFLAGS += $(ccalw_CXXFLAGS)

$(BIN_DIR)/libccal.a: $(call src2obj, $(libccal_a_SRC))
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

$(foreach prog,$(filter-out %.a,$(notdir $(if $(ORIG_TARGETS),$(ORIG_TARGETS),$(TARGETS)))),$(if $(filter libccal.a,$(value $(prog)_SRC)),$(BIN_DIR)/$(prog)$(DOT_EXE))): LDFLAGS += $(libccal_LDFLAGS)
$(foreach prog,$(filter-out %.a,$(notdir $(if $(ORIG_TARGETS),$(ORIG_TARGETS),$(TARGETS)))),$(if $(filter webview.%,$(value $(prog)_SRC)),$(BIN_DIR)/$(prog)$(DOT_EXE))): LDFLAGS += $(ccalw_LDFLAGS)
$(foreach prog,$(filter-out %.a,$(notdir $(if $(ORIG_TARGETS),$(ORIG_TARGETS),$(TARGETS)))),$(eval $(BIN_DIR)/$(prog)$(DOT_EXE): $(call src2obj, $(value $(prog)_SRC))))
$(filter-out %.a,$(TARGETS)):
	@mkdir -p $(@D)
	$(CXX) -o $@ $^ $(LDFLAGS)

polyfill.h: es5-polyfill/dist/polyfill.min.js
$(ccalw_GEN_HDR): %.h: Makefile
	echo "#pragma once" > $@
	echo "static const char *$(patsubst %.h,%,$(@F)) = R\"gen_hdr(" >> $@
	$(if $(filter-out Makefile,$^),cat $(filter-out Makefile,$^) >> $@)
	echo ")gen_hdr\";" >> $@

DEP = $(addprefix $(DEP_DIR)/,$(patsubst %.c,%.c.d,$(SRC:.cpp=.cpp.d)))
findsrc = $(if $(filter $1,$(SRC)),$1,$(notdir $1))
src2tgt = $(addprefix $(OBJ_DIR)/,$(patsubst %.c,%.c.o,$(patsubst %.cpp,%.cpp.o,$(call findsrc,$1))))
$(DEP_DIR)/%.c.d: %.c
	@mkdir -p $(@D)
	@$(CC) -MM $(CFLAGS) $< | $(SED) -n "H;$$ {g;s@.*:\(.*\)@$< := \$$\(wildcard\1\)\n$(call src2tgt,$<) $@: $$\($<\)@; p}" > $@
$(DEP_DIR)/%.cpp.d: %.cpp
	@mkdir -p $(@D)
	@$(CXX) -MM $(CXXFLAGS) $< | $(SED) -n "H;$$ {g;s@.*:\(.*\)@$< := \$$\(wildcard\1\)\n$(call src2tgt,$<) $@: $$\($<\)@; p}" > $@
ifeq ($(filter clean,$(MAKECMDGOALS)),)
-include $(DEP)
endif
$(OBJ_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $(CFLAGS) $<
$(OBJ_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $(CXXFLAGS) $<
