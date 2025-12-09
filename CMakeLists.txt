cmake_minimum_required(VERSION 3.22)
project(TestCollision)

set(CMAKE_CXX_STANDARD 20)

# Chemin vers SFML
set(SFML_DIR "C:/SFML/lib/cmake/SFML")

# Recherche des composants SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Télécharger nlohmann/json (header-only)
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/include.zip
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(json)

# Fichiers sources
add_executable(TestCollision
        main.cpp
        Modele/Modele.cpp
        Vue/Vue.cpp
        Controleur/Controleur.cpp
        Modele/Player.cpp
        Modele/Player.h

        Vue/Vue.cpp
        Vue/HomePage.cpp
        Vue/ScoreWindow.cpp
        Vue/CreditsWindow.cpp

        Controleur/Controleur.cpp
)

# Inclusion des headers
target_include_directories(TestCollision PRIVATE
        Modele
        Vue
        Controleur
        ${json_SOURCE_DIR}/include
)

# Lien avec les bibliothèques
target_link_libraries(TestCollision
        sfml-graphics
        sfml-window
        sfml-system
)