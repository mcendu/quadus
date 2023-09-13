project('quadus', 'c',
    version: '1.0.0',
    license: 'MIT',
    meson_version: '>=0.58.2',
    default_options: [
        'buildtype=debugoptimized',
        'strip=true',
        'c_std=gnu17',
    ],
)

cc = meson.get_compiler('c')

if cc.get_define('__GNUC__') != ''
    # fine tune warnings
    add_project_arguments([
        '-Werror=implicit-int',
        '-Werror=implicit-function-declaration',
        '-Wformat-security',
    ], language: ['c'])
endif

# disable asserts if not in a debug optimization preset
if get_option('optimization') not in ['plain', '0', 'g']
    add_project_arguments([
        '-DNDEBUG'
    ], language: ['c'])
endif

quaduscore_include = include_directories('include')

subdir('src')
subdir('tests')
