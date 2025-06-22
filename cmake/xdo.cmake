set(xdo_source_url "https://github.com/jordansissel/xdotool/archive/33092d8a74d60c9ad3ab39c4f05b90e047ea51d8.zip")
set(xdo_checksum "cba1c7d0c23cc60cd0b1f5483f84296ff91360598e2c19c57395aec530af99ad")

ExternalProject_Add(xdo
    SOURCE_DIR ${external_dir}/xdo
    BINARY_DIR ${PROJECT_BINARY_DIR}/external/xdo
    INSTALL_DIR ${PROJECT_BINARY_DIR}/external
    URL ${xdo_source_url}
    URL_HASH SHA256=${xdo_checksum}
    PATCH_COMMAND patch --batch --unified -p1 --directory=<SOURCE_DIR>
                -i ${patches_dir}/xdo.patch ||
                    echo "patch cmd failed, likely already patched"
    CONFIGURE_COMMAND cp -r --no-target-directory <SOURCE_DIR> <BINARY_DIR>
    BUILD_COMMAND CPATH=${external_include_dir} make libxdo.a
    BUILD_ALWAYS False
    INSTALL_COMMAND PKG_CONFIG_PATH=${external_lib_dir}/pkgconfig
        make PREFIX=<INSTALL_DIR> install &&
        cp <BINARY_DIR>/libxdo.a ${external_lib_dir}/
)

list(APPEND deps_libs "${external_lib_dir}/libxdo.a")
list(APPEND deps xdo)
if(BUILD_XKBCOMMON)
    ExternalProject_Add_StepDependencies(xdo configure xkbcommon)
endif()

# libxdo depends on Xtst and Xinerama
pkg_search_module(xtst REQUIRED xtst)
list(APPEND deps_libs ${xtst_LIBRARIES})
list(APPEND includes ${xtst_INCLUDE_DIRS})
pkg_search_module(xinerama REQUIRED xinerama)
list(APPEND deps_libs ${xinerama_LIBRARIES})
list(APPEND includes ${xinerama_INCLUDE_DIRS})
