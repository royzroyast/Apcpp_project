// AgentCardWidget.cpp - Revised
#include "AgentCardWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QFont>

AgentCardWidget::AgentCardWidget(const QString& name, AgentType type,
                                 int hp, int mobility, int damage, int attackRange,
                                 QWidget* parent)
    : QWidget(parent), m_name(name), m_type(type),
    m_hp(hp), m_mobility(mobility), m_damage(damage),
    m_attackRange(attackRange)
{
    setFixedSize(160, 125); // Match XML size

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // Icon (left side)
    iconLabel = new QLabel(this);
    iconLabel->setFixedSize(50, 50);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setPixmap(QPixmap(getImagePathForType(type)).scaled(50, 50, Qt::KeepAspectRatio));

    // Stats (right side)
    QVBoxLayout* statsLayout = new QVBoxLayout();
    statsLayout->setSpacing(2);

    nameLabel = new QLabel(name, this);
    QFont titleFont("Arial", 9, QFont::Bold);
    nameLabel->setFont(titleFont);

    // Type label
    QLabel* typeLabel = new QLabel(getTypeString(type), this);
    typeLabel->setStyleSheet("color: white; font-size: 8pt;");

    hpLabel = new QLabel(QString("HP: %1").arg(hp), this);
    mobilityLabel = new QLabel(QString("Mobility: %1").arg(mobility), this);
    damageLabel = new QLabel(QString("Damage: %1").arg(damage), this);
    rangeLabel = new QLabel(QString("Range: %1").arg(attackRange), this);

    // Apply consistent styling
    QString statStyle = "color: white; font-size: 8pt;";
    hpLabel->setStyleSheet(statStyle);
    mobilityLabel->setStyleSheet(statStyle);
    damageLabel->setStyleSheet(statStyle);
    rangeLabel->setStyleSheet(statStyle);

    // Add to layouts
    statsLayout->addWidget(nameLabel);
    statsLayout->addWidget(typeLabel);
    statsLayout->addWidget(hpLabel);
    statsLayout->addWidget(mobilityLabel);
    statsLayout->addWidget(damageLabel);
    statsLayout->addWidget(rangeLabel);

    mainLayout->addWidget(iconLabel);
    mainLayout->addLayout(statsLayout);

    setLayout(mainLayout);
}

QString AgentCardWidget::getTypeString(AgentType type) {
    switch(type) {
    case WaterWalking: return "Water Walking";
    case Grounded: return "Grounded";
    case Flying: return "Flying";
    case Floating: return "Floating";
    default: return "Unknown";
    }
}

QString AgentCardWidget::getImagePathForType(AgentType type) {
    // Map agent types to image resources
    switch(type) {
    case WaterWalking: return ":/new/prefix1/agent5.jpg";
    case Grounded: return ":/new/prefix1/agent1.jpg";
    case Flying: return ":/new/prefix1/agent3.jpg";
    case Floating: return ":/new/prefix1/agent6.jpg";
    default: return ":/new/prefix1/agent1.jpg";
    }
}
void AgentCardWidget::setSelected(bool selected) {
    m_selected = selected;
    setStyleSheet(m_selected ?
                      "background-color: rgb(85, 170, 255);":
                      "background-color: rgba(255, 255, 255, 0);");
}

void AgentCardWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        setSelected(!m_selected);
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}
