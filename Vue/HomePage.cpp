// HomePage.cpp - implémentation de la classe HomePage
// Gère le rendu du menu principal et les interactions utilisateur.
// Utilise une image d'arrière-plan "Cherub" si disponible, et un callback pour récupérer les scores.

#include "HomePage.h"
#include "CreditsWindow.h"
#include "ScoreWindow.h"
#include "LevelPage.h"
#include <numeric> // accumulateur (std::accumulate)
#include <cmath>
#include <iostream>

namespace Vue
{
    /**
     * Constructeur HomePage
     * - Tente de charger une police système puis initialise tous les composants UI (titre, boutons, etc.)
     * - Tente aussi de charger l'image d'arrière-plan depuis plusieurs chemins (fallback)
     * @param getScores callback pour récupérer les scores (utilisé par ScoreWindow)
     * @param backgroundPath chemin optionnel pour l'image de fond
     */
    HomePage::HomePage(std::function<std::vector<int>()> getScores, const std::string& backgroundPath)
        : getScoresCb(std::move(getScores))
    {
        // Charger une police système (Arial) si possible
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;

            // Titre - "CHERUB: Hades Operation"
            titleText.setFont(font);
            titleText.setString("CHERUB");
            titleText.setCharacterSize(72);
            titleText.setFillColor(sf::Color(255, 80, 80)); // rouge néon
            titleText.setStyle(sf::Text::Bold);

            // Ombre du titre
            titleShadow = titleText;
            titleShadow.setFillColor(sf::Color(0,0,0,160));

            // Sous-titre - "Operation Hades"
            inputText.setFont(font);
            inputText.setString("Operation Hades");
            inputText.setCharacterSize(28);
            inputText.setFillColor(sf::Color(200, 100, 100)); // rouge clair
            inputText.setStyle(sf::Text::Bold);

            // Boîte de saisie (visuelle non utilisée ici, seulement pour intercepter clics)
            inputBox.setSize({0.f, 0.f});

            // ========== STYLE BOUTONS ROUGE NÉON ==========

            // Bouton PLAY - style correspondant à l'image
            playButton.setSize({380.f, 65.f});
            playButton.setFillColor(sf::Color(180, 20, 20)); // rouge foncé
            playButton.setOutlineColor(sf::Color(255, 100, 100)); // contour rouge vif (lueur néon)
            playButton.setOutlineThickness(0.f);
            playLabel.setFont(font);
            playLabel.setString("PLAY");
            playLabel.setCharacterSize(32);
            playLabel.setFillColor(sf::Color(255, 100, 100)); // texte rouge néon
            playLabel.setStyle(sf::Text::Bold);

            // Bouton SCORES - style rouge néon
            scoreButton.setSize({380.f, 65.f});
            scoreButton.setFillColor(sf::Color(180, 20, 20)); // rouge foncé
            scoreButton.setOutlineColor(sf::Color(255, 100, 100)); // contour rouge vif
            scoreButton.setOutlineThickness(0.f);
            scoreLabel.setFont(font);
            scoreLabel.setString("SCORES");
            scoreLabel.setCharacterSize(32);
            scoreLabel.setFillColor(sf::Color(180, 80, 80)); // texte rouge légèrement plus foncé
            scoreLabel.setStyle(sf::Text::Bold);

            // Bouton CREDITS - style rouge néon
            creditsButton.setSize({380.f, 65.f});
            creditsButton.setFillColor(sf::Color(180, 20, 20)); // rouge foncé
            creditsButton.setOutlineColor(sf::Color(255, 100, 100)); // contour rouge vif
            creditsButton.setOutlineThickness(0.f);
            creditsLabel.setFont(font);
            creditsLabel.setString("CREDITS");
            creditsLabel.setCharacterSize(32);
            creditsLabel.setFillColor(sf::Color(180, 80, 80)); // texte rouge légèrement plus foncé
            creditsLabel.setStyle(sf::Text::Bold);


        }

        // Tester plusieurs chemins probables pour l'image CHERUB
        std::vector<std::string> tryPaths;
        if (!backgroundPath.empty())
            tryPaths.push_back(backgroundPath);

        tryPaths.push_back("Asset/Menu/CherubMenuJeu.png");
        tryPaths.push_back("Asset/Menu/CherubMenu.png");
        tryPaths.push_back("CherubMenuJeu.png");
        tryPaths.push_back("CherubMenu.png");
        tryPaths.push_back("cmake-build-debug/Asset/Menu/CherubMenuJeu.png");
        tryPaths.push_back("cmake-build-debug/Asset/Menu/CherubMenu.png");

        for (const auto& p : tryPaths)
        {
            if (cherubTexture.loadFromFile(p))
            {
                cherubLoaded = true;
                cherubSprite.setTexture(cherubTexture);
                cherubSprite.setColor(sf::Color(255,255,255,200)); // semi-transparent
                break;
            }
        }
    }

    // Utilitaire pour dessiner un bouton — style cohérent, sans ombre, avec brillance et lueur
    static void drawStyledButton(sf::RenderWindow& window, sf::RectangleShape button, sf::Text label, bool hovered)
    {
        // Base du bouton
        sf::RectangleShape base = button;
        sf::Color baseColor = hovered ? sf::Color(230, 60, 60) : sf::Color(170, 30, 30);
        base.setFillColor(baseColor);
        base.setOutlineColor(hovered ? sf::Color(255, 120, 80) : sf::Color(200, 80, 60));
        base.setOutlineThickness(hovered ? 4.f : 2.f);
        window.draw(base);

        // Lueur au survol
        if (hovered)
        {
            sf::RectangleShape glow = button;
            glow.setFillColor(sf::Color::Transparent);
            glow.setOutlineColor(sf::Color(255, 160, 110, 200));
            glow.setOutlineThickness(6.f);
            window.draw(glow);
        }

        // Texte centré dans le bouton
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
            button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
        );
        label.setFillColor(hovered ? sf::Color(255, 250, 240) : sf::Color(255, 220, 200));
        window.draw(label);
    }

    void HomePage::centerLabel(sf::Text& label, const sf::RectangleShape& button)
    {
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
            button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
        );
    }

    void HomePage::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        if (!active) return;

        // Transmettre les événements à la fenêtre de crédits si elle est active
        if (creditsWindow && creditsWindow->isActive())
        {
            creditsWindow->handleEvent(event);
            return;
        }

        if (event.type == sf::Event::TextEntered)
        {
            if (event.text.unicode >= 32 && event.text.unicode < 127 && playerName.size() < 32 && inputFocused)
            {
                playerName.push_back(static_cast<char>(event.text.unicode));
                inputText.setString(playerName);
            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::BackSpace && !playerName.empty() && inputFocused)
            {
                // Edit name when input is focused
                playerName.pop_back();
                inputText.setString(playerName);
            }
            else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
            {
                // If the name input is focused, Enter starts the game (legacy behavior)
                if (inputFocused)
                {
                    active = false; // start the game
                    return;
                }

                // Otherwise, activate the currently selected menu item
                if (selectedIndex == 0)
                {
                    openLevelPage(fenetre);
                    return;
                }
                else if (selectedIndex == 1)
                {
                    openScoreWindow();
                    return;
                }
                else if (selectedIndex == 2)
                {
                    if (!creditsWindow)
                    {
                        creditsWindow = std::make_unique<CreditsWindow>();
                    }
                    else
                    {
                        creditsWindow->setActive(true);
                    }
                    return;
                }
            }
            else if (!inputFocused)
            {
                // Navigate menu with Up/Down (wrap around three entries)
                if (event.key.code == sf::Keyboard::Up)
                {
                    selectedIndex = (selectedIndex + 2) % 3; // wrap-around for 3 items
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    selectedIndex = (selectedIndex + 1) % 3;
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mp = sf::Mouse::getPosition(fenetre);
            sf::Vector2f world = fenetre.mapPixelToCoords(mp);

            if (playButton.getGlobalBounds().contains(world))
            {
                // Ouvrir le sélecteur de niveaux (au lieu de démarrer immédiatement)
                openLevelPage(fenetre);
                return;
            }

            if (scoreButton.getGlobalBounds().contains(world))
            {
                // Ouvrir la fenêtre des scores
                openScoreWindow();
                return;
            }

            if (creditsButton.getGlobalBounds().contains(world))
            {
                // Créer ou afficher la fenêtre de crédits
                if (!creditsWindow)
                {
                    creditsWindow = std::make_unique<CreditsWindow>();
                }
                else
                {
                    creditsWindow->setActive(true);
                }
                return;
            }


            if (inputBox.getGlobalBounds().contains(world))
            {
                inputFocused = true;
            }
            else
            {
                inputFocused = false;
            }
        }
    }

    void HomePage::draw(sf::RenderWindow& fenetre)
    {
        // Calculer la mise en page centrée selon la taille actuelle de la fenêtre
        sf::Vector2u win = fenetre.getSize();
        float centerX = static_cast<float>(win.x) * 0.5f;

        // Effacer la fenêtre (background)
        fenetre.clear(sf::Color(18,18,28)); // fond sombre

        // Dessiner l'arrière-plan CHERUB (plein écran) s'il est chargé
        if (cherubLoaded)
        {
            const sf::Texture& t = cherubTexture;
            float texW = static_cast<float>(t.getSize().x);
            float texH = static_cast<float>(t.getSize().y);

            // Calculer l'échelle pour couvrir tout l'écran (en conservant le ratio)
            float scaleX = static_cast<float>(win.x) / texW;
            float scaleY = static_cast<float>(win.y) / texH;
            float scale = std::max(scaleX, scaleY); // utiliser la plus grande échelle pour tout couvrir

            cherubSprite.setScale(scale, scale);

            // Positionner en haut à gauche
            cherubSprite.setOrigin(0.f, 0.f);
            cherubSprite.setPosition(0.f, 0.f);

            // Superposition semi-transparente pour assombrir l'arrière-plan
            sf::Color c = cherubSprite.getColor();
            c.a = 180; // plus transparent pour que l'UI soit plus lisible
            cherubSprite.setColor(c);

            fenetre.draw(cherubSprite);
        }

        // Positionner le TITRE à gauche (comme sur l'image) — légèrement plus bas
        float titleX = static_cast<float>(win.x) * 0.15f;
        float titleY = static_cast<float>(win.y) * 0.35f;

        titleText.setPosition(titleX, titleY);
        titleShadow.setPosition(titleX + 3.f, titleY + 3.f);

        // Positionner le sous-titre sous le titre
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        inputText.setPosition(titleX, titleY + titleBounds.height + 10.f);

        // Positionner les boutons sur la partie droite de l'écran (pour correspondre à l'image)
        float buttonX = centerX + 120.f; // côté droit du centre
        float startY = static_cast<float>(win.y) * 0.25f;
        float buttonGap = 85.f;

        // Bouton PLAY
        playButton.setPosition(buttonX, startY);
        centerLabel(playLabel, playButton);
        startY += buttonGap;

        // Bouton SCORES
        scoreButton.setPosition(buttonX, startY);
        centerLabel(scoreLabel, scoreButton);
        startY += buttonGap;

        // Bouton CREDITS
        creditsButton.setPosition(buttonX, startY);
        centerLabel(creditsLabel, creditsButton);
        startY += buttonGap;



        if (fontLoaded)
        {
            // Récupérer la position de la souris pour les effets de survol
            sf::Vector2i mousePixel = sf::Mouse::getPosition(fenetre);
            sf::Vector2f mousePos = fenetre.mapPixelToCoords(mousePixel);

            bool hoveredPlay = playButton.getGlobalBounds().contains(mousePos);
            bool hoveredScore = scoreButton.getGlobalBounds().contains(mousePos);
            bool hoveredCredits = creditsButton.getGlobalBounds().contains(mousePos);

            // If mouse hovers over a button, sync the keyboard selection
            if (hoveredPlay) selectedIndex = 0;
            else if (hoveredScore) selectedIndex = 1;
            else if (hoveredCredits) selectedIndex = 2;

            // Combine mouse hover with keyboard selection for visual feedback
            hoveredPlay = hoveredPlay || (selectedIndex == 0);
            hoveredScore = hoveredScore || (selectedIndex == 1);
            hoveredCredits = hoveredCredits || (selectedIndex == 2);

            // Dessiner les trois boutons en utilisant la même fonction utilitaire stylée
            drawStyledButton(fenetre, playButton, playLabel, hoveredPlay);
            drawStyledButton(fenetre, scoreButton, scoreLabel, hoveredScore);
            drawStyledButton(fenetre, creditsButton, creditsLabel, hoveredCredits);

            // Dessiner le titre (avec son ombre)
            fenetre.draw(titleShadow);
            fenetre.draw(titleText);

            // Dessiner le sous-titre
            fenetre.draw(inputText);
        }

        // Superposition des crédits (overlay)
        if (showCredits)
        {
            sf::RectangleShape overlay({(float)win.x * 0.6f, (float)win.y * 0.35f});
            overlay.setFillColor(sf::Color(0,0,0,200));
            overlay.setPosition(centerX - overlay.getSize().x/2.f, centerX*0.15f);
            fenetre.draw(overlay);

            sf::Text txt;
            txt.setFont(font);
            txt.setString("CREDITS\nDeveloper: ...\nGraphics: ...");
            txt.setCharacterSize(18);
            txt.setFillColor(sf::Color::White);
            txt.setPosition(overlay.getPosition() + sf::Vector2f(20.f,20.f));
            fenetre.draw(txt);
        }

        // Afficher la fenêtre de crédits si active
        if (creditsWindow && creditsWindow->isActive())
        {
            creditsWindow->draw(fenetre);
        }

        fenetre.display();
    }

    void HomePage::openScoreWindow()
    {
        ScoreWindow scoreWindow(getScoresCb);

        // Ouvre une fenêtre plein écran "Scores"
        sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Scores", sf::Style::Fullscreen);

        while (window.isOpen() && scoreWindow.isActive())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                scoreWindow.handleEvent(event);
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            scoreWindow.draw(window);
        }
    }

    void HomePage::openCreditsWindow()
    {
        CreditsWindow creditsWindow;

        // Afficher les crédits comme une modale sur la page d'accueil
        // La fenêtre parent est maintenant la fenêtre du jeu passée à handleEvent
    }

    // Ouvre le sélecteur LevelPage dans une fenêtre plein écran et stocke la sélection.
    void HomePage::openLevelPage(sf::RenderWindow& parent)
    {
        LevelPage selector(this->getScoresCb);
        LevelPage::Selection sel = selector.run(); // bloque jusqu'à la fermeture

        if (sel.valid)
        {
            selectedChapter = sel.chapterIndex;
            selectedLevel = sel.levelIndex;
            selectedLevelData = sel.levelData;

            // Si l'utilisateur a choisi un niveau, fermer la page d'accueil pour démarrer le jeu.
            // Le contrôleur peut récupérer getSelectedChapter/getSelectedLevel()
            active = false;
        }
        else
        {
            // L'utilisateur a annulé ou fermé la fenêtre ; on retourne simplement au menu d'accueil
        }
    }
} // namespace Vue