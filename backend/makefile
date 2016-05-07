MACHINE = $(shell uname -s)
SOURCES = src
BUILD   = build
DEVEXT  = -dev

.PHONY: all clean

FLAGS  = -ffast-math -Werror -Wall -Wextra -Wfloat-equal -Wshadow -Wcast-align -Wunreachable-code -Wunused-variable -std=c++11 -Isrc/
CFLAGS = 

ifeq ($(MACHINE), Linux) # Linux-specific setup
	LIBS = -pthread
	FLAGS += -DUNIX
else
ifeq ($(MACHINE), Darwin) # OSX specific setup
	LIBS = 
	FLAGS += -DUNIX
else
	# TODO: Windows "support"
endif
endif

MKDIR  = mkdir -p
RM     = rm -rf

MODE = release
BNRY = db_backend
SDRS = $(shell find $(SOURCES) -type d | xargs echo)
SRCS = $(filter-out %.inc.cpp,$(foreach d,$(SDRS),$(wildcard $(addprefix $(d)/*,.cpp))))
OBJS = $(patsubst %.cpp,%.o,$(addprefix $(BUILD)/$(MODE)/,$(SRCS)))
DEPS = $(OBJS:%.o=%.d)
DIRS = $(sort $(dir $(OBJS)))

ifdef DEBUG
	BNRY  := $(BNRY)$(DEVEXT)
	FLAGS += -g -DDEBUG_MODE
	MODE   = debug
else
	FLAGS += -O3
endif



all: $(BNRY)

clean:
	$(RM) $(BUILD) $(BNRY) $(BNRY)$(DEVEXT)


$(DIRS):
	$(MKDIR) $@

$(BNRY): $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $@ $(LIBS)

$(OBJS): | $(DIRS)

$(BUILD)/$(MODE)/%.o: %.cpp
	$(CXX) $(FLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

run: all
	@touch users posts follows
	./$(BNRY)

-include $(DEPS)
