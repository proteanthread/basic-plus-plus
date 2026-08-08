from setuptools import setup, find_packages

setup(
    name="basicpp",
    version="6.5.2",
    description="Python C-FFI Bindings for the BASIC++ Modular Language Engine",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    author="BASIC++ Community",
    author_email="dev@basicplusplus.org",
    url="https://github.com/proteanthread/basic-plus-plus",
    packages=find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Topic :: Software Development :: Interpreters",
    ],
    python_requires=">=3.7",
)
