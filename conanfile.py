import os

from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy


class ImGuiExample(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("fmt/11.0.2")
        self.requires("sdl/2.30.6")
        self.requires("glm/1.0.1")
        self.requires("imgui/1.92.5")

    def generate(self):
        copy(self, "*sdl2*", os.path.join(self.dependencies["imgui"].package_folder,
                                          "res", "bindings"), os.path.join(self.source_folder, "bindings"))
        copy(self, "*vulkan*", os.path.join(self.dependencies["imgui"].package_folder,
                                             "res", "bindings"), os.path.join(self.source_folder, "bindings"))

    def layout(self):
        cmake_layout(self)