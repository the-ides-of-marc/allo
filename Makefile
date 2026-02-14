CMAKE_BUILD_TYPE ?= RelWithDebInfo

BUILD_DIR := build

.PHONY: all build test clean

all: build

build:
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake --build $(BUILD_DIR) --parallel
	@ln -sf $(BUILD_DIR)/compile_commands.json .

test: build
	@ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	@rm -rf $(INSTALL_DIR)
	@rm -rf $(BUILD_DIR)
	@rm -f compile_commands.json
