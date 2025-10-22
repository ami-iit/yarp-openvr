set(plugin_name OpenVRCamera)

yarp_prepare_plugin(${plugin_name}
  CATEGORY device
  TYPE yarp::dev::${plugin_name}
  INCLUDE ${plugin_name}.h
  OPTION ENABLE_${plugin_name}
  DEFAULT ON
  EXTRA_CONFIG
    WRAPPER=frameGrabber_nws_yarp
)

if(NOT ENABLE_${plugin_name})
  return()
endif()

set(${plugin_name}_SRCS
  ${plugin_name}.cpp
)

set(${plugin_name}_HDRS
  ${plugin_name}.h
)

yarp_add_plugin(${plugin_name})


target_sources(${plugin_name}
  PRIVATE
    ${${plugin_name}_SRCS}
    ${${plugin_name}_HDRS}
)

target_link_libraries(${plugin_name}
  PRIVATE
    YARP::YARP_os
    YARP::YARP_sig
    YARP::YARP_dev
    YARP::YARP_math
    PkgConfig::openvr
)

target_include_directories(${plugin_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(${plugin_name} PUBLIC cxx_std_17)

yarp_install(
  TARGETS ${plugin_name}
  EXPORT ${plugin_name}
  COMPONENT ${plugin_name}
  LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
  ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
  YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
)
