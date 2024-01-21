
.PHONY: build configure test clang-tidy

build:
	cmake --build --preset "debug_build"

configure:
	cmake --preset "debug_cfg" $(if $(SANITIZER), -DCMAKE_CXX_FLAGS="-fsanitize=$(SANITIZER)")

test: build
	ctest --preset "debug_utest"

clang-tidy:
	find src/ include/ \( -iname "*.hpp" -or -iname "*.cpp" \) | xargs clang-tidy -p build
