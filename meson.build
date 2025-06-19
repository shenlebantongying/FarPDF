project(
    'FarPDF',
    ['cpp'],
    version: '1.0.0',
    default_options: ['warning_level=3', 'cpp_std=gnu++23'],
)

cc = meson.get_compiler('cpp')

# Destructors of mupdfcpp is not useable...
add_project_arguments('-DNDEBUG', language: 'cpp')

deps = []

deps += cc.find_library(
    'mupdf',
    has_headers: ['mupdf/fitz/version.h', 'mupdf/fitz.h', 'mupdf/pdf.h'],
    required: true,
)

dep_mupdfcpp = cc.find_library(
    'mupdfcpp',
    has_headers: ['mupdf/fitz/version.h', 'mupdf/fitz.h', 'mupdf/pdf.h'],
    required: true,
)



deps += dep_mupdfcpp


qt6 = import('qt6')
deps += dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])

subdir('src')

far_moc_files = qt6.compile_moc(headers: far_moc_sources, dependencies: deps)
farpdf = executable(
    'farpdf',
    far_sources,
    far_moc_files,
    dependencies: deps,
    install: true,
)


configure_file(
    copy: true,
    input: 'org.slbtty.farpdf.desktop.in',
    output: '@BASENAME@',
    install: true,
    install_dir: join_paths(get_option('datadir'), 'applications'),
)
