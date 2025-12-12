#pragma once
#include <SFML/Graphics.hpp>

namespace Vue
{
    /**
     * SplashPage : écran d'introduction / écran titre.
     * - Affiche un arrière-plan (si disponible) et un texte "START GAME".
     * - Gère un léger clignotement du texte et répond aux entrées (Enter/Space/clic/ESC).
     */
    class SplashPage
    {
    private:
        bool active = true; ///< Le splash est-il actif (tant que true, l'écran s'affiche) ?

        // Ressources d'affichage
        sf::Texture backgroundTexture; ///< Texture pour l'arrière-plan (Cherub)
        sf::Sprite backgroundSprite;   ///< Sprite associé à texture
        bool backgroundLoaded = false; ///< Indique si l'arrière-plan a été chargé avec succès

        // Police et texte de démarrage
        sf::Font font;
        sf::Text startText;

        // Horloge utilisée pour l'effet de clignotement du texte
        sf::Clock blinkClock;

        // Chemins par défaut testés pour la recherche de l'image d'arrière-plan
        std::vector<std::string> defaultPaths();

    public:
        SplashPage();

        // Indique si l'écran splash est toujours actif
        bool isActive() const { return active; }

        // Gestion des événements pour le splash (entrée clavier/clic/fermeture)
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);

        // Dessine l'écran splash
        void draw(sf::RenderWindow& fenetre);
    };
}