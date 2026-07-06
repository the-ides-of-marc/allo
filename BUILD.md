# BUILD

While the underlying project is built with CMake,
there is a Makefile that helps build the project.

```shell

# BUILD_DIR is the build directory that defaults to ./build/
# INSTALL_DIR is the directory that defaults to ./dist/
# ENABLE_SANITIZERS flag is for enabling sanitizers (ON | OFF), defaults to OFF.
# CMAKE_BUILD_TYPE follows cmake's expected values, defaults to Debug.

# configures cmake.
make configure

# builds the project.
make build

# installs the project.
make install

# runs tests.
make test

# cleans the project.
make clean
```
