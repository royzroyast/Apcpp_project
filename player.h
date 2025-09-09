#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QString>
#include <QList>
#include "agent.h"

class AgentCard;

class Player : public QObject {
    Q_OBJECT
public:
    Player(const QString& name, bool isPlayer1, QObject* parent = nullptr);

    // Getters
    QString getName() const;
    bool isPlayer1() const;
    const QList<Agent*>& getAgents() const;
    bool hasAliveAgents() const;

    // Agent management
    void addAgent(Agent* agent);
    void removeAgent(Agent* agent);

    // Game actions
    void startTurn();
    void endTurn();

signals:
    void turnStarted();
    void turnEnded();

private:
    QString m_name;
    bool m_isPlayer1;
    QList<Agent*> m_agents;
};

#endif // PLAYER_H
