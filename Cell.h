#ifndef CELL_H
#define CELL_H

#include <QObject>
#include <QGraphicsPolygonItem>
#include <QPointF>
#include <qbrush.h>
#include <qpen.h>

class Agent;

class Cell : public QObject, public QGraphicsPolygonItem {
    Q_OBJECT
public:
    enum CellType {
        Normal,
        Water,
        Rock,
        Goal
    };

    Cell(int row, int col, CellType type, QGraphicsItem* parent = nullptr);

    void resetBrush();

    // Getters
    QPointF getCenter() const;
    CellType getType() const;
    bool isOccupied() const;
    Agent* getAgent() const;
    int getRow() const;
    int getCol() const;

    // Setters
    void setAgent(Agent* agent);

    // Game logic
    int distanceTo(Cell* other) const;
    QList<Cell*> getAdjacentCells() const;
    bool canPlaceAgent(Agent* agent) const;
    
    // Visual feedback for placement
    void highlightForPlacement(bool canPlace);
    void clearHighlight();
    void setPlacementHint(bool canPlace);

signals:
    void clicked(Cell* cell);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void createHexagon();

    CellType m_type;
    int m_row;
    int m_col;
    QPointF m_center;
    Agent* m_agent = nullptr;
    static const int m_size = 30;
    
    // Visual state tracking
    bool m_isHighlighted = false;
    QBrush m_originalBrush;
    QPen m_originalPen;
};

#endif // CELL_H
