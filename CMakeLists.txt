cmake_minimum_required(VERSION 3.16)

project(goodasm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)



find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

set(FETCHCONTENT_UPDATES_DISCONNECTED 1)
include(FetchContent)
FetchContent_Declare(replxx GIT_REPOSITORY https://github.com/AmokHuginnsson/replxx.git)
FetchContent_MakeAvailable(replxx)


set(GOODASM_SOURCES
    # GoodASM classes
    gainstruction.h gainstruction.cpp
    gamnemonic.h gamnemonic.cpp
    gaparameter.h gaparameter.cpp
    goodasm.h goodasm.cpp
    gasymboltable.h gasymboltable.cpp

    # Listing Styles
    galisting.h galisting.cpp
    galistingdefault.h galistingdefault.cpp
    galistingnasm.h galistingnasm.cpp
    galistingc.h galistingc.cpp
    galistinggo.h galistinggo.cpp
    galistingyarax.h galistingyarax.cpp
    galistingmarkdown.h galistingmarkdown.cpp
    galistinghex.h galistinghex.cpp
    galistingga.h galistingga.cpp

    #Graders
    gagrader.h gagrader.cpp
    gagradervalidops.h gagradervalidops.cpp
    gagraderjumptargets.h gagraderjumptargets.cpp
    gagradergameboy.h gagradergameboy.cpp
    gagraderz80.h gagraderz80.cpp
    gagradermovtarget.h gagradermovtarget.cpp
    gagrader8051jmptable.h gagrader8051jmptable.cpp
    gagrader8051pushpop.h gagrader8051pushpop.cpp

    # Lexer and Parser
    gatoken.h gatoken.cpp
    galexer.h galexer.cpp
    gaparser.h gaparser.cpp

    # Languages
    galanguage.h galanguage.cpp
    galangucom43.h galangucom43.cpp
    galangtlcs47.h galangtlcs47.cpp
    galangs2000.h galangs2000.cpp
    galangpic16c5x.h galangpic16c5x.cpp
    galangmarc4.h galangmarc4.cpp
    galang6502.h galang6502.cpp
    galang8051.h galang8051.cpp
    galangsm83.h galangsm83.cpp
    galang6805.h galang6805.cpp
    galangst7.h galangst7.cpp
    galangchip8.h galangchip8.cpp
    galangz80.h galangz80.cpp
    galangz8.h galangz8.cpp
    galang8086.h galang8086.cpp
    galangfcard.h galangfcard.cpp
    galang8080.h galang8080.cpp
    galangh83.h galangh83.cpp
    galangarm7tdmi.h galangarm7tdmi.cpp
    galangtms320c28x.h galangtms320c28x.cpp
    galangti80.h galangti80.cpp
)


# We don't build the GUI on Linux, as Ubuntu ships
# older versions of Qt Widgets and we aren't compatible.
if(ANDROID OR IOS)
  qt_add_executable(appgoodasmapp
      maingui.cpp
      ${GOODASM_SOURCES}
  )

  qt_add_qml_module(appgoodasmapp
      URI goodasmapp
      VERSION 1.0
      QML_FILES
          Main.qml
          SOURCES gaobject.h gaobject.cpp
  )

  # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
  # If you are developing for iOS or macOS you should consider setting an
  # explicit, fixed bundle identifier manually though.
  set_target_properties(appgoodasmapp PROPERTIES
  #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appgoodasmapp
      MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
      MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
      MACOSX_BUNDLE TRUE
      WIN32_EXECUTABLE TRUE
  )

  # App mustn't contain external libraries.
  target_link_libraries(appgoodasmapp PRIVATE Qt6::Quick)

  set(APPS
    appgoodasmapp)
else()
  set(APPS
    goodasm)
endif()


include(GNUInstallDirs)

# This defines the CLI client.
if(WIN32 OR (LINUX AND NOT ANDROID) OR (APPLE AND NOT IOS) )
    qt_add_library(libgoodasm STATIC
        #main.cpp
        ${GOODASM_SOURCES}
    )
    qt_add_executable(goodasm
        main.cpp

        # Interactive mode.
        # garepl.h garepl.cpp  ## Old REPL, removed 2025.07.06
        gareplxx.h gareplxx.cpp
    )
    target_link_libraries(libgoodasm PUBLIC Qt6::Quick)
    target_include_directories(libgoodasm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    set_property(TARGET libgoodasm PROPERTY OUTPUT_NAME goodasm)
    # Have MSVC define the the __cplusplus macro for Qt6 to build correctly.
    # Also suppress two small warnings due to Qt6's header files.
    # Reference: https://stackoverflow.com/a/60890947
    if (MSVC)
        if (MSVC_VERSION GREATER_EQUAL 1914)
            target_compile_options(libgoodasm PUBLIC /Zc:__cplusplus /wd26495 /wd26813)
        else()
            message(FATAL_ERROR "Only Visual Studio versions 2017 version 15.7 and above are supported.")
        endif()
    endif()
    if(WIN32 AND (TARGET Qt6::windeployqt))
        add_custom_command(TARGET goodasm
            POST_BUILD
            COMMAND Qt6::windeployqt --dir "$<TARGET_FILE_DIR:goodasm>" "$<TARGET_FILE_DIR:goodasm>/$<TARGET_FILE_NAME:goodasm>"
                $<IF:$<CONFIG:Debug>,--debug,--release> --no-translations
        )
    endif()
    ## replxx will soon replace readline.
    target_link_libraries(goodasm PRIVATE libgoodasm replxx::replxx)
    install(TARGETS ${APPS} DESTINATION bin)
endif()


install(TARGETS ${APPS}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(WIN32)
    qt_generate_deploy_app_script(
        TARGET goodasm
        OUTPUT_SCRIPT GOODASM_DEPLOY_SCRIPT
        NO_TRANSLATIONS
    )
    install(SCRIPT ${GOODASM_DEPLOY_SCRIPT})
endif()
