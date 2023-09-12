testutils_src = [
    'main.c',
    'mockruleset.c'
]

testutils_lib = static_library('testutils', testutils_src,
    build_by_default: false,
    include_directories: quaduscore_include,
    install: false
)

testutils_include = include_directories('.')
