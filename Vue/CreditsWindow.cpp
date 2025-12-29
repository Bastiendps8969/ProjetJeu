#include "CreditsWindow.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <map>

namespace Vue
{
    CreditsWindow::CreditsWindow()
    {
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;
            loadCreditsFromCSV();
            initializeTexts();
        }
    }

    std::vector<std::string> CreditsWindow::splitCSVLine(const std::string& line)
    {
        std::vector<std::string> fields;
        std::string field;
        bool inQuotes = false;

        for (size_t i = 0; i < line.length(); ++i)
        {
            char c = line[i];
            if (c == '"')
            {
                inQuotes = !inQuotes;
            }
            else if (c == ',' && !inQuotes)
            {
                fields.push_back(field);
                field.clear();
            }
            else
            {
                field += c;
            }
        }
        fields.push_back(field);
        return fields;
    }

    void CreditsWindow::parseCSVFile(const std::string& filepath, std::map<std::string, std::vector<std::string>>& assetAuthors)
    {
        std::ifstream file(filepath);
        
        if (!file.is_open())
        {
            return;
        }

        std::string line;
        bool headerSkipped = false;

        while (std::getline(file, line))
        {
            // Skip header line
            if (!headerSkipped)
            {
                headerSkipped = true;
                continue;
            }

            if (line.empty()) continue;

            auto fields = splitCSVLine(line);
            if (fields.size() >= 3)
            {
                std::string filename = fields[0];
                std::string authors = fields[2];  // Column 2 is authors
                
                // Remove quotes
                if (!filename.empty() && filename.front() == '"')
                    filename = filename.substr(1);
                if (!filename.empty() && filename.back() == '"')
                    filename.pop_back();
                    
                if (!authors.empty() && authors.front() == '"')
                    authors = authors.substr(1);
                if (!authors.empty() && authors.back() == '"')
                    authors.pop_back();

                if (!filename.empty() && !authors.empty())
                {
                    // Split authors by comma
                    std::stringstream ss(authors);
                    std::string author;
                    while (std::getline(ss, author, ','))
                    {
                        // Trim leading/trailing whitespace
                        size_t start = author.find_first_not_of(" \t");
                        size_t end = author.find_last_not_of(" \t");
                        if (start != std::string::npos)
                        {
                            author = author.substr(start, end - start + 1);
                        }

                        if (!author.empty())
                        {
                            // Add author to the asset's author list
                            if (assetAuthors.find(filename) == assetAuthors.end())
                            {
                                assetAuthors[filename] = std::vector<std::string>();
                            }
                            // Avoid duplicate authors for same asset
                            auto& authors_list = assetAuthors[filename];
                            if (std::find(authors_list.begin(), authors_list.end(), author) == authors_list.end())
                            {
                                authors_list.push_back(author);
                            }
                        }
                    }
                }
            }
        }

        file.close();
    }

    void CreditsWindow::loadCreditsFromCSV()
    {
        credits.clear();
        std::map<std::string, std::vector<std::string>> assetAuthors;
        
        // Load from the three CSV files (paths relative to the executable)
        std::vector<std::string> csvFiles = {
            "Asset/credits/james_adams_credits.csv",
            "Asset/credits/john_jones_credits.csv",
            "Asset/credits/ennemy_credits.csv"
        };

        for (const auto& csvFile : csvFiles)
        {
            parseCSVFile(csvFile, assetAuthors);
        }

        // Convert map to vector of CreditEntry
        for (const auto& pair : assetAuthors)
        {
            CreditEntry entry;
            entry.filename = pair.first;
            entry.authors = pair.second;
            credits.push_back(entry);
        }
    }

    void CreditsWindow::initializeTexts()
    {
        // Titre
        titleText.setFont(font);
        titleText.setString("CREDITS - ARTISTES GRAPHIQUES");
        titleText.setCharacterSize(40);
        titleText.setFillColor(sf::Color(220, 30, 30));
        titleText.setStyle(sf::Text::Bold);

        // Crédits détaillés
        creditTexts.clear();
        for (const auto& credit : credits)
        {
            // Asset/Filename as title
            sf::Text assetText;
            assetText.setFont(font);
            assetText.setString(credit.filename);
            assetText.setCharacterSize(20);
            assetText.setFillColor(sf::Color(200, 200, 255));  // Light blue for filenames
            assetText.setStyle(sf::Text::Bold);
            creditTexts.push_back(assetText);

            // Authors for this asset
            for (const auto& author : credit.authors)
            {
                sf::Text authorText;
                authorText.setFont(font);
                authorText.setString("  * " + author + " *  ");
                authorText.setCharacterSize(16);
                authorText.setFillColor(sf::Color(230, 230, 230));
                creditTexts.push_back(authorText);
            }
        }

        // Back button
        backButton.setSize({220.f, 60.f});
        backButton.setFillColor(sf::Color(170, 30, 30));
        backButton.setOutlineColor(sf::Color(200, 80, 60));
        backButton.setOutlineThickness(2.f);
        backButtonLabel.setFont(font);
        backButtonLabel.setString("Close");
        backButtonLabel.setCharacterSize(22);
        backButtonLabel.setFillColor(sf::Color::White);
    }

    void CreditsWindow::updateMaxScrollOffset(float windowHeight)
    {
        // Calculate the total height of all credit texts
        float totalHeight = 0.f;
        for (const auto& text : creditTexts)
        {
            if (text.getStyle() & sf::Text::Bold)
            {
                totalHeight += 32.f;
            }
            else
            {
                totalHeight += 24.f;
            }
        }
        
        // Available space for credits (accounting for title and button)
        float availableHeight = windowHeight - 150.f;
        
        // Max scroll is when we reach the end of the list
        maxScrollOffset = std::max(0.f, totalHeight - availableHeight + 100.f);
    }

    void CreditsWindow::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape)
            {
                active = false;
            }
            else if (event.key.code == sf::Keyboard::Up)
            {
                scrollOffset = std::max(0.f, scrollOffset - scrollSpeed);
            }
            else if (event.key.code == sf::Keyboard::Down)
            {
                scrollOffset = std::min(maxScrollOffset, scrollOffset + scrollSpeed);
            }
        }

        if (event.type == sf::Event::MouseWheelScrolled)
        {
            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
            {
                float delta = event.mouseWheelScroll.delta * scrollSpeed;
                scrollOffset = std::max(0.f, std::min(maxScrollOffset, scrollOffset - delta));
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            float mx = static_cast<float>(event.mouseButton.x);
            float my = static_cast<float>(event.mouseButton.y);
            if (backButtonRect.contains(mx, my)) {
                active = false;
            }
        }
    }

    void CreditsWindow::draw(sf::RenderWindow& fenetre)
    {
        if (!fontLoaded) return;

        // Update max scroll offset based on window height
        updateMaxScrollOffset(fenetre.getSize().y);

        // Semi-transparent background
        sf::RectangleShape bg(sf::Vector2f((float)fenetre.getSize().x, (float)fenetre.getSize().y));
        bg.setFillColor(sf::Color(0, 0, 0, 180));
        fenetre.draw(bg);

        // Title
        sf::FloatRect tb = titleText.getLocalBounds();
        float centerX = fenetre.getSize().x * 0.5f;
        titleText.setPosition(centerX - tb.width * 0.5f - tb.left, 30.f - tb.top);
        fenetre.draw(titleText);

        // Create a scissor-like clipping region for scrolling credits
        float topY = 100.f;
        float maxHeight = fenetre.getSize().y - 150.f;
        float currentY = topY - scrollOffset;  // Apply scroll offset

        for (size_t i = 0; i < creditTexts.size(); ++i)
        {
            sf::Text text = creditTexts[i];
            
            // Skip drawing if text is above the visible area
            if (currentY + 32.f < topY)
            {
                // Add spacing but don't draw
                if (text.getStyle() & sf::Text::Bold)
                {
                    currentY += 32.f;
                }
                else
                {
                    currentY += 24.f;
                }
                continue;
            }
            
            // Skip drawing if text goes below the button area
            if (currentY > maxHeight) break;
            
            // Center text horizontally
            sf::FloatRect textBounds = text.getLocalBounds();
            float posX = centerX - textBounds.width * 0.5f - textBounds.left;
            text.setPosition(posX, currentY);
            fenetre.draw(text);
            
            // Add extra spacing after asset titles (bold texts)
            if (text.getStyle() & sf::Text::Bold)
            {
                currentY += 32.f;  // More space after filename
            }
            else
            {
                currentY += 24.f;  // Less space for author names
            }
        }

        // Draw scroll indicator if there's content to scroll
        if (maxScrollOffset > 0.f)
        {
            // Scroll bar background
            sf::RectangleShape scrollBarBg(sf::Vector2f(8.f, maxHeight - 20.f));
            scrollBarBg.setPosition(fenetre.getSize().x - 20.f, topY + 10.f);
            scrollBarBg.setFillColor(sf::Color(50, 50, 50, 200));
            fenetre.draw(scrollBarBg);

            // Scroll bar thumb
            float thumbHeight = std::max(20.f, (maxHeight - 20.f) * ((maxHeight - 20.f) / (maxHeight - 20.f + maxScrollOffset)));
            float thumbY = topY + 10.f + (scrollOffset / maxScrollOffset) * (maxHeight - 20.f - thumbHeight);
            sf::RectangleShape scrollBar(sf::Vector2f(8.f, thumbHeight));
            scrollBar.setPosition(fenetre.getSize().x - 20.f, thumbY);
            scrollBar.setFillColor(sf::Color(150, 30, 30, 200));
            fenetre.draw(scrollBar);
        }

        // Back button at the bottom
        float btnX = centerX - backButton.getSize().x * 0.5f;
        float btnY = fenetre.getSize().y - 100.f;
        backButton.setPosition(btnX, btnY);
        fenetre.draw(backButton);

        sf::FloatRect lb = backButtonLabel.getLocalBounds();
        backButtonLabel.setPosition(
            btnX + (backButton.getSize().x - lb.width) * 0.5f - lb.left,
            btnY + (backButton.getSize().y - lb.height) * 0.5f - lb.top
        );
        fenetre.draw(backButtonLabel);

        backButtonRect = backButton.getGlobalBounds();
    }
}
