// PlacementHelper.h - Helper class for managing agent placement with visual feedback
#ifndef PLACEMENTHELPER_H
#define PLACEMENTHELPER_H

#include <QObject>
#include <QList>
#include "Agent.h"
#include "Cell.h"

class PlacementHelper : public QObject {
    Q_OBJECT

public:
    explicit PlacementHelper(QObject* parent = nullptr);
    
    // Show placement zones for an agent type (when selecting from deck)
    void showPlacementZonesForAgentType(AgentType type, const QList<Cell*>& cells);
    
    // Show placement zones for a specific agent (when agent is selected)
    void showPlacementZonesForAgent(Agent* agent, const QList<Cell*>& cells);
    
    // Hide all placement zones
    void hideAllPlacementZones(const QList<Cell*>& cells);
    
    // Check if placement is valid and provide feedback
    bool validatePlacement(Agent* agent, Cell* targetCell, QString& errorMessage);
    
    // Get color-coded legend for UI
    QString getPlacementLegend();

signals:
    void placementValidated(bool isValid, const QString& message);

private:
    bool m_isShowingZones = false;
    AgentType m_currentAgentType;
};

#endif // PLACEMENTHELPER_H
