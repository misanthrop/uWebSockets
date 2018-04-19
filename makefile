name := uWebSockets
out ?= /tmp/$(name)
o ?= $(out)/lib
t ?= $(out)/.build/$(name)

arch ?= -march=native
warnings := -Wall -Wextra -Wnon-virtual-dtor -Wno-unused-parameter -Wno-missing-field-initializers
fixcpp := -std=c++14 $(warnings) -g -O3 -fno-threadsafe-statics
srcs := $(wildcard src/*.cpp)
objs := $(addprefix $t/,$(addsuffix .o,$(srcs)))
deps := $(addsuffix .d,$(objs))

all:: $o/lib$(name).a

clean::
	$(RM) $(objs) $o/lib$(name).a

$t/%.cpp.o: %.cpp makefile
	@mkdir -p $(@D)
	$(CXX) -MMD -MT $@ -MF $@.d $(arch) $(fixcpp) -c $< -o $@

$o/lib$(name).a: $(objs)
	@mkdir -p $(@D)
	$(AR) crs $@ $^

-include $(deps)
