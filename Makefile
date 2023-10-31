
.PHONY: build configure test

build:
	cmake --build --preset "debug_build"

configure:
	cmake --preset "debug_cfg"

test: build
	ctest --preset "debug_utest"
