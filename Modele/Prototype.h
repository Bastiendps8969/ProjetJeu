// Prototype.h
// Interface minimale pour le pattern Prototype.
// Toute classe qui veut être utilisée comme "prototype" doit
// dériver de `Prototype` et implémenter `clone()` qui retourne
// un nouvel objet alloué (std::unique_ptr) représentant une copie.

#pragma once
#include <memory>

namespace Modele {

// Forward declaration to avoid circular include.
class Enemy;

// Interface abstraite pour le pattern Prototype adaptée au projet.
// Nous utilisons une signature de clone qui retourne directement
// `std::unique_ptr<Enemy>` pour éviter des conversions dynamiques
// (release + dynamic_cast) lors de l'instanciation des prototypes.
struct Prototype {
    virtual std::unique_ptr<Enemy> clone() const = 0;
    virtual ~Prototype() = default;
};

} // namespace Modele
