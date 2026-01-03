
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>

namespace Vue
{
    // CreditEntry:
    // Represents one credited asset and its list of authors.
    struct CreditEntry
    {
        std::string filename;                // Asset filename / identifier
        std::vector<std::string> authors;    // List of authors for this asset
    };

    // CreditsWindow:
    // Modal overlay/window that displays credits loaded from CSV files.
    // Features:
    // - loads and aggregates credits from multiple CSV files
    // - displays a scrollable list (keyboard up/down + mouse wheel)
    // - provides a "Close" button + Escape to exit
    class CreditsWindow
    {
    private:
        // Modal state flag.
        bool active = true;

        // Font resources.
        sf::Font font;
        bool fontLoaded = false;

        // Title text.
        sf::Text titleText;

        // Render-ready texts representing the full credits list.
        // Each asset filename is a bold line, followed by multiple author lines.
        std::vector<sf::Text> creditTexts;

        // Back/Close button elements.
        sf::RectangleShape backButton;
        sf::Text backButtonLabel;
        sf::FloatRect backButtonRect; // cached rect for click hit-testing

        // Parsed credits data.
        std::vector<CreditEntry> credits;

        // Scroll support.
        float scrollOffset = 0.f;
        const float scrollSpeed = 30.f; // pixels per scroll step
        float maxScrollOffset = 0.f;

        // Build all sf::Text objects from credits data and set up button/title.
        void initializeTexts();

        // Load credits data by parsing multiple CSV files.
        void loadCreditsFromCSV();

        // Parse one CSV file and append/merge its content into assetAuthors.
        void parseCSVFile(const std::string& filepath,
                          std::map<std::string, std::vector<std::string>>& assetAuthors);

        // Split a CSV line into fields (supports quoted fields).
        std::vector<std::string> splitCSVLine(const std::string& line);

        // Update maximum scroll value based on window height and content height.
        void updateMaxScrollOffset(float windowHeight);

    public:
        CreditsWindow();

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        // Handle keyboard/mouse events (scroll + close).
        void handleEvent(const sf::Event& event);

        // Draw the credits overlay, scrollable list, scrollbar, and close button.
        void draw(sf::RenderWindow& fenetre);
    };
}
