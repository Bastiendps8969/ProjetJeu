
#include "Level.h"

// Basic Level constructor/destructor implementations.
Level::Level(std::string n, std::string d)
	// Move strings into member variables to avoid unnecessary copies.
	// Initialize countdown to 0 and lives to 3.
	//
	// WHY std::move on parameters:
	// - because parameters were passed by value, we own them locally in the constructor
	// - moving transfers their internal buffers into the members efficiently
	// - avoids an extra allocation/copy compared to name(n) / description(d)
	: name(std::move(n)),
	  description(std::move(d)),
	  countdown(0),
	  lives(3)
{
}

Level::~Level()
{
	// No manual cleanup required:
	// - std::string frees itself (RAII)
	// - ints need no destruction work
}
