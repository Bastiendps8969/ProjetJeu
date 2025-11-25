#include "Modele.h"
#include <cmath>
#include <limits>

#include "Player.h"

namespace Modele {
    // Constructeur : création des formes obstacles rouges
    // qui sont ajoutées au vecteur obstacles de pointeurs d'obstacles
    Modele::Modele()
    {
        player = Player();
        agent = Agent();

        // Détermine la résolution du bureau pour adapter tailles et points de patrouille
        sf::VideoMode dm = sf::VideoMode::getDesktopMode();
        float W = static_cast<float>(dm.width);
        float H = static_cast<float>(dm.height);
        // Taille relative (approx 64 sur 800 => 0.08)
        float boxSize = std::max(8.f, std::min(W, H) * 0.08f);

        // Création du rectangle joueur (taille adaptée)
        joueur.setSize(sf::Vector2f(boxSize, boxSize));
        joueur.setFillColor(sf::Color::Green);
        joueur.setPosition(W * 0.125f, H * 0.1666667f); // position initiale relative

        // Création d'un seul carré rouge (obstacle) taille adaptée
        sf::RectangleShape* carre = new sf::RectangleShape(sf::Vector2f(boxSize, boxSize));
        carre->setFillColor(sf::Color::Red);
        carre->setPosition(W * 0.25f, H * 0.25f);
        obstacles.emplace_back(carre);

        obstacleVitesse = sf::Vector2f(0.3f, 0.2f);

        // Initialisation des points de patrouille
        // Points relatifs tirés des fractions utilisées précédemment (100/800=0.125, 600/800=0.75, 100/600~0.1667, 400/600~0.6667)
        pointsPatrouille = {
            sf::Vector2f(W * 0.125f, H * 0.1666667f),
            sf::Vector2f(W * 0.75f,  H * 0.1666667f),
            sf::Vector2f(W * 0.75f,  H * 0.6666667f),
            sf::Vector2f(W * 0.125f, H * 0.6666667f)
        };
        pointCibleIndex = 0;
        vitessePatrouille = 0.125f;  // Plus cette valeur est grande, plus le carré se déplace rapidement

        // Initialisation du flag de collision
        collisionDetectee = false;

        // Initialisation du flag de détection joueur (champ de vision)
        joueurDetecte = false;
    }

    // Destructeur
    Modele::~Modele()
    {
        for (auto obs : obstacles)
        {   delete obs;         }
    }

    Player Modele::getPlayer() {
        return player;
    }
    Agent Modele::getAgent() {
        return agent;
    }

    void Modele::mettreAJourObstacles()
    {
        // Position actuelle de l'obstacle
        sf::Vector2f position = obstacles[0]->getPosition();
        
        // Point cible actuel
        sf::Vector2f cible = pointsPatrouille[pointCibleIndex];
        
        // Calculer la direction vers le point cible
        sf::Vector2f direction = cible - position;
        float distance = sqrt(direction.x * direction.x + direction.y * direction.y);
        
        // Récupération des boîtes englobantes
        sf::FloatRect joueurBounds = joueur.getGlobalBounds();
        sf::FloatRect obstacleBounds = obstacles[0]->getGlobalBounds();

        // Paramètres du champ de vision (portée doublée)
        const float fovRange = 440.0f;         // portée du champ de vision en pixels
        const float fovAngleDeg = 60.0f;       // angle total du cône en degrés
        const float cosHalfFov = std::cos((fovAngleDeg * 0.5f) * 3.14159265f / 180.0f);

        // Si on est assez proche du point cible, passer au suivant
        if (distance < 5.0f) {
            pointCibleIndex = (pointCibleIndex + 1) % pointsPatrouille.size();
            // Met à jour l'état de collision (si déjà en contact)
            collisionDetectee = obstacleBounds.intersects(joueurBounds) &&
                (std::min(obstacleBounds.left + obstacleBounds.width, joueurBounds.left + joueurBounds.width)
                 - std::max(obstacleBounds.left, joueurBounds.left) > 0.0f) &&
                (std::min(obstacleBounds.top + obstacleBounds.height, joueurBounds.top + joueurBounds.height)
                 - std::max(obstacleBounds.top, joueurBounds.top) > 0.0f);
            // Met à jour détection joueur (recalcule au repos)
            // calculer vecteur centre->centre
            {
                sf::Vector2f obstacleCenter(obstacleBounds.left + obstacleBounds.width * 0.5f,
                                            obstacleBounds.top  + obstacleBounds.height * 0.5f);
                sf::Vector2f joueurCenter(joueurBounds.left + joueurBounds.width * 0.5f,
                                          joueurBounds.top  + joueurBounds.height * 0.5f);
                sf::Vector2f toJoueur = joueurCenter - obstacleCenter;
                float distJ = std::sqrt(toJoueur.x*toJoueur.x + toJoueur.y*toJoueur.y);
                if (distJ <= 0.0f) { joueurDetecte = true; }
                else {
                    // si obstacle immobile, on considère la direction vers la prochaine cible (déjà nulle ici)
                    sf::Vector2f forward = (distance > 0.0001f) ? (direction / distance) : sf::Vector2f(1.f, 0.f);
                    sf::Vector2f toJNorm = sf::Vector2f(toJoueur.x / distJ, toJoueur.y / distJ);
                    float dot = forward.x * toJNorm.x + forward.y * toJNorm.y;
                    joueurDetecte = (distJ <= fovRange && dot >= cosHalfFov);
                }
            }
            return;
        }

        // Calcul du petit pas vers la cible
        sf::Vector2f directionNorm = direction / distance;
        sf::Vector2f deplacement = directionNorm * vitessePatrouille;

        // Swept AABB pour trouver le premier instant de contact dans [0,1]
        float txEntry, tyEntry, txExit, tyExit;
        const float INF_NEG = -std::numeric_limits<float>::infinity();
        const float INF_POS =  std::numeric_limits<float>::infinity();

        if (deplacement.x > 0.0f) {
            float invEntryX = joueurBounds.left - (obstacleBounds.left + obstacleBounds.width);
            float invExitX  = (joueurBounds.left + joueurBounds.width) - obstacleBounds.left;
            txEntry = invEntryX / deplacement.x;
            txExit  = invExitX  / deplacement.x;
        } else if (deplacement.x < 0.0f) {
            float invEntryX = (joueurBounds.left + joueurBounds.width) - obstacleBounds.left;
            float invExitX  = joueurBounds.left - (obstacleBounds.left + obstacleBounds.width);
            txEntry = invEntryX / deplacement.x;
            txExit  = invExitX  / deplacement.x;
        } else {
            txEntry = INF_NEG;
            txExit  = INF_POS;
        }

        if (deplacement.y > 0.0f) {
            float invEntryY = joueurBounds.top - (obstacleBounds.top + obstacleBounds.height);
            float invExitY  = (joueurBounds.top + joueurBounds.height) - obstacleBounds.top;
            tyEntry = invEntryY / deplacement.y;
            tyExit  = invExitY  / deplacement.y;
        } else if (deplacement.y < 0.0f) {
            float invEntryY = (joueurBounds.top + joueurBounds.height) - obstacleBounds.top;
            float invExitY  = joueurBounds.top - (obstacleBounds.top + obstacleBounds.height);
            tyEntry = invEntryY / deplacement.y;
            tyExit  = invExitY  / deplacement.y;
        } else {
            tyEntry = INF_NEG;
            tyExit  = INF_POS;
        }

        float tEntry = std::max(txEntry, tyEntry);
        float tExit  = std::min(txExit, tyExit);

        // Collision projetée si tEntry dans [0,1] et tEntry <= tExit
        if (tEntry >= 0.0f && tEntry <= 1.0f && tEntry <= tExit) {
            // Déplacement jusqu'au contact exact (pas de chevauchement)
            sf::Vector2f deplacementContact = deplacement * tEntry;
            obstacles[0]->setPosition(position + deplacementContact);
            collisionDetectee = true;
            // Mettre à jour la détection joueur en utilisant la directionNorm
            {
                sf::FloatRect newObsBounds = obstacles[0]->getGlobalBounds();
                sf::Vector2f obstacleCenter(newObsBounds.left + newObsBounds.width * 0.5f,
                                            newObsBounds.top  + newObsBounds.height * 0.5f);
                sf::Vector2f joueurCenter(joueurBounds.left + joueurBounds.width * 0.5f,
                                          joueurBounds.top  + joueurBounds.height * 0.5f);
                sf::Vector2f toJoueur = joueurCenter - obstacleCenter;
                float distJ = std::sqrt(toJoueur.x*toJoueur.x + toJoueur.y*toJoueur.y);
                if (distJ <= 0.0f) joueurDetecte = true;
                else {
                    sf::Vector2f toJNorm = sf::Vector2f(toJoueur.x / distJ, toJoueur.y / distJ);
                    float dot = directionNorm.x * toJNorm.x + directionNorm.y * toJNorm.y;
                    joueurDetecte = (distJ <= fovRange && dot >= cosHalfFov);
                }
            }
            // Ne change pas pointCibleIndex ici : l'obstacle est bloqué par le joueur
        } else {
            // Aucun contact sur ce pas : bouger normalement
            obstacles[0]->setPosition(position + deplacement);
            // Vérifier s'il y a chevauchement (rare, mais par sécurité)
            sf::FloatRect nouveauBounds = obstacles[0]->getGlobalBounds();
            float overlapW = std::min(nouveauBounds.left + nouveauBounds.width, joueurBounds.left + joueurBounds.width)
                             - std::max(nouveauBounds.left, joueurBounds.left);
            float overlapH = std::min(nouveauBounds.top + nouveauBounds.height, joueurBounds.top + joueurBounds.height)
                             - std::max(nouveauBounds.top, joueurBounds.top);
            collisionDetectee = (overlapW > 0.0f && overlapH > 0.0f);

            // Mettre à jour la détection joueur en utilisant la directionNorm nouvellement calculée
            {
                sf::FloatRect newObsBounds = obstacles[0]->getGlobalBounds();
                sf::Vector2f obstacleCenter(newObsBounds.left + newObsBounds.width * 0.5f,
                                            newObsBounds.top  + newObsBounds.height * 0.5f);
                sf::Vector2f joueurCenter(joueurBounds.left + joueurBounds.width * 0.5f,
                                          joueurBounds.top  + joueurBounds.height * 0.5f);
                sf::Vector2f toJoueur = joueurCenter - obstacleCenter;
                float distJ = std::sqrt(toJoueur.x*toJoueur.x + toJoueur.y*toJoueur.y);
                if (distJ <= 0.0f) joueurDetecte = true;
                else {
                    sf::Vector2f toJNorm = sf::Vector2f(toJoueur.x / distJ, toJoueur.y / distJ);
                    float dot = directionNorm.x * toJNorm.x + directionNorm.y * toJNorm.y;
                    joueurDetecte = (distJ <= fovRange && dot >= cosHalfFov);
                }
            }
        }
    }

    // Renvoie le centre (en pixels) de l'obstacle idx (si idx invalide, (0,0))
    sf::Vector2f Modele::getObstacleCenter(size_t idx) const
    {
        if (idx >= obstacles.size() || obstacles[idx] == nullptr)
            return sf::Vector2f(0.f, 0.f);
        sf::FloatRect b = obstacles[idx]->getGlobalBounds();
        return sf::Vector2f(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
    }

    // Renvoie la direction normalisée vers la cible actuelle pour l'obstacle idx
    sf::Vector2f Modele::getObstacleForward(size_t idx) const
    {
        if (idx >= obstacles.size() || obstacles[idx] == nullptr)
            return sf::Vector2f(1.f, 0.f);
        sf::Vector2f pos = obstacles[idx]->getPosition();
        // Utilise le point de patrouille courant comme cible (comme dans mettreAJourObstacles)
        sf::Vector2f cible = pointsPatrouille[pointCibleIndex];
        sf::Vector2f dir = cible - pos;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len < 1e-5f) return sf::Vector2f(1.f, 0.f);
        return sf::Vector2f(dir.x / len, dir.y / len);
    }

}
