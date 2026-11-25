#ifndef _BOARD_HPP_
#define _BOARD_HPP_

#include "Point.hpp"
#include "containers/BSTDictionary.hpp"
#include "containers/ListSequence.hpp"

enum class Cell {
    Empty = 0,
    X = 1,
    O = 2
};

inline Cell GetOpponent(Cell player) {
    if (player == Cell::X) return Cell::O;
    if (player == Cell::O) return Cell::X;
    return Cell::Empty;
}

class Board {
private:
    BSTDictionary<Point, Cell> occupied_cells;
    BSTDictionary<Point, int> neighbor_counts;

    int min_x = 0, max_x = 0;
    int min_y = 0, max_y = 0;

    void UpdateNeighborsOnAdd(const Point& pt, int radius = 2) {
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dy = -radius; dy <= radius; ++dy) {
                if (dx == 0 && dy == 0) {
                    continue;
                }

                Point neighbor(pt.x + dx, pt.y + dy);
                
                if (!occupied_cells.ContainsKey(neighbor)) {
                    int current_count = 0;
                    neighbor_counts.TryGet(neighbor, current_count);
                    neighbor_counts.Add(neighbor, current_count + 1);
                }
            }
        }
        if (neighbor_counts.ContainsKey(pt)) {
            neighbor_counts.Remove(pt);
        }
    }

    void UpdateNeighborsOnRemove(const Point& pt, int radius = 2) {
        int occupied_around = 0;
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dy = -radius; dy <= radius; ++dy) {
                if (dx == 0 && dy == 0) continue;
                Point neighbor(pt.x + dx, pt.y + dy);
                if (occupied_cells.ContainsKey(neighbor)) {
                    occupied_around++;
                } else {
                    if (neighbor_counts.ContainsKey(neighbor)) {
                        int cnt = neighbor_counts.Get(neighbor) - 1;
                        if (cnt <= 0) {
                            neighbor_counts.Remove(neighbor);
                        } else {
                            neighbor_counts.Add(neighbor, cnt);
                        }
                    }
                }
            }
        }
        if (occupied_around > 0) {
            neighbor_counts.Add(pt, occupied_around);
        }
    }

    void UpdateBounds(const Point& pt) {
        if (occupied_cells.GetCount() == 1) {
            min_x = max_x = pt.x;
            min_y = max_y = pt.y;
        } else {
            if (pt.x < min_x) min_x = pt.x;
            if (pt.x > max_x) max_x = pt.x;
            if (pt.y < min_y) min_y = pt.y;
            if (pt.y > max_y) max_y = pt.y;
        }
    }

public:
    Board() = default;

    Cell GetCell(const Point& pt) const {
        Cell cell = Cell::Empty;
        occupied_cells.TryGet(pt, cell);
        return cell;
    }

    bool IsEmpty(const Point& pt) const {
        return !occupied_cells.ContainsKey(pt);
    }

    int GetMoveCount() const {
        return occupied_cells.GetCount();
    }

    bool MakeMove(const Point& pt, Cell player) {
        if (!IsEmpty(pt) || player == Cell::Empty) {
            return false;
        }
        occupied_cells.Add(pt, player);
        UpdateNeighborsOnAdd(pt);
        UpdateBounds(pt);
        return true;
    }

    bool UndoMove(const Point& pt) {
        if (!occupied_cells.ContainsKey(pt)) {
            return false;
        }
        occupied_cells.Remove(pt);
        UpdateNeighborsOnRemove(pt);
        return true;
    }

    bool CheckWin(const Point& last_pt, Cell player) const {
        static const int directions[4][2] = {
            {1, 0}, {0, 1}, {1, 1}, {1, -1}
        };

        for (const auto& dir : directions) {
            int count = 1;

            for (int step = 1; step < 5; ++step) {
                Point p(last_pt.x + dir[0] * step, last_pt.y + dir[1] * step);
                if (GetCell(p) == player) {
                    count++;
                } else {
                    break;
                }
            }

            for (int step = 1; step < 5; ++step) {
                Point p(last_pt.x - dir[0] * step, last_pt.y - dir[1] * step);
                if (GetCell(p) == player) {
                    count++;
                } else {
                    break;
                }
            }

            if (count >= 5) {
                return true;
            }
        }
        return false;
    }

    ListSequence<Point> GetCandidateMoves() const {
        ListSequence<Point> candidates;
        if (occupied_cells.GetCount() == 0) {
            candidates.Append(Point(0, 0));
            return candidates;
        }

        auto entries = neighbor_counts.GetEntries();
        auto* it = entries.GetEnumerator();
        while (it->MoveNext()) {
            candidates.Append(it->GetCurrent().key);
        }
        delete it;

        return candidates;
    }

    int GetMinX() const { return min_x; }
    int GetMaxX() const { return max_x; }
    int GetMinY() const { return min_y; }
    int GetMaxY() const { return max_y; }

    ListSequence<KeyValuePair<Point, Cell>> GetOccupiedCellsList() const {
        return occupied_cells.GetEntries();
    }
    
};

#endif // _BOARD_HPP_