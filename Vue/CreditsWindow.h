#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>

namespace Vue
{
    struct CreditEntry
    {
        std::string filename;    // Nom du fichier/asset
        std::vector<std::string> authors;  // Liste des auteurs
    };

    class CreditsWindow
    {
    private:
        bool active = true;
        sf::Font font;
        bool fontLoaded = false;

        sf::Text titleText;
        std::vector<sf::Text> creditTexts;
        sf::RectangleShape backButton;
        sf::Text backButtonLabel;
        sf::FloatRect backButtonRect;

        std::vector<CreditEntry> credits;

        // Scroll support
        float scrollOffset = 0.f;
        const float scrollSpeed = 30.f;  // pixels per scroll
        float maxScrollOffset = 0.f;

        void initializeTexts();
        void loadCreditsFromCSV();
        void parseCSVFile(const std::string& filepath, std::map<std::string, std::vector<std::string>>& assetAuthors);
        std::vector<std::string> splitCSVLine(const std::string& line);
        void updateMaxScrollOffset(float windowHeight);

    public:
        CreditsWindow();

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        void handleEvent(const sf::Event& event);
        void draw(sf::RenderWindow& fenetre);
    };
}