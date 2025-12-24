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
        Modele/Modele.h
        Vue/Vue.cpp
        Controleur/Controleur.cpp
        Controleur/Controleur.h
        Modele/Player.cpp
        Modele/Player.h
        Modele/Modele.cpp
        Vue/CreditsWindow.cpp
        Vue/DialogueBox.cpp
        Vue/DialogueManager.cpp
        Vue/HomePage.cpp
        Vue/LevelPage.cpp
        Vue/ScoreWindow.cpp
        Modele/Objective.cpp
        Modele/ChapterLoader.cpp
        Modele/Objective.h
        Controleur/ControllerLevel.cpp
        Controleur/ControllerLevel.h
        Controleur/ControllerLevel/ControllerLevelTutorial1.cpp
        Controleur/ControllerLevel/ControllerLevelTutorial1.h
        Modele/Level.cpp
        Modele/Level.h
        Controleur/ControllerLevel/ControllerLevelTutorial2.cpp
        Controleur/ControllerLevel/ControllerLevelTutorial2.h
        Vue/SplashPage.cpp
        Modele/RoomManager.cpp
        Modele/Agent.cpp
        Vue/PauseMenu.cpp
        Vue/PauseMenu.h
        Vue/CesarVue.cpp
        Vue/ConfirmationDialog.cpp
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