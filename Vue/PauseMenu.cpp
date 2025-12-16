//
// Pause menu implementation
//

#include "PauseMenu.h"
#include <iostream>

using namespace Vue;

PauseMenu::PauseMenu(Modele::Modele& modele)
	: modelePtr(&modele)
{
	fontLoaded = font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

	titleText.setFont(font);
	titleText.setString("Pause");
	titleText.setCharacterSize(40);
	titleText.setFillColor(textColor);

	resumeButton.setSize(sf::Vector2f(300.f, 60.f));
	resumeButton.setFillColor(buttonColor);
	resumeLabel.setFont(font);
	resumeLabel.setString("Resume");
	resumeLabel.setCharacterSize(24);
	resumeLabel.setFillColor(textColor);

	exitLevelButton.setSize(sf::Vector2f(300.f, 60.f));
	exitLevelButton.setFillColor(buttonColor);
	exitLevelLabel.setFont(font);
	exitLevelLabel.setString("Exit level");
	exitLevelLabel.setCharacterSize(24);
	exitLevelLabel.setFillColor(textColor);

	exitGameButton.setSize(sf::Vector2f(300.f, 60.f));
	exitGameButton.setFillColor(buttonColor);
	exitGameLabel.setFont(font);
	exitGameLabel.setString("Exit game");
	exitGameLabel.setCharacterSize(24);
	exitGameLabel.setFillColor(textColor);
}

void PauseMenu::initUI(sf::RenderWindow& fenetre)
{
	sf::Vector2u sz = fenetre.getSize();
	float cx = sz.x / 2.f;

	titleText.setPosition(cx - titleText.getLocalBounds().width/2.f, 50.f);

	float bx = cx + 100.f; // buttons on right
	float by = 140.f;
	resumeButton.setPosition(bx - resumeButton.getSize().x/2.f, by);
	centerLabel(resumeLabel, resumeButton);

	exitLevelButton.setPosition(bx - exitLevelButton.getSize().x/2.f, by + 80.f);
	centerLabel(exitLevelLabel, exitLevelButton);

	exitGameButton.setPosition(bx - exitGameButton.getSize().x/2.f, by + 160.f);
	centerLabel(exitGameLabel, exitGameButton);
}

void PauseMenu::centerLabel(sf::Text& label, const sf::RectangleShape& button)
{
	sf::FloatRect tb = label.getLocalBounds();
	sf::FloatRect bb = button.getGlobalBounds();
	label.setPosition(bb.left + (bb.width - tb.width)/2.f - tb.left, bb.top + (bb.height - tb.height)/2.f - tb.top);
}

void PauseMenu::updateButtonColors()
{
	resumeButton.setFillColor(selectedOption == Option::Resume ? selectedColor : buttonColor);
	exitLevelButton.setFillColor(selectedOption == Option::ExitLevel ? selectedColor : buttonColor);
	exitGameButton.setFillColor(selectedOption == Option::ExitGame ? selectedColor : buttonColor);
}

void PauseMenu::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
{
	if (!fontLoaded) return;

	if (event.type == sf::Event::KeyPressed)
	{
		if (event.key.code == sf::Keyboard::Escape)
		{
			// resume
			active = false;
			return;
		}
		else if (event.key.code == sf::Keyboard::Up)
		{
			if (selectedOption == Option::ExitGame) selectedOption = Option::ExitLevel;
			else if (selectedOption == Option::ExitLevel) selectedOption = Option::Resume;
		}
		else if (event.key.code == sf::Keyboard::Down)
		{
			if (selectedOption == Option::Resume) selectedOption = Option::ExitLevel;
			else if (selectedOption == Option::ExitLevel) selectedOption = Option::ExitGame;
		}
		else if (event.key.code == sf::Keyboard::Enter)
		{
			active = false; // close menu and return selection
		}
	}

	if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
	{
		sf::Vector2f mousePos = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre));
		if (resumeButton.getGlobalBounds().contains(mousePos)) { selectedOption = Option::Resume; active = false; }
		else if (exitLevelButton.getGlobalBounds().contains(mousePos)) { selectedOption = Option::ExitLevel; active = false; }
		else if (exitGameButton.getGlobalBounds().contains(mousePos)) { selectedOption = Option::ExitGame; active = false; }
	}
	else if (event.type == sf::Event::MouseMoved)
	{
		sf::Vector2f mousePos = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre));
		if (resumeButton.getGlobalBounds().contains(mousePos)) selectedOption = Option::Resume;
		else if (exitLevelButton.getGlobalBounds().contains(mousePos)) selectedOption = Option::ExitLevel;
		else if (exitGameButton.getGlobalBounds().contains(mousePos)) selectedOption = Option::ExitGame;
	}

	updateButtonColors();
}

void PauseMenu::draw(sf::RenderWindow& fenetre)
{
	if (!fontLoaded) return;

	// overlay
	sf::RectangleShape overlay(sf::Vector2f((float)fenetre.getSize().x, (float)fenetre.getSize().y));
	overlay.setFillColor(sf::Color(0,0,0,160));
	fenetre.draw(overlay);

	initUI(fenetre);

	// Objectives on left
	if (modelePtr)
	{
		auto& objs = modelePtr->getCurrentRoomObjectives();
		float x = 40.f;
		float y = 120.f;

		sf::Text primHeader("Objectifs primaires:", font, 24);
		primHeader.setFillColor(sf::Color::White);
		primHeader.setPosition(x, y);
		fenetre.draw(primHeader);
		y += 36.f;

		for (auto& o : objs) {
			if (!o.isPrimary()) continue;
			sf::Text t(o.getTitle(), font, 20);
			t.setPosition(x, y);
			t.setFillColor(o.isAccomplished() ? sf::Color(0,200,0) : sf::Color(200,0,0));
			fenetre.draw(t);
			y += 28.f;
		}

		y += 12.f;
		sf::Text secHeader("Objectifs secondaires:", font, 24);
		secHeader.setFillColor(sf::Color::White);
		secHeader.setPosition(x, y);
		fenetre.draw(secHeader);
		y += 36.f;

		for (auto& o : objs) {
			if (o.isPrimary()) continue;
			sf::Text t(o.getTitle(), font, 20);
			t.setPosition(x, y);
			t.setFillColor(o.isAccomplished() ? sf::Color(0,200,0) : sf::Color(200,0,0));
			fenetre.draw(t);
			y += 28.f;
		}
	}

	// Title and buttons
	fenetre.draw(titleText);

	fenetre.draw(resumeButton);
	fenetre.draw(resumeLabel);

	fenetre.draw(exitLevelButton);
	fenetre.draw(exitLevelLabel);

	fenetre.draw(exitGameButton);
	fenetre.draw(exitGameLabel);
}
