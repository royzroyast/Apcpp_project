#include "tacticalmonster.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QMovie>
#include <qmessagebox.h>
#include "AgentCardWidget.h"
#include "AgentType.h"

TacticalMonster::TacticalMonster(QWidget *parent) : QMainWindow(parent), ui(new Ui::TacticalMonster)
{
    ui->setupUi(this);
    setupUI();
    setupAgentSelection();
}

TacticalMonster::~TacticalMonster()
{
    if (m_currentTurnLabel) {
        delete m_currentTurnLabel;
        m_currentTurnLabel = nullptr;
    }
    delete ui;
}

void TacticalMonster::setupAgentSelection()
{
    // Clear existing cards
    qDeleteAll(ui->player1CardsLayout->children());
    qDeleteAll(ui->player2CardsLayout->children());

    createAgentCards();
    updateSelectionStatus();

    //connect(ui->StartBattle_Btn, &QPushButton::clicked, this, &TacticalMonster::onStartBattleClicked);
}

void TacticalMonster::createAgentCards()
{
    struct AgentDef {
        QString name;
        AgentType type;
        int hp, mobility, damage, attackRange;
    };

    QList<AgentDef> agents = {
        {"Sir Lamorak", Grounded, 320, 3, 110, 1},
        {"Kabul", Grounded, 400, 2, 120, 1},
        {"Rajakal", Grounded, 320, 2, 130, 1},
        {"Salih", Grounded, 400, 2, 80, 1},
        {"Khan", Grounded, 320, 2, 90, 1},
        {"Boi", Grounded, 400, 2, 100, 1},
        {"Eloi", Grounded, 240, 2, 100, 2},
        {"Kanar", Grounded, 160, 2, 100, 2},
        {"Elsa", Grounded, 320, 2, 140, 2},
        {"Karissa", Grounded, 280, 2, 80, 2},
        {"Sir Philip", Grounded, 400, 2, 100, 1},
        {"Frost", Grounded, 260, 2, 80, 2},
        {"Tusk", Grounded, 400, 2, 100, 1},
        {"Rambu", Flying, 320, 3, 120, 1},
        {"Sabrina", Floating, 320, 3, 100, 1},
        {"Death", Floating, 240, 3, 120, 2},
        {"Reketon", WaterWalking, 320, 2, 80, 2},
        {"Angus", WaterWalking, 400, 2, 100, 1},
        {"Duraham", WaterWalking, 320, 2, 100, 2},
        {"Colonel Baba", WaterWalking, 400, 2, 100, 1},
        {"Medusa", WaterWalking, 320, 2, 90, 2},
        {"Bunka", WaterWalking, 320, 3, 100, 1},
        {"Sanka", WaterWalking, 320, 3, 100, 1},
        {"Billy", WaterWalking, 320, 3, 90, 1}
    };

    for (const auto& agent : agents)
    {
        // Player 1 card
        AgentCardWidget* card1 = new AgentCardWidget(agent.name, agent.type, agent.hp,
                                                     agent.mobility, agent.damage, agent.attackRange);
        connect(card1, &AgentCardWidget::clicked, [this, card1]() {
            onAgentCardClicked(card1, 0);
        });
        ui->player1CardsLayout->insertWidget(0,card1);

        // Player 2 card (similar)
        AgentCardWidget* card2 = new AgentCardWidget(agent.name, agent.type, agent.hp,
                                                        agent.mobility, agent.damage, agent.attackRange);
        connect(card2, &AgentCardWidget::clicked, [this, card2]() {
            qDebug() << "Player 2 card clicked - calling onAgentCardClicked";
            onAgentCardClicked(card2, 1);
        });
        ui->player2CardsLayout->insertWidget(0,card2);
    }
}

void TacticalMonster::onAgentCardClicked(AgentCardWidget* card, int playerIndex) {
    qDebug() << "Card clicked:" << card->getName() << "Player:" << playerIndex;
    qDebug() << "Current widget:" << (ui->stackedWidget->currentWidget() == ui->PreCombat_Page ? "PreCombat_Page" : "Other");
    qDebug() << "Current phase:" << m_currentPhase;
    
    // Only handle clicks in PreCombat page during placement phase
    if (ui->stackedWidget->currentWidget() == ui->PreCombat_Page && m_currentPhase == AgentPlacement) {
        QList<AgentCardWidget*>& placedCards = (playerIndex == 0) ? m_player1PlacedCards : m_player2PlacedCards;
        
        qDebug() << "Player" << playerIndex << "placed cards count:" << placedCards.size();
        qDebug() << "Card is already placed:" << placedCards.contains(card);
        
        // Check if this card is currently selected (deselection case)
        if (m_currentPlacementCard == card && m_currentPlacementPlayer == playerIndex) {
            qDebug() << "Deselecting card:" << card->getName();
            
            // Clear highlights and reset selection
            clearCellHighlights();
            m_currentPlacementCard = nullptr;
            m_currentPlacementPlayer = -1;
            return;
        }
        
        // Only allow placement if this card hasn't been placed yet and player has less than 3 placed
        if (!placedCards.contains(card) && placedCards.size() < 3) {
            qDebug() << "Starting placement for card:" << card->getName();
            
            // Clear previous highlights
            clearCellHighlights();
            
            // Start placement for this card
            m_currentPlacementCard = card;
            m_currentPlacementPlayer = playerIndex;
            
            // Highlight valid cells for this player
            highlightValidCells(playerIndex);
        } else if (placedCards.contains(card)) {
            qDebug() << "Card already placed!";
        } else {
            qDebug() << "Player already has 3 agents placed!";
        }
    }
}
void TacticalMonster::updateSelectionStatus() {
    // This function is no longer used - we go directly to placement
    updatePlacementStatus();
}

void TacticalMonster::on_OKButton_clicked() {
    qDebug() << "=== OK Button Clicked ===";
    qDebug() << "Before transition - Player1 selected:" << m_player1SelectedCards.size() 
             << "Player2 selected:" << m_player2SelectedCards.size();
             
    QString name1 = ui->Player1_LineEdit->text();
    QString name2 = ui->Player2_LineEdit->text();

    if (name1.isEmpty() || name2.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter player names!");
        return;
    }

    delete m_gamePage;
    m_player1 = new Player(name1, true, this);
    m_player2 = new Player(name2, false, this);

    m_gamePage = new GamePage(ui->MapSelector_CBox, ui->GameView_GView,
                              m_player1, m_player2, this);
    
    // Connect cell interaction signals
    connect(m_gamePage, &GamePage::cellClicked, this, &TacticalMonster::onCellClicked);
    
    m_gamePage->startGame();
    
    qDebug() << "Before setupCombatPage - Player1 selected:" << m_player1SelectedCards.size() 
             << "Player2 selected:" << m_player2SelectedCards.size();
    
    setupCombatPage();
    ui->stackedWidget->setCurrentWidget(ui->PreCombat_Page);
    
    qDebug() << "=== Transition Complete ===";
}

void TacticalMonster::setupUI() {
    // Setup animations and fonts
    QMovie *movie = new QMovie(":/new/prefix1/animation.gif");
    ui->BackGround_Label->setMovie(movie);
    movie->start();

    ui->stackedWidget->setCurrentWidget(ui->Welcome_Page);

    // Connect navigation buttons
    connect(ui->Play_Btn, &QPushButton::clicked, [this]() {ui->stackedWidget->setCurrentWidget(ui->MainMenu_Page);});
    connect(ui->Gallery_Btn, &QPushButton::clicked, [this]() {ui->stackedWidget->setCurrentWidget(ui->GalleryMain_Page);});
    connect(ui->StartGame_Btn, &QPushButton::clicked, [this]() {ui->stackedWidget->setCurrentWidget(ui->CreatServer_Page);});
    connect(ui->JoinGame_Btn, &QPushButton::clicked, this, &TacticalMonster::on_OKButton_clicked);
    connect(ui->StartBattle_Btn, &QPushButton::clicked, this, &TacticalMonster::onStartBattleClicked);
    connect(ui->Floating_Btn, &QPushButton::clicked, [this]() {ui->stackedWidget->setCurrentWidget(ui->GalleryFloating_Page);});
    connect(ui->Flying_Btn, &QPushButton::clicked, [this]() {ui->stackedWidget->setCurrentWidget(ui->GalleryFlying_Page);});
    connect(ui->WaterWalking_Btn, &QPushButton::clicked, [this]() {ui->stackedWidget->setCurrentWidget(ui->GalleryWaterWalking_Page);});
    connect(ui->Grounded_Btn, &QPushButton::clicked, [this]() {ui->stackedWidget->setCurrentWidget(ui->GalleryGrounded_Page);});
    // Connect back buttons using a shared slot
    connect(ui->Back_Btn_MainMenu, &QPushButton::clicked, this, &TacticalMonster::handleNavigation);
    connect(ui->Back_Btn_GalleryMain, &QPushButton::clicked, this, &TacticalMonster::handleNavigation);
    connect(ui->Back_Btn_FloatingGallery, &QPushButton::clicked, this, &TacticalMonster::handleNavigation);
    connect(ui->Back_Btn_FlyingGallery, &QPushButton::clicked, this, &TacticalMonster::handleNavigation);
    connect(ui->Back_Btn_GroundedGallery, &QPushButton::clicked, this, &TacticalMonster::handleNavigation);
    connect(ui->Back_Btn_WaterWalkingGallery, &QPushButton::clicked, this, &TacticalMonster::handleNavigation);
    connect(ui->Back_Btn_CreateServer, &QPushButton::clicked, this, &TacticalMonster::handleNavigation);
    connect(ui->Back_Btn_PreCombat, &QPushButton::clicked, this, &TacticalMonster::handleNavigation);
}

void TacticalMonster::handleNavigation()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (button == ui->Back_Btn_MainMenu)
        ui->stackedWidget->setCurrentWidget(ui->Welcome_Page);
    if (button == ui->Back_Btn_GalleryMain)
        ui->stackedWidget->setCurrentWidget(ui->MainMenu_Page);
    if (button == ui->Back_Btn_FloatingGallery)
        ui->stackedWidget->setCurrentWidget(ui->GalleryMain_Page);
    if (button == ui->Back_Btn_FlyingGallery)
        ui->stackedWidget->setCurrentWidget(ui->GalleryMain_Page);
    if (button == ui->Back_Btn_GroundedGallery)
        ui->stackedWidget->setCurrentWidget(ui->GalleryMain_Page);
    if (button == ui->Back_Btn_WaterWalkingGallery)
        ui->stackedWidget->setCurrentWidget(ui->GalleryMain_Page);
    if (button == ui->Back_Btn_CreateServer)
        ui->stackedWidget->setCurrentWidget(ui->MainMenu_Page);
    if (button == ui->Back_Btn_PreCombat) {
        resetUIState();
        ui->stackedWidget->setCurrentWidget(ui->CreatServer_Page);
    }
}

void TacticalMonster::setupCombatPage() {
    qDebug() << "Setting up combat page";
    
    // Start directly in placement mode
    m_currentPhase = AgentPlacement;
    m_player1PlacedCards.clear();
    m_player2PlacedCards.clear();
    m_currentPlacementCard = nullptr;
    m_currentPlacementPlayer = -1;
    
    // Create the current turn label if it doesn't exist
    if (!m_currentTurnLabel) {
        m_currentTurnLabel = new QLabel(ui->PreCombat_Page);
        m_currentTurnLabel->setGeometry(60, 670, 800, 40); // Repositioned to top center, larger
        m_currentTurnLabel->setAlignment(Qt::AlignCenter);
        m_currentTurnLabel->setStyleSheet("QLabel { font: bold 16px; color: blue; background-color: rgba(255, 255, 255, 200); border: 3px solid blue; border-radius: 8px; padding: 5px; }");
        m_currentTurnLabel->setText("Placement Phase");
        m_currentTurnLabel->setVisible(true);
    }
    
    // End Turn button removed - using automatic turn progression instead
    
    // Update UI for placement phase
    updatePlacementStatus();
    
    qDebug() << "Combat page setup complete, current phase:" << m_currentPhase;
}

void TacticalMonster::updatePlacementStatus() {
    qDebug() << "updatePlacementStatus - Player1 placed:" << m_player1PlacedCards.size() 
             << "Player2 placed:" << m_player2PlacedCards.size();
             
    ui->player1Status_Label->setText(
        QString("Placed: %1/3").arg(m_player1PlacedCards.size()));
    ui->player2Status_Label->setText(
        QString("Placed: %1/3").arg(m_player2PlacedCards.size()));
    
    // Update card appearance for placed agents
    for (AgentCardWidget* card : m_player1PlacedCards) {
        // Make placed cards appear dimmed/inactive
        card->setStyleSheet("QWidget { background-color: rgba(128, 128, 128, 100); }");
    }
    
    for (AgentCardWidget* card : m_player2PlacedCards) {
        // Make placed cards appear dimmed/inactive
        card->setStyleSheet("QWidget { background-color: rgba(128, 128, 128, 100); }");
    }
    
    // Enable start battle button when all 6 agents are placed
    if (m_player1PlacedCards.size() == 3 && m_player2PlacedCards.size() == 3) {
        ui->StartBattle_Btn->setText("Start Battle");
        ui->StartBattle_Btn->setEnabled(true);
        qDebug() << "All agents placed - battle can start!";
    } else {
        ui->StartBattle_Btn->setText(QString("Place All Agents (%1/6)")
                                    .arg(m_player1PlacedCards.size() + m_player2PlacedCards.size()));
        ui->StartBattle_Btn->setEnabled(false);
    }
}

void TacticalMonster::onCellClicked(Cell* cell) {
    if (m_currentPhase == AgentPlacement && m_currentPlacementCard && m_gamePage) {
        qDebug() << "Cell clicked during placement phase";
        
        // Try to place the agent
        QList<Cell*> validCells = m_gamePage->getValidPlacementCells(m_currentPlacementPlayer);
        
        if (validCells.contains(cell) && !cell->isOccupied()) {
            qDebug() << "Placing agent" << m_currentPlacementCard->getName() << "on valid cell";
            
            // Place the agent
            if (m_gamePage->placeAgent(m_currentPlacementCard, cell, m_currentPlacementPlayer)) {
                // Mark this card as placed
                if (m_currentPlacementPlayer == 0) {
                    m_player1PlacedCards.append(m_currentPlacementCard);
                } else {
                    m_player2PlacedCards.append(m_currentPlacementCard);
                }
                
                qDebug() << "Agent placed successfully! Total placed: Player1=" 
                         << m_player1PlacedCards.size() << "Player2=" << m_player2PlacedCards.size();
                
                // Clear placement state and highlights
                clearCellHighlights();
                m_currentPlacementCard = nullptr;
                m_currentPlacementPlayer = -1;
                
                // Update UI
                updatePlacementStatus();
            }
        } else {
            qDebug() << "Invalid cell clicked - not in valid cells or occupied";
        }
    } else if (m_currentPhase == Battle) {
        // Handle battle phase cell interactions
        // This will be handled by the GamePage's existing logic
        qDebug() << "Cell clicked during battle phase - letting GamePage handle it";
    } else {
        // During other phases, don't allow any interactions
        qDebug() << "Cell clicked but no valid action for current phase:" << m_currentPhase;
    }
}

void TacticalMonster::onAgentPlaced(int playerIndex) {
    // This slot can be used for additional logic when an agent is placed
    // For example, playing sound effects, updating animations, etc.
}



void TacticalMonster::onStartBattleClicked() {
    if (m_player1PlacedCards.size() == 3 && m_player2PlacedCards.size() == 3) {
        startBattlePhase();
    }
}

void TacticalMonster::startBattlePhase() {
    m_currentPhase = Battle;
    
    // Clear any remaining highlights
    clearCellHighlights();
    
    // Activate battle phase in GamePage
    if (m_gamePage) {
        m_gamePage->activateBattlePhase();
    }
    
    // Hide GUI elements that are no longer needed
    hideUnnecessaryUIElements();
    
    // Update UI for battle phase
    ui->StartBattle_Btn->setText("Battle in Progress");
    ui->StartBattle_Btn->setEnabled(false);
    
    ui->player1Status_Label->setText("In Battle");
    ui->player2Status_Label->setText("In Battle");
    
    // Update turn label to show current player's turn (Player 1 starts)
    if (m_currentTurnLabel) {
        QString currentPlayerName = m_gamePage->currentPlayer()->getName();
        m_currentTurnLabel->setText(QString("Current Turn: %1 (Select an agent to move/attack)").arg(currentPlayerName));
        m_currentTurnLabel->setStyleSheet("QLabel { font: bold 16px; color: green; background-color: rgba(255, 255, 255, 200); border: 3px solid green; border-radius: 8px; padding: 5px; }");
    }
    
    // No End Turn button - automatic turn progression after each move
    
    // Connect to game page battle signals if needed
    if (m_gamePage) {
        connect(m_gamePage, &GamePage::gameOver, this, [this](Player* winner) {
            QString winnerName = winner ? winner->getName() : "Draw";
            QMessageBox::information(this, "Game Over", 
                                   QString("Game Over! Winner: %1").arg(winnerName));
        });
        
        connect(m_gamePage, &GamePage::gameStateChanged, this, [this]() {
            // Update UI based on game state changes during battle
            if (m_gamePage->currentPlayer()) {
                QString currentPlayerName = m_gamePage->currentPlayer()->getName();
                ui->player1Status_Label->setText(
                    m_player1->getName() == currentPlayerName ? "Your Turn" : "Waiting");
                ui->player2Status_Label->setText(
                    m_player2->getName() == currentPlayerName ? "Your Turn" : "Waiting");
                
                // Update the turn label
                if (m_currentTurnLabel) {
                    m_currentTurnLabel->setText(QString("Current Turn: %1 (Select an agent to move/attack)").arg(currentPlayerName));
                }
            }
        });
    }
    
    // Start the actual battle
    // The GamePage will handle the battle logic
    qDebug() << "Battle phase started!";
}

void TacticalMonster::highlightValidCells(int playerIndex) {
    qDebug() << "highlightValidCells called for player:" << playerIndex;
    
    if (m_gamePage) {
        QList<Cell*> validCells = m_gamePage->getValidPlacementCells(playerIndex);
        qDebug() << "Valid cells found:" << validCells.size();
        
        if (validCells.isEmpty()) {
            qDebug() << "No valid cells found!";
            return;
        }
        
        // Highlight valid cells with player-specific colors
        QColor highlightColor;
        if (playerIndex == 0) {
            highlightColor = QColor(100, 255, 100, 150); // Light green for player 1
        } else {
            highlightColor = QColor(100, 100, 255, 150); // Light blue for player 2
        }
        
        qDebug() << "Highlighting cells with color:" << highlightColor;
        
        int highlightedCount = 0;
        for (Cell* cell : m_gamePage->getCells()) {
            if (validCells.contains(cell)) {
                cell->setBrush(QBrush(highlightColor));
                highlightedCount++;
            }
        }
        
        qDebug() << "Highlighted" << highlightedCount << "cells";
        
        // Store highlighted cells for later clearing (don't call startPlacement again)
        // m_gamePage->startPlacement(validCells);
    } else {
        qDebug() << "m_gamePage is null!";
    }
}

void TacticalMonster::clearCellHighlights() {
    if (m_gamePage) {
        for (Cell* cell : m_gamePage->getCells()) {
            cell->resetBrush();
        }
        m_gamePage->endPlacement();
    }
}

void TacticalMonster::hideUnnecessaryUIElements() {
    // Hide the player card lists (group boxes)
    ui->Player1_GroupBox->setVisible(false);
    ui->Player2_GroupBox->setVisible(false);
    
    // Hide the map selector and its label
    ui->MapSelector_CBox->setVisible(false);
    ui->Map_Label->setVisible(false);
    
    qDebug() << "Hidden unnecessary UI elements for battle phase";
}

void TacticalMonster::resetUIState() {
    qDebug() << "Resetting UI state to normal";
    
    // Show the player card lists (group boxes)
    ui->Player1_GroupBox->setVisible(true);
    ui->Player2_GroupBox->setVisible(true);
    
    // Show the map selector and its label
    ui->MapSelector_CBox->setVisible(true);
    ui->Map_Label->setVisible(true);
    
    // Reset the start battle button
    ui->StartBattle_Btn->setText("Place All Agents (0/6)");
    ui->StartBattle_Btn->setEnabled(false);
    
    // Clear placed cards lists
    m_player1PlacedCards.clear();
    m_player2PlacedCards.clear();
    
    // Reset current phase to placement
    m_currentPhase = AgentPlacement;
    m_currentPlacementCard = nullptr;
    m_currentPlacementPlayer = -1;
    
    // Clear any cell highlights
    clearCellHighlights();
    
    // Reset GamePage battle state if it exists
    if (m_gamePage) {
        m_gamePage->resetBattleState();
        delete m_gamePage;
        m_gamePage = nullptr;
    }
    
    // Reset status labels
    ui->player1Status_Label->setText("Placed: 0/3");
    ui->player2Status_Label->setText("Placed: 0/3");
    
    // Reset the turn label
    if (m_currentTurnLabel) {
        m_currentTurnLabel->setText("Placement Phase");
        m_currentTurnLabel->setStyleSheet("QLabel { font: bold 16px; color: blue; background-color: rgba(255, 255, 255, 200); border: 3px solid blue; border-radius: 8px; padding: 5px; }");
    }
    
    // No End Turn button to hide
    
    // Reset card appearances - remove the dimmed style from all cards
    for (int i = 0; i < ui->player1CardsLayout->count(); ++i) {
        QLayoutItem* item = ui->player1CardsLayout->itemAt(i);
        if (item && item->widget()) {
            AgentCardWidget* card = qobject_cast<AgentCardWidget*>(item->widget());
            if (card) {
                card->setStyleSheet(""); // Remove dimmed style
            }
        }
    }
    
    for (int i = 0; i < ui->player2CardsLayout->count(); ++i) {
        QLayoutItem* item = ui->player2CardsLayout->itemAt(i);
        if (item && item->widget()) {
            AgentCardWidget* card = qobject_cast<AgentCardWidget*>(item->widget());
            if (card) {
                card->setStyleSheet(""); // Remove dimmed style
            }
        }
    }
    
    qDebug() << "UI state reset complete";
}

