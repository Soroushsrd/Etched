BUILD_DIR := build
.PHONY: all build test clean rebuild

all: build

build:
	cmake -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build
