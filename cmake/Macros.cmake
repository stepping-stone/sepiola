
# The following macro is from ParaViews CMake/ParaViewMacros.cmake
#------------------------------------------------------------------------------
# GENERATE_QT_RESOURCE_FROM_FILES can be used to generate a Qt resource file
# from a given set of files.
# ARGUMENTS:
# resource_file: IN : full pathname of the qrc file to generate. 
# resource_prefix: IN : the name used in the "prefix" attribute for the
#                       generated qrc file.
# file_list: IN : list of files to be added into the resource file.
#------------------------------------------------------------------------------
FUNCTION(GENERATE_QT_RESOURCE_FROM_FILES resource_file resource_prefix file_list)
  SET (pq_resource_file_contents "<RCC>\n  <qresource prefix=\"${resource_prefix}\">\n")
  GET_FILENAME_COMPONENT(current_directory ${resource_file} PATH)
  FOREACH (resource ${file_list})
    GET_FILENAME_COMPONENT(alias ${resource} NAME)
    GET_FILENAME_COMPONENT(resource ${resource} ABSOLUTE)
    FILE(RELATIVE_PATH resource "${current_directory}" "${resource}")
    FILE(TO_NATIVE_PATH "${resource}" resource)
    SET (pq_resource_file_contents
      "${pq_resource_file_contents}    <file alias=\"${alias}\">${resource}</file>\n")
  ENDFOREACH (resource)
  SET (pq_resource_file_contents
    "${pq_resource_file_contents}  </qresource>\n</RCC>\n")

  # Generate the resource file.
  set (CMAKE_CONFIGURABLE_FILE_CONTENT "${pq_resource_file_contents}")
  configure_file(${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in
                 "${resource_file}")
  unset (CMAKE_CONFIGURABLE_FILE_CONTENT)
ENDFUNCTION(GENERATE_QT_RESOURCE_FROM_FILES)
