CMAKE_BUILD_TYPE ?= RelWithDebInfo

BUILD_DIR := build
INSTALL_DIR := dist

.PHONY: all build install test clean

all: build

build:
	@cmake -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake --build $(BUILD_DIR) --parallel
	@ln -sf $(BUILD_DIR)/compile_commands.json .

install:
	@cmake --install $(BUILD_DIR)

test: build
	@ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	@rm -rf $(INSTALL_DIR)
	@rm -rf $(BUILD_DIR)
	@rm -f compile_commands.json
