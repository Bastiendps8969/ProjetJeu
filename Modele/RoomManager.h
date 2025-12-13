// RoomManager.h
// Gestion des pièces (rooms) et du chargement depuis un fichier JSON.
// Ce fichier définit :
// - `Door` : représentation simple d'une porte (direction, index cible, forme visuelle)
// - `EnemyDefinition` : struct contenant les paramètres déclarés dans le JSON
//    pour créer des ennemis / caméras / lasers (position, patrol, type, etc.)
// - `Room` : nom + liste de portes + définitions d'ennemis
// - `RoomManager` : charge les rooms depuis le JSON, prépare des formes
//    visuelles (sf::Shape) pour les portes, et expose l'interface utilisée
//    par `Modele` pour récupérer les ennemis et portes de la pièce courante.

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>

namespace Modele {

// Door
// - `direction` : "up"/"down"/"left"/"right"
// - `targetRoomIndex` : index de la pièce cible dans la map JSON
// - `bounds` : rectangle global (utilisé pour collision / test d'entrée)
// - `visualShape` : forme SFML pour dessiner la porte dans la vue
struct Door {
    std::string direction;
    int targetRoomIndex;
    sf::FloatRect bounds;
    std::unique_ptr<sf::Shape> visualShape;
};

// EnemyDefinition
// Contient les données lues depuis le JSON pour instancier un ennemi
// via le Modele/RoomManager. Ne contient pas de logique live : c'est
// simplement une description (position, patrouille, type, paramètres de vision).
struct EnemyDefinition {
    sf::Vector2f position;                  // position en pixels dans la pièce
    std::vector<sf::Vector2f> patrolPoints; // points de patrouille (facultatif)
    float speed = 2.0f;                     // vitesse de patrouille
    std::string textureName;                // nom de texture (optionnel)

    // Type/paramètres spécifiques
    bool isCamera = false;                  // true si c'est une caméra fixe
    std::string facing;                     // "left"/"right"/"up"/"down"
    float visionRange = 300.f;              // portée du cône (par défaut)
    float visionAngle = 60.f;               // angle du cône (degrés)

    bool isLaser = false;                   // true si c'est un laser (rayon)
    float laserLength = 600.f;              // longueur du laser
};

// Room
// Contient le nom de la pièce, la liste des portes et des définitions
// d'ennemis (utiles pour le Modele lors du remplissage des entités).
struct Room {
    std::string name;
    std::vector<Door> doors;
    std::vector<EnemyDefinition> enemyDefs;
};

// RoomManager
// - Charge le fichier JSON de configuration (`rooms.json`), construit
//   la map `rooms_` et initialise des formes SFML simples pour les portes.
// - Fournit des accesseurs pour la pièce courante et ses ennemis/portes.
class RoomManager {
private:
    std::map<int, Room> rooms_;
    int currentRoomIndex_ = -1;
    float screenW, screenH;
    const float DOOR_SIZE = 120.f;
    const float DOOR_THICKNESS = 100.f;

public:
    // Constructeur : reçoit la taille d'écran pour positionner les portes
    RoomManager(float w, float h);

    // Charge et parse le JSON. Retourne true si réussi.
    bool loadRoomsFromJson(const std::string& filename);

    // Initialise des formes SFML (visualShape) pour une pièce (portes)
    void initializeRoomShapes(Room& room);

    // Accesseurs
    const std::vector<Door>& getCurrentRoomDoors() const;
    std::string getCurrentRoomName() const;
    bool changeRoom(int newRoomIndex, const std::string& entryDirection, sf::RectangleShape& joueur);
    const std::vector<EnemyDefinition>& getCurrentRoomEnemies() const;

    int getCurrentRoomIndex() const { return currentRoomIndex_; }
    std::map<int, Room>& getRooms() { return rooms_; }
    float getScreenW() const { return screenW; }
    float getScreenH() const { return screenH; }
};

} // namespace Modele
