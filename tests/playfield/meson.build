tests_playfield = [
    ['testPlayfieldInit', 'init.c'],
    ['testPlayfieldSpawn', 'spawn.c'],
    ['testPlayfieldCanRotate', 'overlap.c'],
#    ['testPlayfieldMove', 'move.c']
]

foreach t : tests_playfield
    bin = executable(t[0], t[1],
        build_by_default: false,
        install: false,
        include_directories: [quaduscore_include, testutils_include],
        dependencies: check_dep,
        link_with: [quaduscore_lib, testutils_lib]
    )
    test(t[0], bin)
endforeach
