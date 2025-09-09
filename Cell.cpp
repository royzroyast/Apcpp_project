#include "Cell.h"
#include <QPen>
#include <QBrush>
#include <QtMath>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include "agent.h"

Cell::Cell(int row, int col, CellType type, QGraphicsItem* parent)
    : QGraphicsPolygonItem(parent), m_row(row), m_col(col), m_type(type)
{
    createHexagon();
}

void Cell::createHexagon() {
    const double w = m_size * 2;
    const double h = qSqrt(3) * m_size;
    double x = m_col * (0.65 * w);
    double y = m_row * h + (m_col % 2);
    m_center = QPointF(x, y);

    QPolygonF hex;
    for (int i = 0; i < 6; ++i) {
        double angle_deg = 60 * i + 30;
        double angle_rad = M_PI / 180 * angle_deg;
        hex << QPointF(m_center.x() + m_size * qCos(angle_rad),
                       m_center.y() + m_size * qSin(angle_rad));
    }
    setPolygon(hex);

    // Set brush based on type
    QBrush brush(Qt::white);
    if (m_type == Water) {
        brush = Qt::blue;
    } else if (m_type == Rock) {
        brush = Qt::darkGray;
    } else if (m_type == Goal) {
        brush = Qt::yellow;
    }
    setBrush(brush);
    setPen(QPen(Qt::gray));
    setFlag(QGraphicsItem::ItemIsSelectable);
    
    // Store original appearance for highlighting
    m_originalBrush = brush;
    m_originalPen = QPen(Qt::gray);
}

// Getters implementation
QPointF Cell::getCenter() const { return m_center; }
Cell::CellType Cell::getType() const { return m_type; }
bool Cell::isOccupied() const { return m_agent != nullptr; }
Agent* Cell::getAgent() const { return m_agent; }
int Cell::getRow() const { return m_row; }
int Cell::getCol() const { return m_col; }

void Cell::setAgent(Agent* agent) {
    m_agent = agent;
    if (agent) {
        agent->setPos(m_center);
    }
}

int Cell::distanceTo(Cell* other) const {
    if (!other) return INT_MAX;

    // Axial coordinates for hex grid
    int q1 = m_col;
    int r1 = m_row - (m_col - (m_col & 1)) / 2;

    int q2 = other->getCol();
    int r2 = other->getRow() - (other->getCol() - (other->getCol() & 1)) / 2;

    // Manhattan distance in cube coordinates
    return (abs(q1 - q2) + abs(q1 + r1 - q2 - r2) + abs(r1 - r2)) / 2;
}

QList<Cell*> Cell::getAdjacentCells() const {
    QList<Cell*> adjacent;
    // Implementation depends on how you store cells in GamePage
    // This should be implemented in GamePage and called from there
    return adjacent;
}

void Cell::resetBrush() {
    QBrush brush;
    switch(m_type) {
    case Water: brush = Qt::blue; break;
    case Rock: brush = Qt::darkGray; break;
    case Goal: brush = Qt::yellow; break;
    default: brush = Qt::white;
    }
    setBrush(brush);
    m_originalBrush = brush; // Update stored original brush
}

bool Cell::canPlaceAgent(Agent* agent) const {
    // Check if cell is already occupied
    if (isOccupied()) return false;
    
    // Check if agent can be placed on this cell type
    if (!agent) return false;
    
    return agent->canBePlacedOn(m_type);
}

void Cell::highlightForPlacement(bool canPlace) {
    if (m_isHighlighted) return; // Already highlighted
    
    m_isHighlighted = true;
    
    if (canPlace) {
        // Green highlight for valid placement
        setBrush(QBrush(QColor(144, 238, 144, 150))); // Light green with transparency
        setPen(QPen(QColor(0, 255, 0), 3)); // Bright green border
    } else {
        // Red highlight for invalid placement
        setBrush(QBrush(QColor(255, 182, 193, 150))); // Light red with transparency
        setPen(QPen(QColor(255, 0, 0), 3)); // Bright red border
    }
}

void Cell::clearHighlight() {
    if (!m_isHighlighted) return; // Not highlighted
    
    m_isHighlighted = false;
    setBrush(m_originalBrush);
    setPen(m_originalPen);
}

void Cell::setPlacementHint(bool canPlace) {
    if (canPlace) {
        highlightForPlacement(true);
    } else {
        highlightForPlacement(false);
    }
}

void Cell::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsPolygonItem::mousePressEvent(event);
    emit clicked(this);
}
