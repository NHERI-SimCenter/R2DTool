from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import os

class QuoFEMConan(ConanFile):
    name = "quoFEM"
    version = "0.0.9"
    license = "BSD-3-Clause"
    author = "NHERI SimCenter"
    url = "https://github.com/NHERI-SimCenter/quoFEM"
    description = "NHERI SimCenter R2D Tool"
    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        self.requires("jansson/2.13.1")
        self.requires("zlib/1.3.1")
        self.requires("nlohmann_json/3.12.0")
        if self.settings.os != "Linux":
            self.requires("libcurl/8.12.1")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        # Generate toolchain + find_package config files        
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

        # The following solves the "DLL not found" error during local development
        if self.settings.os == "Windows":
            # For CMake, the binary usually ends up in build/Release or build/Debug
            bindir = os.path.join(self.build_folder, str(self.settings.build_type))
            for dep in self.dependencies.values():
                copy(self, "*.dll", dep.cpp_info.bindirs[0], bindir)
                
