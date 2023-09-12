quaduscore_src = [
    'piece.c',
    'playfield/actions.c',
    'playfield/init.c',
    'playfield/properties.c',
]

quaduscore_lib = shared_library('quaduscore', quaduscore_src,
    include_directories: quaduscore_include,
    gnu_symbol_visibility: 'hidden')
