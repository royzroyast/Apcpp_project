// PlacementHelper.cpp - Implementation of PlacementHelper
#include "PlacementHelper.h"
#include <QDebug>

PlacementHelper::PlacementHelper(QObject* parent) : QObject(parent) {
}

void PlacementHelper::showPlacementZonesForAgentType(AgentType type, const QList<Cell*>& cells) {
    if (m_isShowingZones) {
        hideAllPlacementZones(cells);
    }
    
    m_currentAgentType = type;
    m_isShowingZones = true;
    
    Agent::showPlacementZonesForType(type, cells);
    
    qDebug() << "Showing placement zones for agent type:" << type;
    qDebug() << "Placement rules:" << Agent::getPlacementRules(type);
}

void PlacementHelper::showPlacementZonesForAgent(Agent* agent, const QList<Cell*>& cells) {
    if (!agent) return;
    
    if (m_isShowingZones) {
        hideAllPlacementZones(cells);
    }
    
    m_currentAgentType = agent->getType();
    m_isShowingZones = true;
    
    agent->showPlacementZones(cells);
    
    qDebug() << "Showing placement zones for agent:" << agent->getName();
}

void PlacementHelper::hideAllPlacementZones(const QList<Cell*>& cells) {
    if (!m_isShowingZones) return;
    
    Agent::hidePlacementZones(cells);
    m_isShowingZones = false;
    
    qDebug() << "Hiding all placement zones";
}

bool PlacementHelper::validatePlacement(Agent* agent, Cell* targetCell, QString& errorMessage) {
    if (!agent || !targetCell) {
        errorMessage = "Invalid agent or cell";
        emit placementValidated(false, errorMessage);
        return false;
    }
    
    // Check if cell is occupied
    if (targetCell->isOccupied()) {
        errorMessage = "Cell is already occupied by another agent";
        emit placementValidated(false, errorMessage);
        return false;
    }
    
    // Check terrain compatibility
    if (!agent->canBePlacedOn(targetCell->getType())) {
        QString cellTypeName;
        switch(targetCell->getType()) {
        case Cell::Water: cellTypeName = "Water"; break;
        case Cell::Rock: cellTypeName = "Rock"; break;
        case Cell::Normal: cellTypeName = "Ground"; break;
        case Cell::Goal: cellTypeName = "Goal"; break;
        default: cellTypeName = "Unknown"; break;
        }
        
        errorMessage = QString("Agent '%1' cannot be placed on %2 terrain. %3")
                      .arg(agent->getName())
                      .arg(cellTypeName)
                      .arg(Agent::getPlacementRules(agent->getType()));
        emit placementValidated(false, errorMessage);
        return false;
    }
    
    errorMessage = QString("Valid placement for agent '%1'").arg(agent->getName());
    emit placementValidated(true, errorMessage);
    return true;
}

QString PlacementHelper::getPlacementLegend() {
    return QString("Placement Legend:\n"
                  "🟢 Green cells: Valid placement zones\n"
                  "🔴 Red cells: Invalid placement zones\n"
                  "⚪ Normal cells: No highlight when not selecting");
}