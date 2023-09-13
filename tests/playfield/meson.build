tests_playfield = [
    ['testPlayfieldInit', 'init.c'],
    ['testPlayfieldMove', 'move.c'],
    ['testPlayfieldCanRotate', 'overlap.c'],
    ['testPlayfieldRotate', 'rotate.c'],
    ['testPlayfieldSpawn', 'spawn.c'],
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
