include(CMakePackageConfigHelpers)

set(NEOETER_INSTALL_TARGETS
    neoeter_platform_base
    neoeter_platform_locale
    neoeter_platform_math
    neoeter_io
    neoeter_image
    neoeter_render
    neoeter_audio
    neoeter_model_grn
    neoeter_model_speedtree
    neoeter_terrain
    neoeter_effect
    miniaudio
)

set_target_properties(neoeter_platform_base PROPERTIES EXPORT_NAME "Platform::Base")
set_target_properties(neoeter_platform_locale PROPERTIES EXPORT_NAME "Platform::Locale")
set_target_properties(neoeter_platform_math PROPERTIES EXPORT_NAME "Platform::Math")
set_target_properties(neoeter_io PROPERTIES EXPORT_NAME "IO")
set_target_properties(neoeter_image PROPERTIES EXPORT_NAME "Image")
set_target_properties(neoeter_render PROPERTIES EXPORT_NAME "Render")
set_target_properties(neoeter_audio PROPERTIES EXPORT_NAME "Audio")
set_target_properties(neoeter_model_grn PROPERTIES EXPORT_NAME "Model::Grn")
set_target_properties(neoeter_model_speedtree PROPERTIES EXPORT_NAME "Model::SpeedTree")
set_target_properties(neoeter_terrain PROPERTIES EXPORT_NAME "Terrain")
set_target_properties(neoeter_effect PROPERTIES EXPORT_NAME "Effect")
set_target_properties(miniaudio PROPERTIES EXPORT_NAME "Miniaudio")

install(TARGETS ${NEOETER_INSTALL_TARGETS}
    EXPORT NeoeterTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION include)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio/miniaudio.h"
    DESTINATION include/miniaudio)

install(EXPORT NeoeterTargets
    FILE NeoeterTargets.cmake
    NAMESPACE Neoeter::
    DESTINATION lib/cmake/Neoeter
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/NeoeterConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/NeoeterConfig.cmake"
    INSTALL_DESTINATION lib/cmake/Neoeter
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/NeoeterConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/NeoeterConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/NeoeterConfigVersion.cmake"
    DESTINATION lib/cmake/Neoeter
)
