#include "Agent.h"
#include "player.h"
#include "Cell.h"
#include "gamepage.h"
#include <qgraphicsscene.h>
#include <QFont>
#include <QDebug>

Agent::Agent(Player* owner, const QString& name, AgentType type,
             int hp, int mobility, int damage, int attackRange,
             QGraphicsItem* parent)
    : QGraphicsRectItem(parent), m_owner(owner), m_name(name),
    m_type(type), m_maxHP(hp), m_currentHP(hp),
    m_mobility(mobility), m_remainingMoves(mobility),
    m_damage(damage), m_attackRange(attackRange)
{
    // Set up the background rectangle with colored border
    setRect(-15, -15, 30, 30);
    
    // Set border color based on player (thick border for differentiation)
    QColor borderColor = owner->isPlayer1() ? Qt::blue : Qt::red;
    setPen(QPen(borderColor, 3)); // 3-pixel thick border
    
    // Set background to semi-transparent white
    setBrush(QBrush(QColor(255, 255, 255, 200)));
    
    // Create text item for agent name (first 3 characters)
    m_nameText = new QGraphicsTextItem(name.left(3), this);
    m_nameText->setDefaultTextColor(Qt::black);
    m_nameText->setFont(QFont("Arial", 8, QFont::Bold));
    
    // Center the text within the rectangle
    QRectF textRect = m_nameText->boundingRect();
    m_nameText->setPos(-textRect.width()/2, -textRect.height()/2);
    
    // Create health bar background (above the agent)
    m_healthBarBackground = new QGraphicsRectItem(-15, -25, 30, 4, this);
    m_healthBarBackground->setBrush(QBrush(Qt::gray));
    m_healthBarBackground->setPen(QPen(Qt::black, 1));
    
    // Create health bar foreground (shows current health)
    m_healthBarForeground = new QGraphicsRectItem(-15, -25, 30, 4, this);
    m_healthBarForeground->setBrush(QBrush(Qt::green));
    m_healthBarForeground->setPen(QPen(Qt::transparent));
    
    updateHealthBar();
    
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

void Agent::setCell(Cell* cell)
{
    // Validate placement if we're setting a new cell
    if (cell && !canBePlacedOn(cell->getType())) {
        qDebug() << "Cannot place agent" << m_name << "on cell type" << cell->getType();
        return;
    }
    
    if (m_cell) {
        m_cell->setAgent(nullptr);
    }
    m_cell = cell;
    if (cell) {
        cell->setAgent(this);
        setPos(cell->getCenter());
    }
}

bool Agent::canMoveTo(Cell* target, GamePage* gamePage) const {
    if (!target || !isAlive() || !gamePage) {
        qDebug() << "canMoveTo: Invalid parameters";
        return false;
    }

    // Check if target is occupied
    if (target->isOccupied()) {
        qDebug() << "canMoveTo: Target cell is occupied";
        return false;
    }

    // Check if agent can be placed on this cell type
    if (!canBePlacedOn(target->getType())) {
        qDebug() << "canMoveTo: Agent" << m_name << "type" << m_type << "cannot be placed on cell type" << target->getType();
        return false;
    }

    // Use BFS to check if target is reachable within remaining moves
    QList<Cell*> reachableCells = gamePage->getReachableCells(m_cell, m_remainingMoves, const_cast<Agent*>(this));
    bool canReach = reachableCells.contains(target);
    
    qDebug() << "canMoveTo:" << m_name << "with" << m_remainingMoves << "moves can" << (canReach ? "reach" : "NOT reach") 
             << "target at (" << target->getRow() << "," << target->getCol() << ")";
    
    return canReach;
}

bool Agent::canBePlacedOn(Cell::CellType cellType) const {
    // Check terrain restrictions based on agent type for PLACEMENT
    switch(getType()) {
    case WaterWalking:
        // Can be placed on Water and Normal, but NOT on Rock
        return cellType == Cell::Water || cellType == Cell::Normal || cellType == Cell::Goal;
    case Grounded:
        // Can be placed on Normal cells only
        return cellType == Cell::Normal || cellType == Cell::Goal;
    case Flying:
        // Can be placed on Normal cells only (NOT on Water or Rock)
        return cellType == Cell::Normal || cellType == Cell::Goal;
    case Floating:
        // Can be placed on all cell types
        return true;
    }
    return false;
}

bool Agent::canMoveThrough(Cell::CellType cellType) const {
    // Check terrain restrictions based on agent type for MOVEMENT
    switch(getType()) {
    case WaterWalking:
        // Can move through Water and Normal, but NOT through Rock
        return cellType == Cell::Water || cellType == Cell::Normal || cellType == Cell::Goal;
    case Grounded:
        // Can only move through Normal cells
        return cellType == Cell::Normal || cellType == Cell::Goal;
    case Flying:
        // Can pass through any cells (including Rock and Water)
        return true;
    case Floating:
        // Can move through all cell types
        return true;
    }
    return false;
}

void Agent::moveTo(Cell* target, GamePage* gamePage) {
    if (canMoveTo(target, gamePage)) {
        // Calculate actual path distance using BFS
        int distance = gamePage->getBFSDistance(m_cell, target, this);
        if (distance > 0) {
            m_remainingMoves -= distance;
            setCell(target);
        }
    }
}

bool Agent::canAttack(Agent* target, GamePage* gamePage) const {
    if (!target || !isAlive() || !target->isAlive() || !gamePage) return false;

    // Can't attack own agents
    if (target->getOwner() == m_owner) return false;

    // Use BFS to check if target is within attack range
    QList<Cell*> cellsInRange = gamePage->getCellsInRange(m_cell, getAttackRange());
    return cellsInRange.contains(target->getCell());
}

void Agent::attack(Agent* target, GamePage* gamePage) {
    if (!canAttack(target, gamePage)) return;
    
    // 1. Agent attacks the opponent
    target->takeDamage(getDamage());
    
    // 2. Attacker takes half of the damage he deals to himself
    takeDamage(getDamage() / 2);
    
    // 3. Attacker will stand randomly in a valid cell around the opponent (target)
    if (isAlive()) {
        QList<Cell*> adjacentToTarget = gamePage->getAdjacentCells(target->getCell());
        QList<Cell*> available;
        
        for (Cell* cell : adjacentToTarget) {
            if (!cell->isOccupied() && canBePlacedOn(cell->getType())) {
                available.append(cell);
            }
        }

        if (!available.isEmpty()) {
            int randomIndex = rand() % available.size();
            Cell* newPosition = available[randomIndex];
            
            // Move attacker to the random cell around the target
            m_cell->setAgent(nullptr); // Clear current cell
            setCell(newPosition);      // Move to new position
        }
    }
}

void Agent::takeDamage(int amount) {
    m_currentHP -= amount;
    if (m_currentHP < 0) m_currentHP = 0;
    
    // Update health bar
    updateHealthBar();

    // If agent dies, clean up properly
    if (!isAlive()) {
        // Clear the cell reference
        if (m_cell) {
            m_cell->setAgent(nullptr);
            m_cell = nullptr;
        }
        
        // Remove from scene
        if (scene()) {
            scene()->removeItem(this);
        }
        
        // Schedule for deletion
        deleteLater();
    }
}

void Agent::updateHealthBar() {
    if (!m_healthBarForeground) return;
    
    // Calculate health percentage
    float healthPercentage = (float)m_currentHP / (float)m_maxHP;
    
    // Update the width of the foreground bar
    qreal newWidth = 30 * healthPercentage;
    m_healthBarForeground->setRect(-15, -25, newWidth, 4);
    
    // Change color based on health percentage
    QColor healthColor;
    if (healthPercentage > 0.6f) {
        healthColor = Qt::green;
    } else if (healthPercentage > 0.3f) {
        healthColor = Qt::yellow;
    } else {
        healthColor = Qt::red;
    }
    
    m_healthBarForeground->setBrush(QBrush(healthColor));
}

void Agent::showPlacementZones(const QList<Cell*>& allCells) {
    for (Cell* cell : allCells) {
        if (cell) {
            bool canPlace = cell->canPlaceAgent(this);
            cell->highlightForPlacement(canPlace);
        }
    }
}

void Agent::hidePlacementZones(const QList<Cell*>& allCells) {
    for (Cell* cell : allCells) {
        if (cell) {
            cell->clearHighlight();
        }
    }
}

bool Agent::canPlaceAgentType(AgentType type, Cell::CellType cellType) {
    // Static method to check placement rules without creating an agent instance
    switch(type) {
    case WaterWalking:
        // Can be placed on Water and Normal, but NOT on Rock
        return cellType == Cell::Water || cellType == Cell::Normal || cellType == Cell::Goal;
    case Grounded:
        // Can only be placed on Normal cells
        return cellType == Cell::Normal || cellType == Cell::Goal;
    case Flying:
        // Can be placed on Normal cells only (NOT on Water or Rock)
        return cellType == Cell::Normal || cellType == Cell::Goal;
    case Floating:
        // Can be placed on all cell types
        return true;
    }
    return false;
}

void Agent::showPlacementZonesForType(AgentType type, const QList<Cell*>& allCells) {
    for (Cell* cell : allCells) {
        if (cell && !cell->isOccupied()) {
            bool canPlace = canPlaceAgentType(type, cell->getType());
            cell->highlightForPlacement(canPlace);
        }
    }
}

QString Agent::getPlacementRules(AgentType type) {
    switch(type) {
    case WaterWalking:
        return "PLACEMENT: Water, Normal. MOVEMENT: Through Water, Normal (NOT Rock)";
    case Grounded:
        return "PLACEMENT: Normal only. MOVEMENT: Through Normal only";
    case Flying:
        return "PLACEMENT: Normal only. MOVEMENT: Through ANY terrain (can pass over Rock/Water)";
    case Floating:
        return "PLACEMENT: Any terrain. MOVEMENT: Through any terrain";
    }
    return "Unknown agent type";
}
