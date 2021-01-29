from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext
from pathlib import Path

class ivw_build_ext(build_ext):
    def build_extensions(self):
        build_ext.build_extensions(self)

setup(
    name="inviwo",
    description="Python package for Inviwo",
    long_description="long_description",
    long_description_content_type='text/markdown',
    version='0.9.1',
    license='MIT',
    packages=find_packages(),
    url='https://inviwo.org/',
    author='Inviwo Foundation',
    package_data=[],
    dependency_links=[],
    install_requires=['numpy']
)
