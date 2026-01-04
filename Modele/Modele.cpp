
#include "Modele.h"
#include "Enemy.h"
#include "RoomManager.h"
#include "Level.h"
#include <cmath>
#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "Objective.h"

// Namespace alias for nlohmann/json
using json = nlohmann::json;

namespace Modele {

    const float DOOR_MARGIN = 20.f;

    // Room loading is now handled by RoomManager.

    // Constructor: initializes all main subsystems and default game state.
    Modele::Modele()
    : collisionDetectee(false), currentLevel(std::make_unique<Level>("Tutorial", "Test level"))
    {
        // Get desktop resolution (used to initialize RoomManager scaling, door placement, etc.)
        sf::VideoMode dm = sf::VideoMode::getDesktopMode();
        float screenW = static_cast<float>(dm.width);
        float screenH = static_cast<float>(dm.height);

        // RoomManager owns room data (doors, obstacles, enemy definitions, objectives) and handles JSON loading.
        roomManager = std::make_unique<RoomManager>(screenW, screenH);

        // MapManager handles floor/tile matrix and tile/wall textures.
        mapManager = std::make_unique<MapManager>();

        // Initialize enemy prototypes (Prototype design pattern).
        // These prototypes are cloned later to create runtime enemies from EnemyDefinition data.
        enemyPrototypes.clear();
        enemyPrototypes["generic"] = std::make_unique<GenericEnemy>();
        enemyPrototypes["camera"]  = std::make_unique<CameraEnemy>();
        enemyPrototypes["laser"]   = std::make_unique<LaserEnemy>();

        // Compute an approximate player "box size" based on screen size (minimum 8 pixels).
        float boxSize = std::max(8.f, std::min(screenW, screenH) * 0.08f);

        // Variables kept from older hitbox scaling logic (currently unused in this constructor).
        float playerCollisionW = boxSize * playerSpriteDisplayScaleX;
        float playerCollisionH = boxSize * playerSpriteDisplayScaleY;
        const float HITBOX_REDUCTION_FACTOR = 0.8f;

        // Player hitbox (collision shape). Visual sprite is handled separately.
        joueur.setSize(sf::Vector2f(46, 130));
        joueur.setFillColor(sf::Color::Blue);

        // --- Load player sprite-sheet ---
        // Try multiple paths to support different working directories (IDE/build output).
        const std::vector<std::string> tryPlayerPaths = {
            "cmake-build-debug/Asset/Human/james_adams_textures.png",
            "Asset/Human/james_adams_textures.png",
            "Human/james_adams_textures.png",
            "james_adams_textures.png"
        };

        bool playerLoaded = false;
        for (const auto& p : tryPlayerPaths) {
            if (playerTexture.loadFromFile(p)) {
                playerLoaded = true;

                // Bind texture to sprite immediately and set initial frame.
                playerSprite.setTexture(playerTexture);
                playerSprite.setTextureRect(computePlayerTextureRect());
                break;
            }
        }

        if (!playerLoaded) {
            // Player texture failed to load (debug logging removed).
        } else {
            playerSprite.setTexture(playerTexture);

            // Initialize animation state (frame 0, default row).
            playerFrameIndex = 0;
            playerClock.restart();

            // Initialize texture rect using the current zoom/crop logic.
            playerSprite.setTextureRect(computePlayerTextureRect());

            // Sprite origin will be updated properly in syncPlayerSprite() based on crop size.
            playerSprite.setOrigin(0.f, 0.f);

            // Temporary positioning centered on the hitbox (final alignment done in syncPlayerSprite()).
            sf::Vector2f ppos = joueur.getPosition();
            playerSprite.setPosition(ppos.x + joueur.getSize().x / 2.f,
                                     ppos.y + joueur.getSize().y / 2.f);

            // Synchronize sprite position/scale/origin with the hitbox.
            syncPlayerSprite();
        }

        // Default asset search paths for floors and walls (robust to build/run directories).
        const std::vector<std::string> tryFloorPaths = {
            "cmake-build-debug/Asset/Floor/floor_01.png",
            "Asset/Floor/floor_01.png",
            "floor_01.png"
        };

        const std::vector<std::string> tryFloor02Paths = {
            "cmake-build-debug/Asset/Floor/floor_02.png",
            "Asset/Floor/floor_02.png",
            "floor_02.png"
        };

        const std::vector<std::string> tryWallPaths = {
            "cmake-build-debug/Asset/Wall/Wall1_2.png",
            "cmake-build-debug/Asset/Wall/Wall1_3.png",
            "cmake-build-debug/Asset/Wall/Wall1_4.png",
            "cmake-build-debug/Asset/Wall/Wall1_8.png"
        };

        if (mapManager) {
            mapManager->loadDefaults(tryFloorPaths, tryFloor02Paths, tryWallPaths);
        }

        // Wall/floor tile codes have been moved to MapManager.
        // Floor tile registrations (e.g., floor_01 -> 22, floor_02 -> 21) are handled in MapManager::loadDefaults().
        //
        // NOTE:
        // By default we do not automatically assign wall tile codes (11..18) into the floor matrix.
        // Walls should be defined explicitly by level data (JSON) or dedicated logic.

        // Load rooms using RoomManager.
        currentLevelPath = "Asset/levels/tutorial/tutorial.json";
        if (roomManager->loadRoomsFromJson(currentLevelPath) && roomManager->getRooms().count(0))
        {
            // Switch to room 0; RoomManager may reposition the player depending on entry direction.
            roomManager->changeRoom(0, "", joueur);

            // Default spawn: place the player at screen center.
            joueur.setPosition(roomManager->getScreenW() * 0.5f - boxSize * 0.5f,
                               roomManager->getScreenH() * 0.5f - boxSize * 0.5f);

            // Keep sprite aligned with hitbox.
            syncPlayerSprite();

            // If room 0 references an external map file, load it into MapManager.
            auto& rooms = roomManager->getRooms();
            auto it = rooms.find(0);
            if (it != rooms.end() && !it->second.mapFile.empty() && mapManager) {
                if (!mapManager->loadMapFromFile(it->second.mapFile)) {
                    std::cerr << "Warning: failed to load map file '"
                              << it->second.mapFile << "' for room 0" << std::endl;
                }
            }

            // Instantiate runtime enemies for current room (definitions -> instances).
            reloadEnemiesForCurrentRoom();
        }
        else
        {
            // Fallback if level/room loading failed.
            std::cerr << "Échec du chargement de la carte. Pièce 0 non valide." << std::endl;
            roomManager->changeRoom(-1, "", joueur);

            joueur.setPosition(roomManager->getScreenW() * 0.5f - boxSize * 0.5f,
                               roomManager->getScreenH() * 0.5f - boxSize * 0.5f);

            // Align sprite with hitbox (if texture loaded).
            syncPlayerSprite();
        }

        // Example patrol points (kept locally; obstacle Agent system is currently deprecated).
        std::vector<sf::Vector2f> patrouillePoints = {
            sf::Vector2f(screenW * 0.125f, screenH * 0.1666667f),
            sf::Vector2f(screenW * 0.75f,  screenH * 0.1666667f),
            sf::Vector2f(screenW * 0.75f,  screenH * 0.6666667f),
            sf::Vector2f(screenW * 0.125f, screenH * 0.6666667f)
        };

        // Obstacle-related AI/Agent removed; patrol points kept local for potential future use.
    }

    void Modele::reloadEnemiesForCurrentRoom()
    {
        // Rebuild the runtime enemy list based on EnemyDefinition descriptors of the current room.
        enemies.clear();

        // Reference resolution used when authoring JSON coordinates.
        float refW = 2560.f;
        float refH = 1440.f;

        // Convert reference coordinates into current screen coordinates.
        float scaleW = getScreenW() / refW;
        float scaleH = getScreenH() / refH;

        // Create each enemy instance via the factory (Prototype cloning + configuration).
        for (const auto& ed : roomManager->getCurrentRoomEnemies()) {
            auto e = createEnemyFromDefinition(ed, enemyPrototypes, scaleW, scaleH);
            if (e) enemies.push_back(std::move(e));
        }
    }

    void Modele::updateEnemies()
    {
        // Update all enemies per frame:
        // - update() for AI/movement
        // - updateAnimation() for sprite-sheet
        // - detectPlayer() to update per-enemy detection state
        for (auto& e : enemies) {
            e->update();
            e->updateAnimation();
            e->detectPlayer(joueur);
        }
    }

    // Obstacle AI update (Agent-based obstacle AI was removed; currently no-op).
    void Modele::mettreAJourObstacles()
    {
        // Intentionally left empty: obstacle AI handled elsewhere or deprecated.
    }

    const std::vector<std::unique_ptr<sf::Shape>>& Modele::getObstacleShapes() const
    {
        // Return obstacle shapes for current room (or an empty static vector if unavailable).
        static const std::vector<std::unique_ptr<sf::Shape>> empty;
        if (!roomManager) return empty;

        auto& rooms = roomManager->getRooms();
        int idx = roomManager->getCurrentRoomIndex();
        auto it = rooms.find(idx);

        if (it != rooms.end()) return it->second.obstacleShapes;
        return empty;
    }

    const std::vector<Door>& Modele::getCurrentRoomDoors() const
    {
        return roomManager->getCurrentRoomDoors();
    }

    std::string Modele::getCurrentRoomName() const
    {
        return roomManager->getCurrentRoomName();
    }

    std::vector<Objective>& Modele::getCurrentRoomObjectives()
    {
        // Return objectives by reference to allow updating them (e.g., set accomplished).
        static std::vector<Objective> empty;
        if (!roomManager) return empty;

        auto& rooms = roomManager->getRooms();
        int idx = roomManager->getCurrentRoomIndex();
        auto it = rooms.find(idx);

        if (it != rooms.end()) return it->second.objectives;
        return empty;
    }

    std::vector<Objective> Modele::getAllLevelObjectives() const
    {
        // Return a flattened copy of all objectives across all rooms.
        std::vector<Objective> allObjectives;
        if (!roomManager) return allObjectives;

        auto& rooms = roomManager->getRooms();
        for (auto& [idx, room] : rooms) {
            for (auto& obj : room.objectives) {
                allObjectives.push_back(obj);
            }
        }

        return allObjectives;
    }

    const std::vector<std::unique_ptr<Enemy>>& Modele::getEnemies() const
    {
        return enemies;
    }

    sf::Vector2f Modele::getObstacleCenter(size_t idx) const
    {
        // Compute center from the obstacle's global bounds (AABB center).
        if (!roomManager) return sf::Vector2f();
        auto& rooms = roomManager->getRooms();
        int ridx = roomManager->getCurrentRoomIndex();
        auto it = rooms.find(ridx);

        if (it == rooms.end()) return sf::Vector2f();
        const auto& shapes = it->second.obstacleShapes;

        if (idx >= shapes.size() || !shapes[idx]) return sf::Vector2f();

        sf::FloatRect b = shapes[idx]->getGlobalBounds();
        return sf::Vector2f(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
    }

    sf::Vector2f Modele::getObstacleForward(size_t /*idx*/) const
    {
        // Obstacle facing/orientation was part of the old Agent system.
        // Return a safe default direction (right).
        // If you need real facing, compute/store it per obstacle (e.g., in RoomManager).
        return sf::Vector2f(1.f, 0.f);
    }

    float Modele::getScreenW() const { return roomManager ? roomManager->getScreenW() : 0.f; }
    float Modele::getScreenH() const { return roomManager ? roomManager->getScreenH() : 0.f; }

    // Compute the current player texture rect (center-cropped) based on playerTextureZoom.
    // Sheet layout assumption:
    // - Movement frames are on rows 1..4
    // - Idle frames are on rows 5..8 (i.e., playerRow + 4), columns 0..idleFrameCount-1
    sf::IntRect Modele::computePlayerTextureRect() const
    {
        int frameSize = playerTileSize;

        // Zoom-in is implemented via smaller crop inside each tile (center crop).
        int cropSize = static_cast<int>(std::round(frameSize / playerTextureZoom));
        if (cropSize < 1) cropSize = 1;

        // Movement frames are the first (playerFrameCount - idleFrameCount) columns.
        int movementFramesCount = std::max(1, playerFrameCount - idleFrameCount);

        int col = 0;
        int rowIndex = std::max(0, playerRow - 1);

        if (playerIsMoving)
        {
            // Movement columns: 0 .. movementFramesCount-1
            col = playerFrameIndex % movementFramesCount;
            rowIndex = std::max(0, playerRow - 1); // 0..3
        }
        else
        {
            // Idle frames use separate rows (playerRow + 4) and only idleFrameCount columns.
            int idleCols = std::max(1, idleFrameCount);
            col = playerFrameIndex % idleCols;
            rowIndex = std::max(0, playerRow - 1) + 4;
        }

        // Base tile top-left in texture.
        int frameX = col * frameSize;
        int frameY = rowIndex * frameSize;

        // Center crop inside tile.
        int offsetX = frameX + (frameSize - cropSize) / 2;
        int offsetY = frameY + (frameSize - cropSize) / 2;

        return sf::IntRect(offsetX, offsetY, cropSize, cropSize);
    }

    void Modele::setPlayerDirection(int row)
    {
        // Clamp valid row range (1..4).
        if (row < 1) row = 1;
        if (row > 4) row = 4;

        // If direction row changes, reset animation state.
        if (playerRow != row) {
            playerRow = row;
            playerFrameIndex = 0;
            playerClock.restart();

            // Update rect immediately if texture is loaded.
            if (playerTexture.getSize().x > 0)
                playerSprite.setTextureRect(computePlayerTextureRect());
        }
    }

    void Modele::updatePlayerAnimation(bool moving)
    {
        // Do nothing if no texture loaded.
        if (playerTexture.getSize().x == 0) return;

        int movementFramesCount = std::max(1, playerFrameCount - idleFrameCount);
        int idleCols = std::max(1, idleFrameCount);

        // Select duration depending on moving vs idle.
        float frameDuration = moving ? playerFrameDuration : playerIdleFrameDuration;
        float elapsed = playerClock.getElapsedTime().asSeconds();

        // If state changed, reset frame index and restart timing.
        if (moving != playerIsMoving)
        {
            playerIsMoving = moving;
            playerFrameIndex = 0;
            playerSprite.setTextureRect(computePlayerTextureRect());
            playerClock.restart();
            // Intentionally do not return: we allow an immediate tick if elapsed is large.
        }

        if (playerIsMoving)
        {
            // Movement animation cadence.
            if (elapsed >= playerFrameDuration)
            {
                playerFrameIndex = (playerFrameIndex + 1) % movementFramesCount;
                playerSprite.setTextureRect(computePlayerTextureRect());
                playerClock.restart();
            }
        }
        else // idle
        {
            // Idle animation cadence.
            if (idleCols <= 0)
            {
                // Defensive fallback (idleCols should never be <=0 due to max(1,...)).
                playerFrameIndex = 0;
                playerSprite.setTextureRect(computePlayerTextureRect());
                playerClock.restart();
                return;
            }

            if (elapsed >= playerIdleFrameDuration)
            {
                playerFrameIndex = (playerFrameIndex + 1) % idleCols;
                playerSprite.setTextureRect(computePlayerTextureRect());
                playerClock.restart();
            }
        }
    }

    // Synchronize player sprite scale/origin/position with the player hitbox rectangle.
    void Modele::syncPlayerSprite()
    {
        if (playerTexture.getSize().x == 0) return;

        // Hitbox size (used for centering).
        sf::Vector2f size = joueur.getSize();

        // Current crop rect; crop is square.
        sf::IntRect rect = computePlayerTextureRect();
        int cropSize = rect.width;

        // Do not scale sprite to match hitbox size; keep a fixed scale for visuals.
        // This may result in sprite extending outside hitbox (visual-only).
        playerSprite.setScale(2.5f, 2.5f);

        // Origin at crop center (pre-scale texture space).
        playerSprite.setOrigin(static_cast<float>(cropSize) * 0.5f,
                               static_cast<float>(cropSize) * 0.5f);

        // Center sprite on hitbox.
        sf::Vector2f pos = joueur.getPosition();
        playerSprite.setPosition(pos.x + size.x * 0.5f,
                                 pos.y + size.y * 0.5f);
    }

    // Helper declaration (implementation elsewhere or intentionally omitted).
    // The controller is expected to call syncPlayerSprite() after player movement.
    void syncPlayerSpritePosition(Modele& m);

    // Collision flag accessors.
    void Modele::setCollisionDetectee(bool v) { collisionDetectee = v; }
    bool Modele::isCollisionDetectee() const { return collisionDetectee; }

    void Modele::setJoueurDetecte(bool v) { joueurDetecte = v; }

    bool Modele::isJoueurDetecte() const
    {
        // Global detection is the aggregation of per-enemy detection:
        // if any enemy has joueurDetecte == true, the player is considered detected.
        for (const auto& e : enemies)
            if (e->joueurDetecte) return true;

        return false;
    }

    bool Modele::changeRoom(int newRoomIndex, const std::string& entryDirection)
    {
        // Change room via RoomManager (also repositions player based on entry direction).
        bool ok = roomManager->changeRoom(newRoomIndex, entryDirection, joueur);

        // Reset collision/detection flags when entering a new room.
        setCollisionDetectee(false);
        setJoueurDetecte(false);

        if (ok) {
            // Load map only if it exists; otherwise clear map.
            auto& rooms = roomManager->getRooms();
            auto it = rooms.find(newRoomIndex);

            if (it != rooms.end() && !it->second.mapFile.empty()) {
                if (!mapManager->loadMapFromFile(it->second.mapFile)) {
                    std::cerr << "Warning: failed to load map file '"
                              << it->second.mapFile << "' for room "
                              << newRoomIndex << std::endl;
                }
            } else {
                // No map file => clear current map.
                mapManager->clearMap();
            }

            // Rebuild enemies for the new room.
            reloadEnemiesForCurrentRoom();
        }

        return ok;
    }

    int Modele::getCurrentRoomIndex() const
    {
        if (!roomManager) return -1;
        return roomManager->getCurrentRoomIndex();
    }

    std::string Modele::getCurrentRoomDialogueRef() const
    {
        if (!roomManager) return std::string();
        return roomManager->getCurrentRoomDialogueRef();
    }

    bool Modele::isCurrentRoomDialogueShown() const
    {
        // Consider dialogue shown if either the RoomManager marks it shown
        // or if we have recorded the room index in `shownRoomDialogues`.
        if (!roomManager) return false;
        int idx = roomManager->getCurrentRoomIndex();
        if (roomManager->isCurrentRoomDialogueShown()) return true;
        if (shownRoomDialogues.find(idx) != shownRoomDialogues.end()) return true;
        return false;
    }

    void Modele::markCurrentRoomDialogueShown()
    {
        if (!roomManager) return;
        roomManager->markCurrentRoomDialogueShown();
        int idx = roomManager->getCurrentRoomIndex();
        shownRoomDialogues.insert(idx);
    }

    // Objective contact accessors (pointer-based so original objective can be modified).
    void Modele::setObjectiveContact(Objective* obj)
    {
        objectiveContact = obj;
        if (objectiveContact) {
            std::cout << "[Modele] setObjectiveContact -> " << objectiveContact->getTitle()
                      << " cesar=" << objectiveContact->isCesar()
                      << " code=" << objectiveContact->getCode() << std::endl;
        }
    }

    void Modele::setObjectiveContactDetectee(const bool b) { objectiveContactDetectee = b; }
    Objective* Modele::getObjectiveContact() const { return objectiveContact; }
    bool Modele::getObjectiveContactDetectee() const { return objectiveContactDetectee; }

    bool Modele::hasDialogueTriggered() const { return dialogueTriggeredFlag; }
    void Modele::setDialogueTriggered(bool v) { dialogueTriggeredFlag = v; }
    void Modele::resetDialogueTriggered() { dialogueTriggeredFlag = false; }

    bool Modele::setTileTexture(int id, const std::string& path)
    {
        if (!mapManager) return false;
        return mapManager->setTileTexture(id, path);
    }

    // Player score API (in-memory only).
    void Modele::setPlayerScore(int levelIndex, int score)
    {
        if (levelIndex < 0 || levelIndex >= static_cast<int>(playerScores.size())) return;

        // Only store the best score achieved so far.
        if (score > playerScores[levelIndex]) playerScores[levelIndex] = score;
    }

    std::vector<int> Modele::getPlayerScores() const
    {
        return playerScores;
    }

    bool Modele::loadLevelFromFile(const std::string& levelJsonPath)
    {
        if (levelJsonPath.empty()) return false;

        float screenW = getScreenW();
        float screenH = getScreenH();

        // Build candidate paths for robust loading across different directory layouts.
        std::vector<std::string> candidates;
        candidates.push_back(levelJsonPath);
        candidates.push_back(std::string("cmake-build-debug/") + levelJsonPath);
        candidates.push_back(std::string("./") + levelJsonPath);

        // Extract basename (file name only).
        std::string basename = levelJsonPath;
        size_t pos = basename.find_last_of("/\\");
        if (pos != std::string::npos) basename = basename.substr(pos + 1);

        candidates.push_back(std::string("Asset/levels/") + basename);
        candidates.push_back(std::string("Asset/levels/tutorial/") + basename);
        candidates.push_back(std::string("Asset/levels/OH/") + basename);

        // Also try build-output variant.
        candidates.push_back(std::string("cmake-build-debug/Asset/levels/") + basename);

        // Search one directory level deep under Asset/levels for the basename.
        try {
            namespace fs = std::filesystem;
            fs::path levelsDir("Asset/levels");
            if (fs::exists(levelsDir) && fs::is_directory(levelsDir)) {
                for (const auto &entry : fs::directory_iterator(levelsDir)) {
                    if (fs::is_directory(entry.path())) {
                        fs::path p = entry.path() / basename;
                        candidates.push_back(p.string());
                    }
                }
            }
        } catch (...) {
            // Ignore filesystem errors.
        }

        // Pick the first existing file from candidates.
        std::string resolved;
        for (const auto &c : candidates) {
            try {
                if (std::ifstream(c).good()) { resolved = c; break; }
            } catch(...) {}
        }

        if (resolved.empty()) {
            std::cerr << "Modele::loadLevelFromFile: could not find level file for '"
                      << levelJsonPath << "' (tried candidates)" << std::endl;
            for (const auto &c : candidates) std::cerr << " tried: " << c << std::endl;
            return false;
        }

        // Store resolved level path for future reset/reload.
        currentLevelPath = resolved;

        // Recreate RoomManager to ensure a clean room state.
        roomManager.reset();
        roomManager = std::make_unique<RoomManager>(screenW, screenH);

        // Load rooms and ensure room 0 exists.
        if (!roomManager->loadRoomsFromJson(currentLevelPath) || !roomManager->getRooms().count(0)) {
            std::cerr << "Modele::loadLevelFromFile: failed to load rooms from '"
                      << currentLevelPath << "'" << std::endl;
            return false;
        }

        // Enter room 0 (RoomManager positions player based on entry direction).
        roomManager->changeRoom(0, "", joueur);

        // If room 0 references an external map file, load it; otherwise clear map.
        auto& rooms = roomManager->getRooms();
        auto it = rooms.find(0);
        if (it != rooms.end() && !it->second.mapFile.empty() && mapManager) {
            if (!mapManager->loadMapFromFile(it->second.mapFile)) {
                std::cerr << "Warning: failed to load map file '"
                          << it->second.mapFile << "' for room 0" << std::endl;
            }
        } else {
            if (mapManager) mapManager->clearMap();
        }

        // Reload enemies for current room.
        reloadEnemiesForCurrentRoom();

        // Ensure sprite aligns with hitbox after loading.
        syncPlayerSprite();

        return true;
    }

    const sf::Texture* Modele::getTileTexture(int id) const
    {
        if (!mapManager) return nullptr;
        return mapManager->getTileTexture(id);
    }

    const std::vector<std::vector<int>>& Modele::getFloorMatrix() const
    {
        static const std::vector<std::vector<int>> empty;
        if (!mapManager) return empty;
        return mapManager->getFloorMatrix();
    }

    const sf::Texture& Modele::getFloorTexture() const
    {
        static sf::Texture dummy;
        if (!mapManager) return dummy;
        return mapManager->getFloorTexture();
    }

    void Modele::setFloorMatrix(const std::vector<std::vector<int>>& m)
    {
        if (mapManager) mapManager->setFloorMatrix(m);
    }

    int Modele::getLives() const
    {
        return currentLevel ? currentLevel->getLives() : 0;
    }

    void Modele::loseLives(int amount)
    {
        if (currentLevel) {
            currentLevel->loseLives(amount);
        }
    }

    bool Modele::isGameOver() const
    {
        return currentLevel ? currentLevel->isGameOver() : false;
    }

    int Modele::getDetectionCount() const
    {
        return detectionCount;
    }

    void Modele::incrementDetectionCount()
    {
        detectionCount++;
    }

    void Modele::resetDetectionCount()
    {
        detectionCount = 0;
    }

    int Modele::getTileSize() const
    {
        return mapManager ? mapManager->getTileSize() : 0;
    }

    const std::vector<sf::Texture>& Modele::getWallTextures() const
    {
        static const std::vector<sf::Texture> empty;
        if (!mapManager) return empty;
        return mapManager->getWallTextures();
    }

    // Reset the model state (player, enemies, objectives, etc.).
    void Modele::reset()
    {
        objectiveContact = nullptr;
        objectiveContactDetectee = false;

        // Compute a start position for the player (room 0).
        float startX = 0.f, startY = 0.f;

        if (roomManager && roomManager->getRooms().count(0)) {
            auto& room0 = roomManager->getRooms()[0];

            // Optional approach: use an objective named "player_start" as spawn marker.
            if (!room0.objectives.empty()) {
                for (const auto& obj : room0.objectives) {
                    if (obj.getTitle() == "player_start") {
                        startX = obj.getHitboxPosition().x;
                        startY = obj.getHitboxPosition().y;
                        break;
                    }
                }
            }

            // Fallback: center screen if no marker found.
            if (startX == 0.f && startY == 0.f) {
                float boxSize = joueur.getSize().x;
                startX = roomManager->getScreenW() * 0.5f - boxSize * 0.5f;
                startY = roomManager->getScreenH() * 0.5f - boxSize * 0.5f;
            }
        }

        // Apply start position.
        joueur.setPosition(startX, startY);

        // Reset animation state.
        playerFrameIndex = 0;
        playerRow = 3;
        playerIsMoving = false;

        // Reset all objectives in all rooms.
        if (roomManager) {
            for (auto& [idx, room] : roomManager->getRooms()) {
                for (auto& obj : room.objectives) {
                    obj.setAccomplished(false);

                    // Preserve Cesar metadata (project-specific).
                    obj.setCesar(obj.isCesar());
                    obj.setCode(obj.getCode());
                    obj.setchangeValue(obj.getChangeValue());
                }
            }
        }

        // Reset flags.
        collisionDetectee = false;
        joueurDetecte = false;
        objectiveContactDetectee = false;
        dialogueTriggeredFlag = false;

        // Reset lives to 3.
        if (currentLevel) {
            currentLevel->setLives(3);
        }

        // Reset player position and animation (note: this overrides startX/startY).
        joueur.setPosition(0.f, 0.f);
        playerFrameIndex = 0;
        playerRow = 3;
        playerClock.restart();
        playerIsMoving = false;


        // Destroy and recreate RoomManager to guarantee a fresh level state.
        float screenW = getScreenW();
        float screenH = getScreenH();

        // Clearing the RoomManager will also reset per-room dialogue flags;
        // ensure we clear our session-level record as well so dialogues may
        // be shown again after a full reset.
        shownRoomDialogues.clear();
        // Reset detection counter when restarting the level
        detectionCount = 0;
        roomManager.reset();
        roomManager = std::make_unique<RoomManager>(screenW, screenH);

        // Reload rooms from stored level path and enter room 0 if possible.
        if (!currentLevelPath.empty()
            && roomManager->loadRoomsFromJson(currentLevelPath)
            && roomManager->getRooms().count(0))
        {
            roomManager->changeRoom(0, "", joueur);
        }

        // Rebuild enemies for the current room.
        enemies.clear();
        reloadEnemiesForCurrentRoom();

        // Align sprite with hitbox after reset.
        syncPlayerSprite();
    }

} // namespace Modele

