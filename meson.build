project(
    'FarPDF',
    ['cpp'],
    version: '1.0.0',
    default_options: ['warning_level=3', 'cpp_std=gnu++23'],
)

cc = meson.get_compiler('cpp')

# Destructors of mupdfcpp is not useable...
add_project_arguments(
    '-DNDEBUG',
    '-DQT_ENABLE_STRICT_MODE_UP_TO=0x070000',
    language: 'cpp',
)

deps = []

if target_machine.system() == 'darwin'
    mupdf_inc = include_directories('/opt/homebrew/include')
    mupdf_dirs = ['/opt/homebrew/lib']
else
    mupdf_inc = include_directories()
    mupdf_dirs = []
endif

deps += cc.find_library(
    'mupdf',
    dirs: mupdf_dirs,
    header_include_directories: [mupdf_inc],
    has_headers: ['mupdf/fitz/version.h', 'mupdf/fitz.h', 'mupdf/pdf.h'],
    required: true,
)

dep_mupdfcpp = cc.find_library(
    'mupdfcpp',
    dirs: mupdf_dirs,
    header_include_directories: [mupdf_inc],
    has_headers: ['mupdf/classes.h'],
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
