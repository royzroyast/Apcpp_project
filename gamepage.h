#ifndef GAMEPAGE_H
#define GAMEPAGE_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QComboBox>
#include "player.h"
#include "Cell.h"

class AgentCardWidget;

class GamePage : public QObject {
    Q_OBJECT

public:
    explicit GamePage(QComboBox* mapSelector, QGraphicsView* gameView,
                      Player* player1, Player* player2, QObject* parent = nullptr);
    ~GamePage();

    // Game state
    void startGame();
    void endTurn();
    bool isGameOver() const;
    Player* getWinner() const;

    // Placement
    void startPlacement(const QList<Cell*>& placableCells);
    void endPlacement();
    
    // Agent placement
    void startAgentPlacement(AgentCardWidget* card, int playerIndex);
    QList<Cell*> getValidPlacementCells(int playerIndex) const;
    bool placeAgent(AgentCardWidget* card, Cell* cell, int playerIndex);
    
    // Battle phase
    void activateBattlePhase();
    void resetBattleState();

    // Accessors
    Player* currentPlayer() const;
    const QVector<Cell*>& getCells() const;
    Cell* getCellAt(int row, int col) const;
    
    // BFS algorithms for hex grid
    QList<Cell*> getAdjacentCells(Cell* cell) const;
    QList<Cell*> getReachableCells(Cell* startCell, int maxDistance, Agent* agent = nullptr) const;
    QList<Cell*> getCellsInRange(Cell* centerCell, int range) const;
    int getBFSDistance(Cell* from, Cell* to, Agent* agent = nullptr) const;
    
    // Cell highlighting for movement and attacks
    void highlightMovementCells(Agent* agent);
    void highlightPassableCells(Agent* agent);
    void highlightAttackableEnemies(Agent* agent);
    void clearAllHighlights();

signals:
    void cellClicked(Cell* cell);
    void gameOver(Player* winner);
    void gameStateChanged();

public slots:
    void onCellInteraction(Cell* cell);

private slots:
    void loadSelectedMap(const QString &mapName);

private:
    void loadMap(const QString& path);
    Cell* createCell(int row, int col, const Cell::CellType type);
    void setupInitialAgents();

    bool m_placementMode;
    bool m_battlePhaseActive = false;  // Track if battle phase is active

    QGraphicsScene* m_scene;
    QGraphicsView* m_gameView;
    QComboBox* m_mapSelector;
    QVector<Cell*> m_cells;
    QList<Cell*> m_placableCells;
    QList<Cell*> m_player1PlacementZones;
    QList<Cell*> m_player2PlacementZones;
    Player* m_player1;
    Player* m_player2;
    Player* m_currentPlayer;

    Agent* m_selectedAgent = nullptr;
    Cell* m_selectedCell = nullptr;
    
    // Placement state
    AgentCardWidget* m_currentPlacementCard = nullptr;
    int m_currentPlacementPlayer = -1;
};

#endif // GAMEPAGE_H
