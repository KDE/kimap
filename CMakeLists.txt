project(kimap)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5327 )

add_subdirectory( tests )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(kimap_LIB_SRCS
   imapstreamparser.cpp
   job.cpp
   appendjob.cpp
   capabilitiesjob.cpp
   fetchjob.cpp
   listjob.cpp
   loginjob.cpp
   logoutjob.cpp
   rfccodecs.cpp
   selectjob.cpp
   session.cpp
   sessionthread.cpp
   closejob.cpp
   expungejob.cpp
   deletejob.cpp
   createjob.cpp
   subscribejob.cpp
   unsubscribejob.cpp
   renamejob.cpp
   checkjob.cpp
   noopjob.cpp
   storejob.cpp
)

kde4_add_library(kimap SHARED ${kimap_LIB_SRCS})

target_link_libraries(kimap ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY} kmime  ${SASL2_LIBRARIES})

include_directories( ${CMAKE_SOURCE_DIR}/kioslave ${SASL2_INCLUDE_DIR} )

set_target_properties(kimap PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kimap EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kimap_export.h job.h appendjob.h capabilitiesjob.h fetchjob.h listjob.h loginjob.h logoutjob.h rfccodecs.h selectjob.h session.h storejob.h DESTINATION ${INCLUDE_INSTALL_DIR}/kimap  COMPONENT Devel)
