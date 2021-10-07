SHELL=/usr/bin/bash
.DEFAULT_GOAL := parse_text
.SILENT: clean

CMAKE_BUILD_TYPE ?= RELEASE
CMAKE_GENERATOR ?= $(shell (command -v ninja > /dev/null 2>&1 && echo "Ninja") ||\
										 echo "Unix Makefiles")

BUILD_DIR := ./build

define prepare_build
	if [ ! -d "$(BUILD_DIR)" ] || ( [ ! -f "$(BUILD_DIR)/Makefile" ] && [ ! -f "$(BUILD_DIR)/build.ninja" ] ); then \
		make run_cmake;\
	fi
endef

define build_cmd
	@echo "=========[[Building $@]]========="
	if [[ "$(CMAKE_GENERATOR)" != "Unix Makefiles" ]]; then \
		ninja -C $(BUILD_DIR) $@; \
	else \
		make -C $(BUILD_DIR) -j$(shell nproc) $@; \
	fi
endef

clean:
	@echo "=========[[Removing Build Directory]]========="
	if [ -d "$(BUILD_DIR)" ]; then\
		rm -fr $(BUILD_DIR);\
	fi

run_cmake: clean
	@echo "=========[[Using "$(CMAKE_GENERATOR)"]]========="
	cmake -G "$(CMAKE_GENERATOR)" -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

parse_text:
	$(call prepare_build)
	$(call build_cmd parse_text)

usbmon:
	$(call prepare_build)
	$(call build_cmd usbmon)

all: run_cmake parse_text usbmon
