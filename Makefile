CMAKE_BUILD_TYPE ?= Debug
ENABLE_SANITIZERS ?= ON

BUILD_DIR := build
INSTALL_DIR := dist

.PHONY: all configure build install test clean

all: build

configure:
	@cmake -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@ln -sf $(BUILD_DIR)/compile_commands.json .

build:
	@cmake --build $(BUILD_DIR) --parallel

install: build
	@cmake --install $(BUILD_DIR)

test: build
	@ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	@rm -rf $(INSTALL_DIR)
	@rm -rf $(BUILD_DIR)
	@rm -f compile_commands.json
