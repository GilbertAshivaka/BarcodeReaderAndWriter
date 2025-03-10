cmake_minimum_required(VERSION 3.16)

project(BarcodeReader4 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Concurrent Multimedia QuickControls2)

qt_standard_project_setup()

qt_add_executable(appBarcodeReader4
    main.cpp
)

qt_add_qml_module(appBarcodeReader4
    URI BarcodeReader4
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES barcodereader.h barcodereader.cpp
    SOURCES barcodewriter.h barcodewriter.cpp
    QML_FILES BarcodeWriter.qml
    QML_FILES ReadBarcode.qml
)

#add the ZXing Directory
add_subdirectory(zxing-cpp)

# Include directories for ZXing (if needed)
target_include_directories(appBarcodeReader4 PRIVATE
    ${CMAKE_SOURCE_DIR}/zxing-cpp/core
)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appBarcodeReader4 PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appBarcodeReader4
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appBarcodeReader4
    PRIVATE Qt6::Quick
    PRIVATE ZXing::ZXing
    PRIVATE Qt6::Multimedia
    PRIVATE Qt6::Concurrent
    PRIVATE Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS appBarcodeReader4
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
