#include "gamepage.h"
#include <QFile>
#include <QTextStream>
#include <QQueue>
#include <QSet>
#include "AgentCardWidget.h"
#include "agent.h"

GamePage::GamePage(QComboBox* mapSelector, QGraphicsView* gameView,
                   Player* player1, Player* player2, QObject* parent)
    : QObject(parent), m_mapSelector(mapSelector), m_gameView(gameView),
    m_player1(player1), m_player2(player2), m_currentPlayer(player1), m_placementMode(false)
{
    m_scene = new QGraphicsScene(this);
    m_gameView->setScene(m_scene);
    m_mapSelector->clear();
    m_mapSelector->addItems({
        "grid1.txt", "grid2.txt", "grid3.txt", "grid4.txt",
        "grid5.txt", "grid6.txt", "grid7.txt", "grid8.txt"
    });

    connect(m_mapSelector, &QComboBox::currentTextChanged, this, &GamePage::loadSelectedMap);
}

GamePage::~GamePage() {
    qDeleteAll(m_cells);
    m_cells.clear();
}

void GamePage::startGame() {
    loadSelectedMap(m_mapSelector->currentText());
    m_currentPlayer->startTurn();
}

void GamePage::endTurn() {
    // Reset selected agent highlighting
    if (m_selectedAgent) {
        m_selectedAgent->setPen(QPen(m_selectedAgent->getOwner()->isPlayer1() ? Qt::blue : Qt::red, 3));
        m_selectedAgent = nullptr;
    }
    
    m_currentPlayer->endTurn();
    m_currentPlayer = (m_currentPlayer == m_player1) ? m_player2 : m_player1;
    m_currentPlayer->startTurn();
    
    // Reset all current player's agents' moves for the new turn
    for (Agent* agent : m_currentPlayer->getAgents()) {
        if (agent && agent->isAlive()) {
            agent->resetMoves();
        }
    }
    
    emit gameStateChanged();
}

bool GamePage::isGameOver() const {
    return !m_player1->hasAliveAgents() || !m_player2->hasAliveAgents();
}

Player* GamePage::getWinner() const {
    if (!m_player1->hasAliveAgents()) return m_player2;
    if (!m_player2->hasAliveAgents()) return m_player1;
    return nullptr;
}

Player* GamePage::currentPlayer() const {
    return m_currentPlayer;
}

const QVector<Cell*>& GamePage::getCells() const {
    return m_cells;
}

Cell* GamePage::getCellAt(int row, int col) const {
    for (Cell* cell : m_cells) {
        if (cell->getRow() == row && cell->getCol() == col) {
            return cell;
        }
    }
    return nullptr;
}

void GamePage::loadSelectedMap(const QString &mapName) {
    m_scene->clear();
    m_cells.clear();
    loadMap(":/new/prefix1/" + mapName);
}

void GamePage::loadMap(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file:" << path;
        return;
    }

    m_player1PlacementZones.clear();
    m_player2PlacementZones.clear();

    QTextStream in(&file);
    int row = 1;
    int haxnum = 0;
    while (haxnum < 41) {
        QString line = in.readLine();
        for (int col = 0; col + 1 < line.length(); col += 3) {
            QString cell = line.mid(col, 2);
            if (!cell.isEmpty() && line[col] == '/') {
                Cell::CellType type = Cell::Normal;
                bool isPlacementZone = false;
                int placementPlayer = -1;
                
                if (cell == "/~") type = Cell::Water;
                else if (cell == "/#") type = Cell::Rock;
                else if (cell == "/*") type = Cell::Goal;
                else if (cell == "/1") {
                    type = Cell::Normal;
                    isPlacementZone = true;
                    placementPlayer = 0;
                }
                else if (cell == "/2") {
                    type = Cell::Normal;
                    isPlacementZone = true;
                    placementPlayer = 1;
                }
                
                Cell* newCell = createCell(row, col / 3, type);
                
                if (isPlacementZone) {
                    if (placementPlayer == 0) {
                        m_player1PlacementZones.append(newCell);
                    } else {
                        m_player2PlacementZones.append(newCell);
                    }
                }
                
                haxnum++;
            }
        }
        row++;
    }
    file.close();
}

Cell* GamePage::createCell(int row, int col, Cell::CellType type) {
    Cell* cell = new Cell(row, col, type);
    m_scene->addItem(cell);
    m_cells.append(cell);
    
    connect(cell, &Cell::clicked, this, &GamePage::onCellInteraction);
    connect(cell, &Cell::clicked, this, &GamePage::cellClicked);
    
    return cell;
}

void GamePage::onCellInteraction(Cell* cell) {
    // Null check for clicks outside the game board
    if (!cell) {
        // Clicking outside board should clear selection in battle phase
        if (m_battlePhaseActive && m_selectedAgent) {
            clearAllHighlights();
            m_selectedAgent = nullptr;
            emit gameStateChanged();
        }
        return;
    }
    
    if (m_placementMode) {
        // Placement mode
        if (m_placableCells.contains(cell) && m_currentPlacementCard) {
            if (placeAgent(m_currentPlacementCard, cell, m_currentPlacementPlayer)) {
                endPlacement();
            }
        }
    } else if (m_battlePhaseActive) {
        // Battle phase
        if (m_selectedAgent) {
            bool actionPerformed = false;
            
            if (m_selectedAgent->canMoveTo(cell, this)) {
                m_selectedAgent->moveTo(cell, this);
                actionPerformed = true;
            } else if (Agent* target = cell->getAgent()) {
                if (m_selectedAgent->canAttack(target, this)) {
                    m_selectedAgent->attack(target, this);
                    actionPerformed = true;

                    // Clear selected agent if it died during attack
                    if (!m_selectedAgent->isAlive()) {
                        m_selectedAgent = nullptr;
                    }

                    // Check for game over
                    if (isGameOver()) {
                        emit gameOver(getWinner());
                        return;
                    }
                }
            }
            
            // Clear all highlights and reset selection
            clearAllHighlights();
            m_selectedAgent = nullptr;
            
            // If an action was performed, automatically end the turn (1 agent per turn)
            if (actionPerformed) {
                endTurn();
            }
            
        } else if (cell->getAgent() && cell->getAgent()->getOwner() == m_currentPlayer) {
            // Clear any previous highlights
            clearAllHighlights();
            
            // Select agent only if it belongs to current player and battle phase is active
            m_selectedAgent = cell->getAgent();
            
            // Highlight selected agent with yellow border
            m_selectedAgent->setPen(QPen(Qt::yellow, 5));
            
            // Highlight cells agent can pass through (for Flying agents mainly)
            highlightPassableCells(m_selectedAgent);
            
            // Highlight available movement cells (destinations)
            highlightMovementCells(m_selectedAgent);
            
            // Highlight attackable enemies
            highlightAttackableEnemies(m_selectedAgent);
        } else {
            // Clicked on empty cell or invalid target - clear selection
            clearAllHighlights();
            m_selectedAgent = nullptr;
        }
    }
    // Don't allow agent selection during placement phase
    emit gameStateChanged();
}
void GamePage::startPlacement(const QList<Cell*>& placableCells) {
    m_placementMode = true;
    m_placableCells = placableCells;
    for (Cell* cell : m_cells) {
        if (placableCells.contains(cell)) {
            cell->setBrush(QBrush(QColor(100, 255, 100, 150))); // Light green
        }
    }
}

void GamePage::endPlacement() {
    m_placementMode = false;
    m_placableCells.clear();
    m_currentPlacementCard = nullptr;
    m_currentPlacementPlayer = -1;
    
    for (Cell* cell : m_cells) {
        cell->resetBrush();
    }
}

void GamePage::startAgentPlacement(AgentCardWidget* card, int playerIndex) {
    if (m_placementMode) {
        endPlacement(); // End current placement
    }
    
    m_currentPlacementCard = card;
    m_currentPlacementPlayer = playerIndex;
    m_placementMode = true;
    
    // Don't highlight here - let TacticalMonster handle it
    QList<Cell*> validCells = getValidPlacementCells(playerIndex);
    m_placableCells = validCells;
}

QList<Cell*> GamePage::getValidPlacementCells(int playerIndex) const {
    QList<Cell*> validCells;
    
    // Get placement zones based on player index
    const QList<Cell*>& placementZones = (playerIndex == 0) ? m_player1PlacementZones : m_player2PlacementZones;
    
    for (Cell* cell : placementZones) {
        // Only include unoccupied cells
        if (!cell->isOccupied()) {
            validCells.append(cell);
        }
    }
    
    return validCells;
}

bool GamePage::placeAgent(AgentCardWidget* card, Cell* cell, int playerIndex) {
    if (!card || !cell || cell->isOccupied()) {
        return false;
    }
    
    // Get the player
    Player* player = (playerIndex == 0) ? m_player1 : m_player2;
    
    // Create the agent
    Agent* agent = new Agent(player, card->getName(), card->getType(),
                             card->getHP(), card->getMobility(), 
                             card->getDamage(), card->getAttackRange());
    
    // Place the agent on the cell
    cell->setAgent(agent);
    agent->setCell(cell);
    
    // Add the agent to the scene and player
    m_scene->addItem(agent);
    player->addAgent(agent);
    
    return true;
}

void GamePage::activateBattlePhase() {
    m_battlePhaseActive = true;
    m_placementMode = false;
    
    // Reset all agents' moves for the battle phase
    for (Agent* agent : m_player1->getAgents()) {
        if (agent) {
            agent->resetMoves();
        }
    }
    for (Agent* agent : m_player2->getAgents()) {
        if (agent) {
            agent->resetMoves();
        }
    }
}

void GamePage::resetBattleState() {
    m_battlePhaseActive = false;
    m_placementMode = false;
    m_selectedAgent = nullptr;
    m_selectedCell = nullptr;
    m_currentPlacementCard = nullptr;
    m_currentPlacementPlayer = -1;
}

QList<Cell*> GamePage::getAdjacentCells(Cell* cell) const {
    QList<Cell*> adjacent;
    if (!cell) return adjacent;
    
    int row = cell->getRow();
    int col = cell->getCol();
    
    // Define hex grid adjacency offsets for neighbor finding
    QList<QPair<int, int>> offsets = {
        {-1, 0}, {-1, 1}, {0, 1}, {1, 0}, {0, -1}, {-1, -1},
        {1, 1}, {1, -1}, {-1, -2}, {-1, 2}, {0, -2}, {0, 2}
    };
    
    for (const auto& offset : offsets) {
        int newRow = row + offset.first;
        int newCol = col + offset.second;
        
        if (newRow >= 0 && newCol >= 0) {
            Cell* neighborCell = getCellAt(newRow, newCol);
            if (neighborCell) {
                int dr = abs(newRow - row);
                int dc = abs(newCol - col);
                
                // Validate proximity for hex grid adjacency
                if ((dr <= 1 && dc <= 1) || (dr <= 2 && dc == 0) || (dr == 0 && dc <= 2)) {
                    if (!adjacent.contains(neighborCell)) {
                        adjacent.append(neighborCell);
                    }
                }
            }
        }
    }
    return adjacent;
}

QList<Cell*> GamePage::getReachableCells(Cell* startCell, int maxDistance, Agent* agent) const {
    QList<Cell*> reachable;
    if (!startCell || maxDistance <= 0) return reachable;
    
    // BFS to find all reachable cells within maxDistance
    QQueue<QPair<Cell*, int>> queue; // Cell and current distance
    QSet<Cell*> visited;
    
    queue.enqueue({startCell, 0});
    visited.insert(startCell);
    
    while (!queue.isEmpty()) {
        QPair<Cell*, int> current = queue.dequeue();
        Cell* currentCell = current.first;
        int currentDistance = current.second;
        
        // Check if this cell can be a final destination (not just passed through)
        if (currentDistance > 0 && !currentCell->isOccupied()) { 
            // Agent must be able to be PLACED on the destination cell
            if (!agent || agent->canBePlacedOn(currentCell->getType())) {
                reachable.append(currentCell);
            }
        }
        
        if (currentDistance < maxDistance) {
            QList<Cell*> neighbors = getAdjacentCells(currentCell);
            
            for (Cell* neighbor : neighbors) {
                if (!visited.contains(neighbor)) {
                    // For pathfinding, check if agent can move THROUGH this cell
                    bool canPassThrough = false;
                    if (agent) {
                        if (neighbor->isOccupied()) {
                            canPassThrough = false;
                        } else {
                            canPassThrough = agent->canMoveThrough(neighbor->getType());
                        }
                    } else {
                        canPassThrough = !neighbor->isOccupied();
                    }
                    
                    if (canPassThrough) {
                        visited.insert(neighbor);
                        queue.enqueue({neighbor, currentDistance + 1});
                    }
                }
            }
        }
    }
    
    return reachable;
}

QList<Cell*> GamePage::getCellsInRange(Cell* centerCell, int range) const {
    QList<Cell*> inRange;
    if (!centerCell || range <= 0) return inRange;
    
    // BFS to find all cells within range (for attack range)
    QQueue<QPair<Cell*, int>> queue;
    QSet<Cell*> visited;
    
    queue.enqueue({centerCell, 0});
    visited.insert(centerCell);
    
    while (!queue.isEmpty()) {
        QPair<Cell*, int> current = queue.dequeue();
        Cell* currentCell = current.first;
        int currentDistance = current.second;
        
        if (currentDistance > 0) { // Don't include the center cell
            inRange.append(currentCell);
        }
        
        if (currentDistance < range) {
            QList<Cell*> neighbors = getAdjacentCells(currentCell);
            for (Cell* neighbor : neighbors) {
                if (!visited.contains(neighbor)) {
                    visited.insert(neighbor);
                    queue.enqueue({neighbor, currentDistance + 1});
                }
            }
        }
    }
    
    return inRange;
}

int GamePage::getBFSDistance(Cell* from, Cell* to, Agent* agent) const {
    if (!from || !to || from == to) return 0;
    
    // BFS to find shortest path distance
    QQueue<QPair<Cell*, int>> queue;
    QSet<Cell*> visited;
    
    queue.enqueue({from, 0});
    visited.insert(from);
    
    while (!queue.isEmpty()) {
        QPair<Cell*, int> current = queue.dequeue();
        Cell* currentCell = current.first;
        int currentDistance = current.second;
        
        if (currentCell == to) {
            return currentDistance;
        }
        
        QList<Cell*> neighbors = getAdjacentCells(currentCell);
        for (Cell* neighbor : neighbors) {
            if (!visited.contains(neighbor)) {
                // Check if agent can move through this cell (if agent is provided)
                if (!agent || (!neighbor->isOccupied() && agent->canMoveThrough(neighbor->getType())) || neighbor == to) {
                    visited.insert(neighbor);
                    queue.enqueue({neighbor, currentDistance + 1});
                }
            }
        }
    }
    
    return -1; // Path not found
}

void GamePage::highlightMovementCells(Agent* agent) {
    if (!agent) return;
    
    // Get reachable cells for agent
    QList<Cell*> reachableCells = getReachableCells(agent->getCell(), agent->getRemainingMoves(), agent);
    
    // Highlight available destinations
    for (Cell* cell : reachableCells) {
        if (!cell->isOccupied()) {
            cell->setBrush(QBrush(QColor(50, 100, 255, 200)));
        }
    }
}

void GamePage::highlightPassableCells(Agent* agent) {
    if (!agent) return;
    
    // Show cells agent can pass through but not necessarily stop on
    QQueue<QPair<Cell*, int>> queue;
    QSet<Cell*> visited;
    
    queue.enqueue({agent->getCell(), 0});
    visited.insert(agent->getCell());
    
    while (!queue.isEmpty()) {
        QPair<Cell*, int> current = queue.dequeue();
        Cell* currentCell = current.first;
        int currentDistance = current.second;
        
        if (currentDistance > 0 && !currentCell->isOccupied() && 
            agent->canMoveThrough(currentCell->getType()) && 
            !agent->canBePlacedOn(currentCell->getType())) {
            currentCell->setBrush(QBrush(QColor(200, 200, 200, 100)));
        }
        
        if (currentDistance < agent->getRemainingMoves()) {
            QList<Cell*> neighbors = getAdjacentCells(currentCell);
            for (Cell* neighbor : neighbors) {
                if (!visited.contains(neighbor) && !neighbor->isOccupied() && 
                    agent->canMoveThrough(neighbor->getType())) {
                    visited.insert(neighbor);
                    queue.enqueue({neighbor, currentDistance + 1});
                }
            }
        }
    }
}

void GamePage::highlightAttackableEnemies(Agent* agent) {
    if (!agent) return;
    
    // Get all cells within attack range
    QList<Cell*> attackRange = getCellsInRange(agent->getCell(), agent->getAttackRange());
    
    // Highlight enemy agents in attack range
    for (Cell* cell : attackRange) {
        if (Agent* enemy = cell->getAgent()) {
            if (enemy->getOwner() != agent->getOwner()) {
                enemy->setPen(QPen(Qt::red, 5));
            }
        }
    }
}

void GamePage::clearAllHighlights() {
    // Clear cell highlights
    for (Cell* cell : m_cells) {
        cell->resetBrush();
    }
    
    // Reset agent pen colors to normal
    for (Agent* agent : m_player1->getAgents()) {
        if (agent && agent->isAlive()) {
            agent->setPen(QPen(Qt::blue, 3));
        }
    }
    for (Agent* agent : m_player2->getAgents()) {
        if (agent && agent->isAlive()) {
            agent->setPen(QPen(Qt::red, 3));
        }
    }
}
