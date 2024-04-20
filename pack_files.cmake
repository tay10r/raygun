include_guard()

find_package(Python REQUIRED COMPONENTS Interpreter)

function(pack_files output_path prefix)

  set (files)

  foreach (file ${ARGN})
    if (NOT IS_ABSOLUTE "${file}")
      list(APPEND files "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    else()
      list(APPEND files "${file}")
    endif()
  endforeach()

  add_custom_command(OUTPUT "${output_path}"
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/pack_files.py" "${output_path}" "${prefix}" ${files}
    DEPENDS ${files} "${CMAKE_CURRENT_SOURCE_DIR}/pack_files.py"
    COMMENT "Packing files to ${output_path}.")

endfunction()
