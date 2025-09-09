# Visual Placement System Usage Guide

This guide demonstrates how to use the new visual placement system for agents in the Tactical Monster game.

## Overview

The system provides visual feedback when placing agents by highlighting cells:
- **Green cells**: Valid placement zones for the selected agent type
- **Red cells**: Invalid placement zones for the selected agent type

## Key Features

### 1. Agent Placement Rules
- **WaterWalking**: Can be placed on Water and Normal (Ground), but NOT on Rock
- **Grounded**: Can be placed on Normal (Ground) only, NOT on Water or Rock  
- **Flying**: Can be placed on Normal (Ground) only, NOT on Water or Rock
- **Floating**: Can be placed on ALL terrain types (Water, Normal, Rock)

### 2. Visual Feedback Methods

#### Cell-level highlighting:
```cpp
// Highlight a single cell for placement validation
cell->highlightForPlacement(true);  // Green highlight for valid
cell->highlightForPlacement(false); // Red highlight for invalid
cell->clearHighlight();             // Remove highlighting
```

#### Agent-level zone display:
```cpp
// Show placement zones for a specific agent instance
agent->showPlacementZones(allCells);
agent->hidePlacementZones(allCells);

// Static methods for agent types (useful when selecting from deck)
Agent::showPlacementZonesForType(AgentType::WaterWalking, allCells);
Agent::hidePlacementZones(allCells);
```

#### Placement validation:
```cpp
// Check if an agent can be placed on a cell
bool canPlace = cell->canPlaceAgent(agent);
bool canPlace = agent->canBePlacedOn(cell->getType());

// Static validation without agent instance
bool canPlace = Agent::canPlaceAgentType(AgentType::Flying, Cell::Water);
```

## Integration Examples

### Example 1: Agent Selection from Deck
```cpp
void GamePage::onAgentCardSelected(AgentType selectedType) {
    // Clear any previous highlights
    placementHelper->hideAllPlacementZones(allCells);
    
    // Show valid placement zones for the selected agent type
    placementHelper->showPlacementZonesForAgentType(selectedType, allCells);
    
    // Update UI with placement rules
    QString rules = Agent::getPlacementRules(selectedType);
    statusLabel->setText("Select a green cell to place agent. " + rules);
}
```

### Example 2: Cell Click Handling
```cpp
void GamePage::onCellClicked(Cell* clickedCell) {
    if (selectedAgentType != nullptr) {
        QString errorMessage;
        if (placementHelper->validatePlacement(pendingAgent, clickedCell, errorMessage)) {
            // Valid placement - place the agent
            pendingAgent->setCell(clickedCell);
            placementHelper->hideAllPlacementZones(allCells);
            statusLabel->setText("Agent placed successfully!");
        } else {
            // Invalid placement - show error
            statusLabel->setText(errorMessage);
        }
    }
}
```

### Example 3: Real-time Validation
```cpp
void GamePage::onCellHovered(Cell* hoveredCell) {
    if (selectedAgent && hoveredCell) {
        QString message;
        bool isValid = placementHelper->validatePlacement(selectedAgent, hoveredCell, message);
        
        // Update cursor or status based on validity
        if (isValid) {
            setCursor(Qt::PointingHandCursor);
            statusLabel->setText("Click to place agent here");
        } else {
            setCursor(Qt::ForbiddenCursor);
            statusLabel->setText(message);
        }
    }
}
```

### Example 4: Using PlacementHelper Class
```cpp
// In your game class constructor
placementHelper = new PlacementHelper(this);
connect(placementHelper, &PlacementHelper::placementValidated,
        this, &GamePage::onPlacementValidated);

// When user selects an agent card
void GamePage::onAgentCardClicked(AgentType type) {
    currentSelectedType = type;
    placementHelper->showPlacementZonesForAgentType(type, gameBoard->getAllCells());
}

// When user clicks on a cell
void GamePage::onBoardCellClicked(Cell* cell) {
    if (currentSelectedType != nullptr) {
        // Create temporary agent for validation
        Agent* tempAgent = new Agent(currentPlayer, "Test", currentSelectedType, 
                                   100, 3, 25, 2);
        
        QString errorMsg;
        if (placementHelper->validatePlacement(tempAgent, cell, errorMsg)) {
            // Place the actual agent
            placeAgentOnBoard(currentSelectedType, cell);
        }
        
        delete tempAgent;
    }
}
```

## Color Scheme

The visual feedback uses the following color scheme:
- **Valid placement**: Light green background (144, 238, 144, 150) with bright green border
- **Invalid placement**: Light red background (255, 182, 193, 150) with bright red border
- **Normal state**: Original cell colors (Blue for Water, Dark Gray for Rock, etc.)

## Tips for Integration

1. **Always clear highlights** before showing new ones to avoid visual conflicts
2. **Use PlacementHelper class** for complex scenarios - it manages state automatically
3. **Validate placement** before actually placing agents to provide immediate feedback
4. **Show placement rules** in your UI to help users understand the restrictions
5. **Handle edge cases** like occupied cells and invalid agent/cell combinations

## Error Handling

The system includes comprehensive error handling:
- Validates null pointers for agents and cells
- Checks cell occupancy before placement
- Provides descriptive error messages for invalid placements
- Includes debug output for development

This visual placement system makes the game more intuitive and helps players understand the terrain restrictions for different agent types.