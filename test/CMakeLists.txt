include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(core)
add_subdirectory(db)
add_subdirectory(gtest-core)
add_subdirectory(gtest-db)
add_subdirectory(gtest-engine)

if (BUILD_ENGINE_LEVELDB)
	add_subdirectory(gtest-leveldb)
endif ()

if (BUILD_LS2)
	add_subdirectory(luna-service)
	add_subdirectory(dbclient)

	install_ls2_roles("files/sysbus" "prv")
	install_ls2_roles("files/sysbus" "pub")
	install_ls2_service("files/sysbus" "prv")
	install_ls2_service("files/sysbus" "pub")
endif()
