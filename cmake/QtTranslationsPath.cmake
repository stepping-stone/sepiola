include_guard(GLOBAL)

include(CheckCXXSourceRuns)

function(get_qt_translations_path)
    if(NOT DEFINED QT_TRANSLATIONS_PATH)
        set(CMAKE_REQUIRED_DEFINITIONS ${Qt5Core_DEFINITIONS})
        set(CMAKE_REQUIRED_INCLUDES ${Qt5Core_INCLUDE_DIRS})
        set(CMAKE_REQUIRED_LIBRARIES ${Qt5Core_LIBRARIES})
        set(CMAKE_REQUIRED_QUIET TRUE)

        check_cxx_source_runs(
            "
            #include <cstdio>
            #include <QLibraryInfo>
            #include <QTextStream>

            int main(int, char**)
            {
                QTextStream(stdout) << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
                return 0;
            }"
            _qt_translations_path)

        if(NOT "${_qt_translations_path}")
            message(
                FATAL_ERROR
                    "Unable to determine Qt's translation path, you have to set QT_TRANSLATIONS_PATH manually"
            )
        endif()

        set(QT_TRANSLATIONS_PATH
            ${RUN_OUTPUT}
            CACHE PATH "Path to Qt's translation files")

        message(STATUS "Qt translations path: ${QT_TRANSLATIONS_PATH}")
    endif()

    if("${QT_TRANSLATIONS_PATH}" STREQUAL "")
        message(
            FATAL_ERROR
                "Qt translation files are unavailable, please make sure you have the Qt translations package installed"
        )
    endif()
endfunction()

get_qt_translations_path()
