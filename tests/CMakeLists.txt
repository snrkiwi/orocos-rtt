IF (BUILD_TESTING)

   if (NOT RUNTIME_OUTPUT_DIRECTORY)
        set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
   endif()

   if ( NOT Boost_UNIT_TEST_FRAMEWORK_FOUND )
     message(FATAL_ERROR "Can't build unit tests without Boost unit_test_framework.")
   endif()

   IF (OROCOS_TARGET STREQUAL win32)
      #SET(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
      LIST(APPEND OROCOS-RTT_DEFINITIONS BOOST_TEST_NO_LIB)
      add_definitions( /EHa )
   ENDIF()

   SET(TEST_LIBRARIES ${Boost_SERIALIZATION_LIBRARY})
   IF (NOT APPLE)
	 LIST(APPEND TEST_LIBRARIES ${Boost_THREAD_LIBRARY})
   ENDIF()

    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/src ${PROJ_BINARY_DIR}/src ${PROJ_BINARY_DIR}/src/os ${PROJ_SOURCE_DIR}/src/os ${PROJ_BINARY_DIR}/src/os/${OROCOS_TARGET})
    INCLUDE_DIRECTORIES( ${OROCOS-RTT_INCLUDE_DIRS} )

    LINK_DIRECTORIES( ${PROJ_BINARY_DIR}/src )
    IF(ENABLE_CORBA)
      INCLUDE_DIRECTORIES( ${PROJ_BINARY_DIR}/src/corba/ ${CORBA_INCLUDE_DIRS})
      LINK_DIRECTORIES( ${PROJ_BINARY_DIR}/src/corba/ ${CORBA_LINK_DIRECTORIES})
      # Work around unresolved symbols in tests when using CORBA *and* STATIC
      IF(BUILD_STATIC)
	LINK_LIBRARIES( orocos-rtt-corba-${OROCOS_TARGET} orocos-rtt-${OROCOS_TARGET} orocos-rtt-corba-${OROCOS_TARGET} orocos-rtt-${OROCOS_TARGET})
      ENDIF(BUILD_STATIC)
      if (NOT OROCOS_TARGET STREQUAL win32)
	add_definitions(-D_REENTRANT)
      endif()
    ENDIF(ENABLE_CORBA)

    # Copy over CPF files.
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/property_test_vect.cpf
                   ${RUNTIME_OUTPUT_DIRECTORY}/property_test_vect.cpf COPYONLY)

    ADD_EXECUTABLE( main-test test-main.cpp )
    TARGET_LINK_LIBRARIES( main-test orocos-rtt-dynamic_${OROCOS_TARGET})
    SET_TARGET_PROPERTIES( main-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( main-test ${RUNTIME_OUTPUT_DIRECTORY}/main-test )

    if ( ${Boost_VERSION} GREATER 103599 )
      ADD_EXECUTABLE( list-test test-runner.cpp  listlocked_test.cpp )
      TARGET_LINK_LIBRARIES( list-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
      SET_TARGET_PROPERTIES( list-test PROPERTIES
	COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
	LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
	COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
      ADD_TEST( list-test ${RUNTIME_OUTPUT_DIRECTORY}/list-test )
      list(APPEND ORO_EXTRA_TESTS "list-test")
    endif()

    # repeat for each test
    if( TESTS_OS_NO_ASM )
    else()
      SET(BUFFERS_TEST buffers_test.cpp)
    endif()
    ADD_EXECUTABLE( core-test test-runner.cpp ${BUFFERS_TEST} logger_test.cpp property_test.cpp time_test.cpp )
    TARGET_LINK_LIBRARIES( core-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
    SET_TARGET_PROPERTIES( core-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( core-test ${RUNTIME_OUTPUT_DIRECTORY}/core-test )

    ADD_EXECUTABLE( task-test test-runner.cpp tasks_test.cpp taskthread_test.cpp tasks_multiple_test.cpp )
    TARGET_LINK_LIBRARIES( task-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
    SET_TARGET_PROPERTIES( task-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( task-test ${RUNTIME_OUTPUT_DIRECTORY}/task-test )

    ADD_EXECUTABLE( event-test test-runner.cpp event_test.cpp eventservice_test.cpp )
    TARGET_LINK_LIBRARIES( event-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
    SET_TARGET_PROPERTIES( event-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( event-test ${RUNTIME_OUTPUT_DIRECTORY}/event-test )

    ADD_EXECUTABLE( taskcontext-test test-runner.cpp generictask_test.cpp generictask_test_2.cpp generictask_test_3.cpp)
    TARGET_LINK_LIBRARIES( taskcontext-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
    SET_TARGET_PROPERTIES( taskcontext-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( taskcontext-test ${RUNTIME_OUTPUT_DIRECTORY}/taskcontext-test )

    ADD_EXECUTABLE( parser-test test-runner.cpp types_test.cpp template_factory_test.cpp dispatch_test.cpp )
    TARGET_LINK_LIBRARIES( parser-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
    SET_TARGET_PROPERTIES( parser-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( parser-test ${RUNTIME_OUTPUT_DIRECTORY}/parser-test )

    ADD_EXECUTABLE( program-test test-runner.cpp program_test.cpp function_test.cpp )
    TARGET_LINK_LIBRARIES( program-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
    SET_TARGET_PROPERTIES( program-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( program-test ${RUNTIME_OUTPUT_DIRECTORY}/program-test )

    ADD_EXECUTABLE( state-test test-runner.cpp state_test.cpp )
    TARGET_LINK_LIBRARIES( state-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
    SET_TARGET_PROPERTIES( state-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}" 
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( state-test ${RUNTIME_OUTPUT_DIRECTORY}/state-test )

    ADD_EXECUTABLE( dev-test test-runner.cpp dev_test.cpp )
    TARGET_LINK_LIBRARIES( dev-test orocos-rtt-dynamic_${OROCOS_TARGET} ${TEST_LIBRARIES})
    SET_TARGET_PROPERTIES( dev-test PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
    COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
    ADD_TEST( dev-test ${RUNTIME_OUTPUT_DIRECTORY}/dev-test )

    IF(ENABLE_CORBA)
      ADD_EXECUTABLE( corba-test test-runner.cpp corba_test.cpp )
      TARGET_LINK_LIBRARIES( corba-test orocos-rtt-dynamic_${OROCOS_TARGET} orocos-rtt-corba-dynamic_${OROCOS_TARGET} ${CORBA_LIBRARIES} ${TEST_LIBRARIES})
      SET_TARGET_PROPERTIES( corba-test PROPERTIES
	COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
	LINK_FLAGS "${CMAKE_LD_FLAGS_ADD}"
	COMPILE_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
      ADD_TEST( corba-test corba-test )
      list(APPEND ORO_EXTRA_TESTS "corba-test")
    ENDIF(ENABLE_CORBA)

    ADD_CUSTOM_TARGET(check ctest -V
      DEPENDS main-test core-test task-test event-test taskcontext-test parser-test program-test state-test dev-test ${ORO_EXTRA_TESTS}
        WORKING_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}")

    #ADD_DEPENDENCIES(check prep-cpf)
ENDIF()
