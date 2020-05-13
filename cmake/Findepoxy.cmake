if(WIN32)
	find_path(epoxy_INCLUDE_DIR NAMES epoxy/gl.h)
	find_library(epoxy_LIBRARY NAMES epoxy)

	mark_as_advanced(epoxy_INCLUDE_DIR epoxy_LIBRARY)

	add_library(epoxy STATIC IMPORTED)
	set_target_properties(epoxy PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${epoxy_INCLUDE_DIR}
		IMPORTED_LOCATION ${epoxy_LIBRARY}
	)
endif()
