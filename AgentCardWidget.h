// AgentCardWidget.h
#ifndef AGENTCARDWIDGET_H
#define AGENTCARDWIDGET_H

#include <QWidget>
#include <QString>
#include <qlabel.h>
#include "AgentType.h"

class AgentCardWidget : public QWidget {
    Q_OBJECT
public:

    AgentCardWidget(const QString& name, AgentType type,
                    int hp, int mobility, int damage, int attackRange,
                    QWidget* parent = nullptr);

    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

    QString getName() const { return m_name; }
    AgentType getType() const { return m_type; }
    int getHP() const { return m_hp; }
    int getMobility() const { return m_mobility; }
    int getDamage() const { return m_damage; }
    int getAttackRange() const { return m_attackRange; }
private:
    QString getTypeString(AgentType type);
    QString getImagePathForType(AgentType type);
signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    //
    QLabel* iconLabel;
    QLabel* nameLabel;
    QLabel* hpLabel;
    QLabel* mobilityLabel;
    QLabel* damageLabel;
    QLabel* rangeLabel;

    QString m_name;
    AgentType m_type;
    int m_hp;
    int m_mobility;
    int m_damage;
    int m_attackRange;
    bool m_selected = false;
};

#endif // AGENTCARDWIDGET_H
