# Release
file(STRINGS nekoray_version.txt NKR_VERSION)
add_compile_definitions(NKR_VERSION=\"${NKR_VERSION}\")

# Debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNKR_CPP_DEBUG")

# Func
function(nkr_add_compile_definitions arg)
    message("[add_compile_definitions] ${ARGV}")
    add_compile_definitions(${ARGV})
endfunction()
