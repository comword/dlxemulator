RELEASE_FLAGS = -Werror
WARNINGS = -Wall -Wextra
DEBUG = -g
VERSION = 0.0.1
TARGET = dlxemulator
BUILD_DIR = $(CURDIR)
SRC_DIR = src
ODIR = obj
OS  = $(shell uname -s)
OS_COMPILER := $(CXX)
OS_LINKER := $(CXX)
CXX = $(OS_COMPILER)
LD = $(OS_LINKER)
CXXFLAGS += -ffast-math
LDFLAGS += $(PROFILE)
ifdef CLANG
  ifeq ($(NATIVE), osx)
    OTHERS += -stdlib=libc++
  endif
  ifdef CCACHE
    CXX = ccache $(CROSS)clang++
    LD  = ccache $(CROSS)clang++
  else
    CXX = $(CROSS)clang++
    LD  = $(CROSS)clang++
  endif
  WARNINGS = -Wall -Wextra -Wno-switch -Wno-sign-compare -Wno-missing-braces -Wno-type-limits -Wno-narrowing
endif
OTHERS += --std=c++11
CXXFLAGS += $(WARNINGS) $(DEBUG) $(PROFILE) $(OTHERS) -MMD
ifeq ($(NATIVE), linux64)
  CXXFLAGS += -m64
  LDFLAGS += -m64
  TARGETSYSTEM=LINUX
else
  # Linux 32-bit
  ifeq ($(NATIVE), linux32)
    CXXFLAGS += -m32
    LDFLAGS += -m32
    TARGETSYSTEM=LINUX
  endif
endif
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
_OBJS = $(SOURCES:$(SRC_DIR)/%.cpp=%.o)
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
all: $(TARGET)
	@
$(TARGET): $(ODIR) $(OBJS)
	$(LD) $(W32FLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
$(ODIR):
	mkdir -p $(ODIR)
$(ODIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(DEFINES) $(CXXFLAGS) -c $< -o $@
clean:
	rm -rf $(TARGET)
	rm -rf $(ODIR)

