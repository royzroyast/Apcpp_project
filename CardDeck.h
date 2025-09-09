#ifndef CARDDECK_H
#define CARDDECK_H

#include "agent.h"
#include <QList>

class CardDeck {
public:
    void addAgent(Agent* agent);
    void removeAgent(Agent* agent);
    const QList<Agent*>& getAgents() const;
    bool hasAgent(const QString& name) const;

private:
    QList<Agent*> m_agents;
};

#endif // CARDDECK_H
