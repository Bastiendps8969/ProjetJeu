
#include "Level.h"

// Basic Level constructor/destructor implementations.

Level::Level(std::string n, std::string d)
	// Move strings into member variables to avoid unnecessary copies.
	// Initialize countdown to 0 and lives to 3.
	: name(std::move(n)),
	  description(std::move(d)),
	  countdown(0),
	  lives(3)
{
}

Level::~Level()
{
}
