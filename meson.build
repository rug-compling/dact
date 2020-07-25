project('dact', 'cpp',
  default_options: ['cpp_std=c++11'],
  version: '3.0.0')

cpp = meson.get_compiler('cpp')

alpinocorpus_dep = dependency('alpinocorpus')
libexslt_dep = dependency('libexslt')
libxml_dep = dependency('libxml-2.0')
libxslt_dep = dependency('libxslt')
xercesc_dep = dependency('xerces-c')
xqilla_dep = cpp.find_library('xqilla',
  has_headers: 'xqilla/xqilla-simple.hpp')
zlib_dep = dependency('zlib')

qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Concurrent', 'Core', 'Network', 'PrintSupport', 'Widgets'])

subdir('include')
subdir('man')
subdir('resources')
subdir('ui')
subdir('src')