cmake_minimum_required(VERSION 3.16)

project(localRS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS SerialPort)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)


target_link_libraries(localRS PRIVATE Qt6::Core)
# 会自动将以下内容转变为qmldir，并将其添加到qrc中，还会将模块注册到engine
qt_add_qml_module(${PROJECT_NAME}
    URI localRS
    VERSION 1.0

        RESOURCES
            Resource.qrc
        RESOURCES
            inc/core/doc.md
            Todo.md

        SOURCES
            inc/future.h
            inc/core/core.h
            inc/core/ViewModel.h
            inc/manager/SettingsManager.h
            inc/model/SettingListModel.h
            inc/interface/ISettingsLoad.h
            inc/view/mainview.h
            inc/view/settings.h

        SOURCES
            src/view/Mainview.cpp
            src/manager/SettingsManager.cpp
            src/model/SettingListModel.cpp
            src/view/settings.cpp

        QML_FILES
            ui/Settings.qml
            ui/Mainview.qml

# exter moudle
        SOURCES
            exmod/SettingItem/SettingItem.h
            exmod/SettingItem/SettingItem.cpp
        QML_FILES
            exmod/SettingItem/SettingItemView.qml

        SOURCES exmod/libusb/libusb.h

        SOURCES exmod/Notification/Notification.cpp
        QML_FILES exmod/Notification/NotificationView.qml

        SOURCES exmod/SessionMessage/SessionMessage.h
        SOURCES exmod/SessionMessage/SessionMessage.cpp
        QML_FILES exmod/SessionMessage/SessionMessageView.qml

# qml style
            QML_FILES res/styles/config.qml

            SOURCES inc/service/themeSettings.h
            SOURCES src/service/themeSettings.cpp
            SOURCES inc/core/gsc.h
            SOURCES src/core/gsc.cpp
            SOURCES inc/interface/IDevice.h
            SOURCES inc/interface/ISession.h
            SOURCES inc/service/usbSession.h
            SOURCES inc/service/serialSession.h
            SOURCES inc/service/networkSession.h
            SOURCES src/service/usbSession.cpp
            SOURCES src/service/serialSession.cpp
            SOURCES src/service/networkSession.cpp
            SOURCES inc/service/bluetoothSession.h
            SOURCES inc/interface/IDataEndpoint.h
            SOURCES inc/core/connection.h
            SOURCES inc/service/usbWarpper.h           
            SOURCES inc/model/SessionMsgListModel.h
            QML_FILES ui/SessionView.qml
            SOURCES exmod/Notification/Notification.h
            SOURCES inc/core/GlobalConfig.h
            SOURCES src/core/GlobalConfig.cpp
            SOURCES inc/manager/ViewManager.h
            SOURCES src/manager/ViewManager.cpp
            SOURCES src/core/core.cpp
            SOURCES inc/interface/IGlobalManager.h
            SOURCES inc/manager/WindowCtrler.h
            SOURCES src/manager/WindowCtrler.cpp


    )

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(${PROJECT_NAME} PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    inc
    inc/core
    inc/interface
    inc/model
    inc/service
    inc/view

    exmod
    exmod/Notification
    exmod/SessionMessage
    exmod/SettingItem
    exmod/libusb

)

# 2. 添加库文件搜索路径
# link_directories(exmod/libusb)
find_library(LIBUSB_LIBRARY
    NAMES usb-1.0 libusb-1.0 libusb-1.0.dll.a
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/exmod/libusb
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick
    Qt6::SerialPort
    Qt6::Multimedia

    ${LIBUSB_LIBRARY}       #usb-1.0
)


include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 禁用debug输出
# target_compile_definitions(${PROJECT_NAME} PRIVATE QT_NO_DEBUG_OUTPUT)
