cmake_minimum_required(VERSION 3.5)

project(ZControl VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Network  Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network  Widgets LinguistTools REQUIRED)

set(TS_FILES ZControl_en_GB.ts)

set(PROJECT_SOURCES
# main project
        main.cpp
        mainwindow.cpp
        mainwindow.h
        zplc.h
        zplc.cpp
        plcs.h
        plcs.cpp
        cyclewindow.h
        cyclewindow.cpp
# complementary archieves
        mainwindow.ui
        cyclewindow.ui
        zcontrol.qrc
        zcontrol.rc
        zcontrol.ini
        zcontrol.ico
# Ethernet/IP communication
        EtherNetIP/zethernetip.cpp
        EtherNetIP/zethernetip.h
# Database interface
        zmariadb.h
        zmariadb.cpp
        mysql/mysql.h
        mysql/ma_list.h
        mysql/mariadb_com.h
        mysql/mariadb_ctype.h
        mysql/mariadb_version.h
        mysql/mariadb_stmt.h
# http client & telegram
        http/zhttpservice.h
        http/zhttpservice.cpp
        http/ztelegramservice.h
        http/ztelegramservice.cpp
# http server
        httpserver/httpconnectionhandler.cpp
        httpserver/httpconnectionhandler.h
        httpserver/httpconnectionhandlerpool.cpp
        httpserver/httpconnectionhandlerpool.h
        httpserver/httpcookie.cpp
        httpserver/httpcookie.h
        httpserver/httpglobal.cpp
        httpserver/httpglobal.h
        httpserver/httplistener.cpp
        httpserver/httplistener.h
        httpserver/httprequest.cpp
        httpserver/httprequest.h
        httpserver/httprequesthandler.cpp
        httpserver/httprequesthandler.h
        httpserver/httpresponse.cpp
        httpserver/httpresponse.h
        httpserver/httpsession.cpp
        httpserver/httpsession.h
        httpserver/httpsessionstore.cpp
        httpserver/httpsessionstore.h
        httpserver/staticfilecontroller.cpp
        httpserver/staticfilecontroller.h
        ${TS_FILES}
)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/zcontrol.rc")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ZControl
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ZControl APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(ZControl SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ZControl WIN64 ${PROJECT_SOURCES} ${APP_ICON_RESOURCE_WINDOWS})
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(ZControl PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network ${CMAKE_SOURCE_DIR}/libs/libmariadb.lib)

set_target_properties(ZControl PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ZControl)
endif()
