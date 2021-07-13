project('dact', 'cpp',
  default_options: ['cpp_std=c++11'],
  version: '3.0.0')

cpp = meson.get_compiler('cpp')

alpinocorpus_dep = dependency('alpinocorpus')
libexslt_dep = dependency('libexslt')
libxml_dep = dependency('libxml-2.0')
libxslt_dep = dependency('libxslt')
zlib_dep = dependency('zlib')

qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Concurrent', 'Core', 'Gui', 'Network', 'PrintSupport', 'Widgets'])

dbxml_bundle = get_option('dbxml_bundle')
if dbxml_bundle == 'system'
  xercesc_dep = dependency('xerces-c')

  xqilla_dep = cpp.find_library('xqilla',
    has_headers: 'xqilla/xqilla-simple.hpp')

  dbxml_rpath = ''
else
  bundle_inc = include_directories(dbxml_bundle / 'include')

  xercesc_lib = cpp.find_library('xerces-c', dirs: dbxml_bundle / 'lib')
  xercesc_dep = declare_dependency(include_directories: bundle_inc,
    dependencies: xercesc_lib)

  xqilla_lib = cpp.find_library('xqilla', dirs: dbxml_bundle / 'lib')
  xqilla_dep = declare_dependency(include_directories: bundle_inc,
    dependencies: xqilla_lib)

  dbxml_rpath = dbxml_bundle / 'lib'
endif

subdir('include')
subdir('man')
subdir('resources')
subdir('ui')
subdir('src')