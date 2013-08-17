project(kimap)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5327 )

add_subdirectory( tests )
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(kimap_LIB_SRCS
   acl.cpp
   imapset.cpp
   imapstreamparser.cpp
   job.cpp
   appendjob.cpp
   capabilitiesjob.cpp
   fetchjob.cpp
   idlejob.cpp
   listjob.cpp
   loginjob.cpp
   logoutjob.cpp
   namespacejob.cpp
   rfccodecs.cpp
   selectjob.cpp
   session.cpp
   sessionlogger.cpp
   sessionthread.cpp
   closejob.cpp
   expungejob.cpp
   deletejob.cpp
   createjob.cpp
   subscribejob.cpp
   unsubscribejob.cpp
   renamejob.cpp
   storejob.cpp
   copyjob.cpp
   searchjob.cpp
   acljobbase.cpp
   setacljob.cpp
   getacljob.cpp
   deleteacljob.cpp
   myrightsjob.cpp
   listrightsjob.cpp
   quotajobbase.cpp
   setquotajob.cpp
   getquotajob.cpp
   getquotarootjob.cpp
   metadatajobbase.cpp
   setmetadatajob.cpp
   getmetadatajob.cpp
)

kde4_add_library(kimap ${LIBRARY_TYPE} ${kimap_LIB_SRCS})

target_link_libraries(kimap ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY} kmime  ${SASL2_LIBRARIES})

include_directories( ${CMAKE_SOURCE_DIR}/kioslave ${SASL2_INCLUDE_DIR} ${Boost_INCLUDE_DIR} )

set_target_properties(kimap PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kimap EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kimap_export.h acl.h imapset.h job.h appendjob.h capabilitiesjob.h fetchjob.h idlejob.h listjob.h loginjob.h logoutjob.h namespacejob.h rfccodecs.h
 selectjob.h closejob.h expungejob.h deletejob.h createjob.h subscribejob.h unsubscribejob.h renamejob.h
 session.h sessionuiproxy.h storejob.h copyjob.h searchjob.h acljobbase.h setacljob.h getacljob.h deleteacljob.h
 myrightsjob.h listrightsjob.h quotajobbase.h setquotajob.h getquotajob.h getquotarootjob.h metadatajobbase.h setmetadatajob.h getmetadatajob.h
 DESTINATION ${INCLUDE_INSTALL_DIR}/kimap  COMPONENT Devel)
