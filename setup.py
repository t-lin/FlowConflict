import os
from distutils.core import setup, Extension

os.environ['CC'] = 'g++ -std=c++0x'

module1 = Extension('tlintest',
                    libraries = ['bitscan'],
                    #library_dirs = ['/usr/local/lib'],
                    #extra_objects = ['bitscan'],
                    #extra_compile_args = ['-O3'],
                    #extra_link_args = ['-O3'],
                    depends = ['FlowRecords.cpp'],
                    sources = ['tlintestmodule.cpp'])

setup (name = 'PackageName',
       version = '1.0',
       description = 'This is a demo package',
       ext_modules = [module1])
