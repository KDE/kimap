project(kimap)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5327 )

add_subdirectory( tests )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(kimap_LIB_SRCS
   imapstreamparser.cpp
   job.cpp
   loginjob.cpp
   rfccodecs.cpp
   session.cpp
   sessionthread.cpp
)

kde4_add_library(kimap SHARED ${kimap_LIB_SRCS})

target_link_libraries(kimap ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY})

set_target_properties(kimap PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kimap EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kimap_export.h job.h loginjob.h rfccodecs.h session.h DESTINATION ${INCLUDE_INSTALL_DIR}/kimap  COMPONENT Devel)
