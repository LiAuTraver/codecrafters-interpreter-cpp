include_guard()


find_package(Python3 COMPONENTS Interpreter)

if(NOT Python3_Interpreter_FOUND)
  message(WARNING "Python3 interpreter was not found. dll may not be copied to the target directory.")
elseif(Python3_FOUND)
  set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
  message(STATUS "Python3 interpreter found at ${Python3_EXECUTABLE}")
else()
  message(WARNING "Python3 interpreter was found, but Python3 was not found. This may caused by venv or conda environment.")
endif()

macro(copy_dlls target deps)
  add_custom_target(copy_dlls_${target} ALL
    COMMENT "copy necessary dynamic libraries to the test directory..."
    COMMAND ${CMAKE_COMMAND} -E env ${PYTHON_EXECUTABLE} ${PROJECT_ROOT_DIR}/scripts/copy_dlls.py $<TARGET_FILE_DIR:${deps}> $<TARGET_FILE_DIR:${target}>
    DEPENDS ${deps}
  )
  add_dependencies(${target} copy_dlls_${target})
endmacro()