from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import os

class R2D_Conan(ConanFile):
    name = "R2D"
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

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

        if self.settings.os == "Windows":
            # Determine where the executable will be (e.g., build/Release)
            bindir = os.path.join(self.build_folder, str(self.settings.build_type))
            
            for dep in self.dependencies.values():
                # Check if the dependency actually has a bin directory defined
                if dep.cpp_info.bindirs:
                    source_dir = dep.cpp_info.bindirs[0]
                    # Only attempt copy if the directory exists on the system
                    if os.path.exists(source_dir):
                        copy(self, "*.dll", source_dir, bindir)
