#ifndef TACTICALMONSTER_H
#define TACTICALMONSTER_H

#include "AgentCardWidget.h"
#include "ui_tacticalmonster.h"
#include "gamepage.h"
#include "player.h"
#include <QLabel>
#include <QPushButton>

class TacticalMonster : public QMainWindow {
    Q_OBJECT

public:
    TacticalMonster(QWidget *parent = nullptr);
    ~TacticalMonster();

private slots:
    // Navigation slots
    void handleNavigation();

    // Game flow slots
    void onAgentCardClicked(AgentCardWidget* card, int playerIndex);
    void onStartBattleClicked();
    void onCellClicked(Cell* cell);
    void onAgentPlaced(int playerIndex);

    void on_OKButton_clicked();
    
private:
    // Combat/Placement phase management
    enum GamePhase {
        AgentSelection,
        AgentPlacement,
        Battle
    };
    
    void setupUI();
    void setupAgentSelection();
    void createAgentCards();
    void updateSelectionStatus();
    void setupCombatPage();
    void createPlacementCards();
    void updatePlacementStatus();
    void checkPlacementComplete();
    void startBattlePhase();
    void highlightValidCells(int playerIndex);
    void clearCellHighlights();
    void hideUnnecessaryUIElements();
    void resetUIState();

    Ui::TacticalMonster *ui;
    GamePage* m_gamePage = nullptr;
    Player* m_player1 = nullptr;
    Player* m_player2 = nullptr;

    QList<AgentCardWidget*> m_player1SelectedCards;
    QList<AgentCardWidget*> m_player2SelectedCards;
    
    // Placement tracking
    QList<AgentCardWidget*> m_player1PlacedCards;
    QList<AgentCardWidget*> m_player2PlacedCards;
    
    GamePhase m_currentPhase = AgentSelection;
    AgentCardWidget* m_currentPlacementCard = nullptr;
    int m_currentPlacementPlayer = -1;
    
    // Battle turn tracking
    QLabel* m_currentTurnLabel = nullptr;
};

#endif // TACTICALMONSTER_H
