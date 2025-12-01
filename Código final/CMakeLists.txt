cmake_minimum_required(VERSION 3.16)
project(Sistema_EducaUTFPR VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================
# DEFINE O CAMINHO DO DIRETÓRIO DO PROJETO
# ============================================
add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
# ============================================

# 1. Forma MODERNA de encontrar o Qt (seja 6 ou 5)
#    Isso define os "alias" genéricos (Qt::Widgets, Qt::Sql)
#    e também a variável ${QT_VERSION_MAJOR} (com 5 ou 6)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets
    Sql
)

# 2. Carrega os comandos específicos da versão (como qt_add_executable)
#    Esta linha é necessária para fazer o if/else funcionar
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets
    Sql
)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        backend/data.cpp
        backend/data.h
        mainwindow.h
        mainwindow.ui
)

# 3. Bloco if/else que você tinha, agora corrigido
#    Isso usa o comando qt_add_executable no Qt 6
#    e o add_executable padrão no Qt 5
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Sistema_EducaUTFPR
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        logindialog.h logindialog.cpp logindialog.ui
        icons.qrc
        criarconta.h criarconta.cpp criarconta.ui
        forumdialog.ui
        forumdialog.cpp
        forumdialog.h
        forumpostsdialog.h forumpostsdialog.cpp forumpostsdialog.ui
        agendaacademicadialog.h agendaacademicadialog.cpp agendaacademicadialog.ui
        duvidasdialog.h duvidasdialog.cpp duvidasdialog.ui
        perfildialog.h perfildialog.cpp perfildialog.ui
        grupoestudodialog.h grupoestudodialog.cpp grupoestudodialog.ui
        chatmateria.h chatmateria.cpp chatmateria.ui
        criargrupodialog.h criargrupodialog.cpp criargrupodialog.ui
    )
else()
    # Lógica para Qt 5 (ou Android com Qt 5)
    if(ANDROID)
        add_library(Sistema_EducaUTFPR SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Sistema_EducaUTFPR
            ${PROJECT_SOURCES}
            logindialog.h logindialog.cpp logindialog.ui
            icons.qrc
            criarconta.h criarconta.cpp criarconta.ui
        )
    endif()
endif()

# 4. Linka as bibliotecas usando os "alias" genéricos
#    O CMake vai linkar com Qt6::Widgets ou Qt5::Widgets automaticamente
target_link_libraries(Sistema_EducaUTFPR PRIVATE
    Qt::Widgets
    Qt::Sql
)

# 5. O if para o Bundle ID (identificador do app)
#    Agora ${QT_VERSION} vai funcionar porque o find_package(QT...) a definiu
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Sistema_EducaUTFPR)
endif()

set_target_properties(Sistema_EducaUTFPR PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Sistema_EducaUTFPR
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 6. O if para finalizar o executável (apenas Qt 6)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Sistema_EducaUTFPR)
endif()
