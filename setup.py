from setuptools import setup, Extension
import pybind11
from pybind11.setup_helpers import Pybind11Extension, build_ext
import os

ext_modules = [
    Pybind11Extension(
        "behavior_tree_cpp",
        ["src/behavior_tree_wrapper.cpp"],
        include_dirs=[
            pybind11.get_include(),
            os.path.abspath("BehaviorTree.CPP/include")  # Ruta absoluta al directorio de include
        ],
        libraries=["behaviortree_cpp_v3"],
        library_dirs=[
            os.path.abspath("BehaviorTree.CPP/build")  # Ruta al directorio donde está la librería compilada
            # Si instalaste la librería, puedes usar "/usr/local/lib"
        ],
        runtime_library_dirs=[
            os.path.abspath("BehaviorTree.CPP/build")  # Necesario para encontrar la librería en tiempo de ejecución
            # O "/usr/local/lib" si instalaste la librería
        ],
        extra_compile_args=['-std=c++17'],
    ),
]

setup(
    name="behavior_tree_cpp",
    version="0.0.2",
    author="Jack",
    description="Wrapper de Python para BehaviorTree.CPP",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
