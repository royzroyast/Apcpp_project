// Agent.h - Revised
#ifndef AGENT_H
#define AGENT_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <qpen.h>
#include "Cell.h"
#include "AgentType.h"
#include "player.h"

class Player;
class Cell;

class Agent : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    Agent(Player* owner, const QString& name, AgentType type,
          int hp, int mobility, int damage, int attackRange,
          QGraphicsItem* parent = nullptr);
    // Getters
    QString getName() const { return m_name; }
    AgentType getType() const { return m_type; }
    int getCurrentHP() const { return m_currentHP; }
    int getMaxHP() const { return m_maxHP; }
    int getMobility() const { return m_mobility; }
    int getDamage() const { return m_damage; }
    int getAttackRange() const { return m_attackRange; }
    Player* getOwner() const { return m_owner; }
    Cell* getCell() const { return m_cell; }
    bool isAlive() const { return m_currentHP > 0; }
    int getRemainingMoves() const { return m_remainingMoves; }

    // Game actions
    void resetMoves() { m_remainingMoves = m_mobility; }
    void setCell(Cell* cell);
    bool canMoveTo(Cell* target, class GamePage* gamePage) const;
    bool canBePlacedOn(Cell::CellType cellType) const;
    bool canMoveThrough(Cell::CellType cellType) const;
    void moveTo(Cell* target, class GamePage* gamePage);
    bool canAttack(Agent* target, class GamePage* gamePage) const;
    void attack(Agent* target, class GamePage* gamePage);
    void takeDamage(int amount);
    
    // Visual feedback for placement
    void showPlacementZones(const QList<Cell*>& allCells);
    //void hidePlacementZones(const QList<Cell*>& allCells);
    
    // Static utility methods
    static QString getPlacementRules(AgentType type);
    static bool canPlaceAgentType(AgentType type, Cell::CellType cellType);
    static void showPlacementZonesForType(AgentType type, const QList<Cell*>& allCells);
    static void hidePlacementZones(const QList<Cell*>& allCells);

private:
    Player* m_owner;
    QString m_name;
    AgentType m_type;
    int m_maxHP;
    int m_currentHP;
    int m_mobility;
    int m_remainingMoves;
    int m_damage;
    int m_attackRange;
    Cell* m_cell = nullptr;
    QGraphicsTextItem* m_nameText;
    
    // Health bar components
    QGraphicsRectItem* m_healthBarBackground = nullptr;
    QGraphicsRectItem* m_healthBarForeground = nullptr;
    
    void updateHealthBar();
};

#endif // AGENT_H
