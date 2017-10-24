project('dae2obm', 'cpp', default_options: ['cpp_std=c++1z', 'warning_level=3'])

subdir('lib/tinyxml2')

executable('dae2obm', 'src/dae2obm.cxx', include_directories: include_directories('inc'), dependencies: tinyxml2_dep)
