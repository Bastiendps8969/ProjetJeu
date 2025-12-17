//
// Pause menu - shows objectives and 3 actions: Resume, Exit level, Exit game
//

#pragma once

#include <SFML/Graphics.hpp>
#include "../Modele/Modele.h"

namespace Vue {

class PauseMenu {
public:
	enum class Option { Resume, ExitLevel, ExitGame };

	PauseMenu(Modele::Modele& modele);

	void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
	void draw(sf::RenderWindow& fenetre);

	bool isActive() const { return active; }
	void setActive(bool v) { active = v; }

	Option getSelectedOption() const { return selectedOption; }

private:
	Modele::Modele* modelePtr = nullptr;

	bool active = true;
	Option selectedOption = Option::Resume;

	sf::Font font;
	bool fontLoaded = false;

	int spaceBetweenButton = 160.f;

	// UI elements
	sf::Text titleText;
	sf::RectangleShape resumeButton;
	sf::Text resumeLabel;
	sf::RectangleShape exitLevelButton;
	sf::Text exitLevelLabel;
	sf::RectangleShape exitGameButton;
	sf::Text exitGameLabel;

	const sf::Color buttonColor = sf::Color(60, 60, 60);
	const sf::Color selectedColor = sf::Color(120, 120, 120);
	const sf::Color textColor = sf::Color::White;

	void initUI(sf::RenderWindow& fenetre);
	void updateButtonColors();
	void centerLabel(sf::Text& label, const sf::RectangleShape& button);
};

} // namespace Vue