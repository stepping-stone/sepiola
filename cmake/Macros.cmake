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
function(GENERATE_QT_RESOURCE_FROM_FILES resource_file resource_prefix file_list)
    set(pq_resource_file_contents "<RCC>\n  <qresource prefix=\"${resource_prefix}\">\n")
    get_filename_component(current_directory ${resource_file} PATH)
    foreach(resource ${file_list})
        get_filename_component(alias ${resource} NAME)
        get_filename_component(resource ${resource} ABSOLUTE)
        file(RELATIVE_PATH resource "${current_directory}" "${resource}")
        file(TO_NATIVE_PATH "${resource}" resource)
        set(pq_resource_file_contents
            "${pq_resource_file_contents}    <file alias=\"${alias}\">${resource}</file>\n")
    endforeach(resource)
    set(pq_resource_file_contents "${pq_resource_file_contents}  </qresource>\n</RCC>\n")

    # Generate the resource file.
    set(CMAKE_CONFIGURABLE_FILE_CONTENT "${pq_resource_file_contents}")
    configure_file(${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in "${resource_file}")
    unset(CMAKE_CONFIGURABLE_FILE_CONTENT)
endfunction(GENERATE_QT_RESOURCE_FROM_FILES)
