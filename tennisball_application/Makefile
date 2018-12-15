#
# File:		Makefile
# Author:   Victor Santos (viic.santos@gmail.com)


PROJECT	= application

CXXFILES = $(wildcard ./*.cpp)

#CDEFS = DEBUG
CDEFS = 

INCPATH	= include /usr/local/include/opencv /usr/local/include/opencv2
LIBPATH = lib /usr/local/lib

DEPS = conf

LIBS = pthread m v4l2 opencv_core opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_calib3d opencv_objdetect opencv_contrib opencv_legacy opencv_flann $(DEPS)

ELF = $(PROJECT)
LST = $(PROJECT).lst

SFLAGS  = -Wall
CXXFLAGS = -mcpu=cortex-a8 -mtune=cortex-a8 -march=armv7-a -mfpu=neon -mfloat-abi=hard -funsafe-math-optimizations -fomit-frame-pointer -ffast-math -funroll-loops -funsafe-loop-optimizations  
LDFLAGS =

CXX     = arm-linux-gnueabihf-g++
LD      = arm-linux-gnueabihf-g++
AR      = ar
OBJCOPY = objcopy
OBJDUMP = objdump
STRIP =	  strip 

LIB_DIR = $(abspath lib)
INSTALL_DIR = ./bin
OFILES = $(CXXFILES:.cpp=.o)

CXXFLAGS := $(OPTIONS) $(CXXFLAGS) -Wall $(addprefix -D,$(CDEFS))

FLAGS_TO_PASS := $(FLAGS_TO_PASS)\
				 'CXX=$(CXX)'\
				 'LD=$(LD)'\
				 'AS=$(AS)'\
				 'AR=$(AR)'\
				 'OBJCOPY=$(OBJCOPY)'\
				 'OBJDUMP=$(OBJDUMP)'\
				 'STRIP=$(STRIP)'\
				 'CDEFS=$(CDEFS)'

build: deps app lst

deps:
	echo $(LIB_DIR)
	@for d in $(DEPS); do\
		if [ -f ./$$d/Makefile ]; then\
			(cd ./$$d && $(MAKE) $(FLAGS_TO_PASS) all) || exit 1;\
		fi;\
	done;\
	for d in $(DEPS); do\
		if [ -f ./$$d/Makefile ]; then\
			(cd ./$$d && $(MAKE) INSTALL_DIR=$(LIB_DIR) install)\
			|| exit 1;\
		fi;\
	done;

build-clean:
	@rm -fv $(OFILES)
	@rm -fv $(ELF) $(LST)

deps-clean:
	@for d in $(DEPS); do\
		if [ -f ./$$d/Makefile ]; then\
			(cd ./$$d && $(MAKE) $(FLAGS_TO_PASS) clean) || exit 1;\
		fi;\
	done

clean: build-clean deps-clean

app: $(ELF)

lst: $(LST)

clean:
	@rm -fv $(OFILES) $(PROJECT).lst $(PROJECT) $(PROJECT)

install: $(ELF)
	if [ ! -d $(INSTALL_DIR) ]; then\
		mkdir $(INSTALL_DIR) || exit 1;\
	fi;\
	cp $(ELF) $(INSTALL_DIR)

$(ELF): Makefile $(OFILES)
	$(LD) $(OPTIONS) $(LDFLAGS) $(addprefix -L,$(LIBPATH)) -o $@ $(OFILES) \
	$(addprefix -l,$(LIBS))

$(LST): $(ELF) Makefile
	$(OBJDUMP) -w -D -t -S -r -z $< | sed '/^[0-9,a-f]\{8\} .[ ]*d[f]\?.*$$/d' > $@

%.o : %.cpp Makefile
	$(CXX) $(OPTIONS) $(CXXFLAGS) $(addprefix -I,$(INCPATH)) -o $@ -c $<

