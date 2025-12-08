cmake_minimum_required(VERSION 3.22)
project(TestCollision)

set(CMAKE_CXX_STANDARD 20)

# Chemin vers SFML
set(SFML_DIR "C:/SFML/lib/cmake/SFML")

# Recherche des composants SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Fichiers sources
add_executable(TestCollision
        main.cpp
        Modele/Modele.cpp
        Modele/Player.cpp
        Modele/Player.h

        Vue/Vue.cpp
        Vue/HomePage.cpp
        Vue/ScoreWindow.cpp
        Vue/CreditsWindow.cpp

        Controleur/Controleur.cpp
)

# Inclusion des headers (optionnel si bien organisés)
target_include_directories(TestCollision PRIVATE
        Modele
        Vue
        Controleur
)

# Lien avec les bibliothèques SFML
target_link_libraries(TestCollision
        sfml-graphics
        sfml-window
        sfml-system
)