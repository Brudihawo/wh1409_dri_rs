.DEFAULT_GOAL := parse_text

CMAKE_GENERATOR ?= $(shell (command -v ninja > /dev/null 2>&1 && echo "Ninja") ||\
										 echo "Unix Makefiles")

BUILD_DIR := ./build

clean:
	@echo "Removing build directory"
	if [ -d "$(BUILD_DIR)" ]; then\
		rm -fr $(BUILD_DIR);\
	fi

parse_text:
	if [ ! -d "$(BUILD_DIR)" ]; then\
		make run_cmake;\
	fi
ifeq (${CMAKE_GENERATOR}, "Unix Makefiles")
	if [ ! -f "$(BUILD_DIR)/Makefile" ]; then make run_cmake; fi
	make -C $(BUILD_DIR) -j$(nproc)
else
	if [ ! -f "$(BUILD_DIR)/build.ninja" ]; then make run_cmake; fi
	ninja -C $(BUILD_DIR)
endif

run_cmake: clean
	@echo "=========[[Using "$(CMAKE_GENERATOR)"]]========="
	cmake -G $(CMAKE_GENERATOR) -B $(BUILD_DIR) -S .


all:
	run_cmake
	parse_text
	# usbmon
