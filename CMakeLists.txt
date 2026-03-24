cmake_minimum_required(VERSION 3.21)

project(R2D VERSION 1.0.0 LANGUAGES CXX C)

# ---- C++17 ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Qt ----
find_package(Qt6 REQUIRED COMPONENTS
  Core Gui Widgets
  Charts Concurrent Network Sql Xml SerialPort WebEngineWidgets
  3DCore 3DRender 3DExtras
  OpenGL OpenGLWidgets Svg
  Positioning QuickWidgets
)

#qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---- Paths ----
set(PATH_TO_COMMON      "${CMAKE_CURRENT_LIST_DIR}/../SimCenterCommon")
set(PATH_TO_EXAMPLES    "${CMAKE_CURRENT_LIST_DIR}/../../R2DExamples")
set(PATH_TO_QGIS_DEPS "${CMAKE_CURRENT_LIST_DIR}/../QGIS_DEPS")

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# ---- Conan packages ---
find_package(QCA REQUIRED)
find_package(jansson REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ---- Other packages ----


# ---- Target name (critical!) ----
set(TARGET_NAME ${PROJECT_NAME})   # "R2D"

# ---- Sources ----
set(SOURCES
  main.cpp
)

set(HEADERS
  # you can leave empty; your helper cmake will add more
)

set(QRC_FILES
  images.qrc
  styles.qrc
)

# ---- App icon ----
set(APP_ICON "")
if(APPLE)
  set(APP_ICON "${CMAKE_CURRENT_LIST_DIR}/icons/NHERI-R2D-Icon.icns")
  if(EXISTS "${APP_ICON}")
    set_source_files_properties("${APP_ICON}" PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources"
    )
  else()
    message(WARNING "App icon not found: ${APP_ICON}")
    set(APP_ICON "")
  endif()
endif()


# ---- Create executable ----
qt_add_executable(${TARGET_NAME}
  WIN32
  MACOSX_BUNDLE
  ${SOURCES}
  ${HEADERS}
  ${QRC_FILES}
  ${APP_ICON}
)

# ---- Defines ----
target_compile_definitions(${TARGET_NAME} PRIVATE
  APP_VERSION="${PROJECT_VERSION}"
  Q_GIS
  _R2D
)

if(WIN32)
  target_compile_definitions(${TARGET_NAME} PRIVATE CURL_STATICLIB)
endif()

# ---- Optional PEER password header ----
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/R2DUserPass.h")
  target_compile_definitions(${TARGET_NAME} PRIVATE INCLUDE_USER_PASS)
  target_sources(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/R2DUserPass.h")
else()
  message(WARNING "PEER password file not found. See SampleUserPass.h to add your credentials.")
  target_sources(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/SampleUserPass.h")
endif()

# ---- Add what was in .pro via your helper functions ----

# Common
include("${PATH_TO_COMMON}/Common/Common.cmake")
simcenter_add_common(${TARGET_NAME})

# RandomVariables
include("${PATH_TO_COMMON}/RandomVariables/RandomVariables.cmake")
simcenter_add_randomvariables(${TARGET_NAME})

# InputSheetBM
include("${PATH_TO_COMMON}/InputSheetBM/InputSheet.cmake")
simcenter_add_inputsheet(${TARGET_NAME})

# SimCenterQGIS
include("${PATH_TO_COMMON}/QGIS/SimCenterQGIS.cmake")
simcenter_add_qgis(${TARGET_NAME})

# R2D
include("${CMAKE_CURRENT_LIST_DIR}/R2D.cmake")
simcenter_add_r2d(${TARGET_NAME})

# Common Workflow
include(${CMAKE_CURRENT_LIST_DIR}/../SimCenterCommon/Workflow/Workflow.cmake)
simcenter_add_workflow(${TARGET_NAME})


target_include_directories(${TARGET_NAME} PRIVATE
  ${QCA_INCLUDE_DIR}
  ${PATH_TO_QGIS_DEPS}/include
)
  

# ---- Qt link libs ----
target_link_libraries(${TARGET_NAME} PRIVATE
  Qt6::Core Qt6::Gui Qt6::Widgets
  Qt6::Charts Qt6::Concurrent Qt6::Network Qt6::Sql Qt6::Xml
  Qt6::SerialPort Qt6::WebEngineWidgets
  Qt6::3DCore Qt6::3DRender Qt6::3DExtras
  Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg
  Qt6::Positioning Qt6::QuickWidgets
)

# Define variables with default values (can be overridden via CLI)
set(QGIS_LIB_DIR "/Users/fmckenna/NHERI/QGIS/build/output/Contents/Frameworks" CACHE PATH "Path to QGIS libraries")
set(QGIS_VERSION "4.1.0" CACHE STRING "QGIS Library version")

if(APPLE)
   target_link_libraries(${TARGET_NAME} PRIVATE
        "${QGIS_LIB_DIR}/libqgis_app.${QGIS_VERSION}.dylib"
        "${QGIS_LIB_DIR}/libqgis_gui.${QGIS_VERSION}.dylib"
        "${QGIS_LIB_DIR}/libqgis_native.${QGIS_VERSION}.dylib"
        "${QGIS_LIB_DIR}/libqgis_analysis.${QGIS_VERSION}.dylib"
        "${QGIS_LIB_DIR}/libqgis_3d.${QGIS_VERSION}.dylib"
        "${QGIS_LIB_DIR}/libqgis_core.${QGIS_VERSION}.dylib"
   )
endif()
	
# ---- External libs (mirror your .pro, but prefer find_package results) ----
target_link_libraries(${TARGET_NAME} PRIVATE
  ${QWT_LIBRARY}
  ${QSCINTILLA_LIBRARY}
  CURL::libcurl
  ZLIB::ZLIB
  jansson::jansson
  nlohmann_json::nlohmann_json
  ${QCA_LIBRARY}
)

if(WIN32)
  target_link_libraries(${TARGET_NAME} PRIVATE Advapi32)
elseif(APPLE)
  # If you truly need these, keep them; otherwise prefer find_package(LAPACK/BLAS)
  target_link_libraries(${TARGET_NAME} PRIVATE lapack blas)
endif()

qt_finalize_executable(${TARGET_NAME})

