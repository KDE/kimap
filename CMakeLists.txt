project(kimap)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5327 )

add_subdirectory( tests )

########### next target ###############

set(kimap_LIB_SRCS
   rfccodecs.cpp)


kde4_add_library(kimap SHARED ${kimap_LIB_SRCS})

target_link_libraries(kimap ${KDE4_KDECORE_LIBS})

set_target_properties(kimap PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kimap ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kimap_export.h rfccodecs.h DESTINATION ${INCLUDE_INSTALL_DIR}/kimap  )
