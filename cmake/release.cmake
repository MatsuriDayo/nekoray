file(STRINGS nekoray_version.txt NKR_RELEASE_DATE)
add_compile_definitions(NKR_RELEASE_DATE=\"${NKR_RELEASE_DATE}\")
