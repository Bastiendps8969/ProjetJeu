//
// Created by bertr on 13-12-25.
//

#include "Level.h"

// Basic Level constructor/destructor implementations
Level::Level(std::string n, std::string d)
	: name(std::move(n)), description(std::move(d)), countdown(0), lives(3)
{
}

Level::~Level()
{
}