
// Prototype.h
//
// Minimal interface for the Prototype design pattern.
//
// ===============================
// DESIGN / RATIONALE
// ===============================
// The Prototype pattern is used to create objects by cloning a "prototype"
// instead of directly instantiating a concrete class.
//
// Why do we want clone() here?
// - In C++, copying polymorphic objects through a base type causes slicing.
// - clone() provides a "virtual copy constructor": it returns a copy of the
//   *dynamic* type (GenericEnemy, CameraEnemy, LaserEnemy, etc.).
//
// Why does clone() return std::unique_ptr<Enemy>?
// - unique_ptr expresses unique ownership (RAII; no manual delete).
// - Returning Enemy directly avoids extra conversions / casts when storing
//   enemies polymorphically.
// - The comment in this file explicitly mentions avoiding dynamic conversions
//   at instantiation time. (Project-specific decision.)

#pragma once

#include <memory>     // std::unique_ptr

namespace Modele {

    // Forward declaration avoids including Enemy headers here,
    // which helps prevent circular dependencies and speeds up compilation.
    class Enemy;

    // Abstract interface for the Prototype pattern.
    // Any class used as a prototype must implement clone().
    struct Prototype
    {
        // Clone must return a new heap-allocated copy wrapped in a unique_ptr.
        // Ownership transfers to the caller.
        virtual std::unique_ptr<Enemy> clone() const = 0;

        // Virtual destructor ensures derived objects are destroyed properly
        // through a Prototype pointer/reference.
        virtual ~Prototype() = default;
    };

} // namespace Modele
