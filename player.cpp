#include "player.h"
#include "agent.h"

Player::Player(const QString& name, bool isPlayer1, QObject* parent)
    : QObject(parent), m_name(name), m_isPlayer1(isPlayer1)
{
}

// Getters implementation
QString Player::getName() const { return m_name; }
bool Player::isPlayer1() const { return m_isPlayer1; }
const QList<Agent*>& Player::getAgents() const { return m_agents; }

bool Player::hasAliveAgents() const {
    for (Agent* agent : m_agents) {
        if (agent->isAlive()) return true;
    }
    return false;
}

void Player::addAgent(Agent* agent) {
    if (agent && !m_agents.contains(agent)) {
        m_agents.append(agent);
    }
}

void Player::removeAgent(Agent* agent) {
    m_agents.removeOne(agent);
}

void Player::startTurn() {
    for (Agent* agent : m_agents) {
        if (agent->isAlive()) {
            agent->resetMoves();
        }
    }
    emit turnStarted();
}

void Player::endTurn() {
    emit turnEnded();
}
