project(
    'FarPDF',
    'cpp',
    version: '1.0.0',
    default_options: ['warning_level=3', 'cpp_std=c++20'],
)

cc = meson.get_compiler('cpp')

mupdf_dep = cc.find_library(
    'mupdf',
    has_headers: ['mupdf/fitz/version.h', 'mupdf/fitz.h', 'mupdf/pdf.h'],
    required: true,
)

qt6 = import('qt6')
qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])

deps = [mupdf_dep, qt6_dep]

subdir('src')

far_moc_files = qt6.compile_moc(headers: far_moc_sources, dependencies: qt6_dep)
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
