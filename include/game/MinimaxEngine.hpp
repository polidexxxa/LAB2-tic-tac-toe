#ifndef _MINIMAX_ENGINE_HPP_
#define _MINIMAX_ENGINE_HPP_

#include "Board.hpp"
#include "Point.hpp"
#include "containers/ListSequence.hpp"

namespace EvalWeights {
    constexpr int WIN_SCORE = 1000000;
    constexpr int OPEN_FOUR = 50000;
    constexpr int OPEN_THREE = 3000;
    constexpr int OPEN_TWO = 200;
}

struct SearchResult {
    Point best_move;
    int score = 0;
    long long evaluated_nodes = 0;
};

class MinimaxEngine {
private:
    long long node_counter = 0;

    int EvaluateWindow(int my_count, int opp_count) const {
        if (my_count > 0 && opp_count > 0) {
            return 0;
        }
        static constexpr int scores[6] = {
            0, 10, EvalWeights::OPEN_TWO, EvalWeights::OPEN_THREE, EvalWeights::OPEN_FOUR, EvalWeights::WIN_SCORE
        };

        if (my_count > 0) return scores[my_count];
        if (opp_count > 0) return -scores[opp_count] * 2;

        return 0;
    }

    int EvaluateBoard(const Board& board, Cell ai_player) const {
        Cell human_player = GetOpponent(ai_player);
        int total_score = 0;

        if (board.GetMoveCount() == 0) return 0;

        int min_x = board.GetMinX() - 1;
        int max_x = board.GetMaxX() + 1;
        int min_y = board.GetMinY() - 1;
        int max_y = board.GetMaxY() + 1;

        static const int directions[4][2] = {
            {1, 0}, {0, 1}, {1, 1}, {1, -1}
        };

        for (int x = min_x; x <= max_x; ++x) {
            for (int y = min_y; y <= max_y; ++y) {
                Point start(x, y);

                for (const auto& dir : directions) {
                    int my_count = 0;
                    int opp_count = 0;

                    for (int step = 0; step < 5; ++step) {
                        Point curr(start.x + dir[0] * step, start.y + dir[1] * step);
                        Cell c = board.GetCell(curr);
                        if (c == ai_player) my_count++;
                        else if (c == human_player) opp_count++;
                    }

                    total_score += EvaluateWindow(my_count, opp_count);
                }
            }
        }
        return total_score;
    }

    int AlphaBeta(Board& board, int depth, int alpha, int beta, bool is_maximizing, Cell ai_player, Point last_move) {
        node_counter++;
        Cell current_mover = is_maximizing ? GetOpponent(ai_player) : ai_player;
        int q = is_maximizing ? 1 : -1;

        if (board.GetMoveCount() > 0 && board.CheckWin(last_move, current_mover)) {
            return -q * (EvalWeights::WIN_SCORE - (10 - depth));
        }

        if (depth == 0) {
            return EvaluateBoard(board, ai_player);
        }

        ListSequence<Point> candidates = board.GetCandidateMoves();
        if (candidates.GetLength() == 0) {
            return 0;
        }

        Cell current_player = is_maximizing ? ai_player : GetOpponent(ai_player);
        int best_eval = -q * 2000000;

        IEnumerator<Point>* it = candidates.GetEnumerator();
        while (it->MoveNext()) {
            Point pt = it->GetCurrent();
            board.MakeMove(pt, current_player);

            int eval = AlphaBeta(board, depth - 1, alpha, beta, !is_maximizing, ai_player, pt);
            board.UndoMove(pt);

            if (q * eval > q * best_eval) {
                best_eval = eval;
            }

            if (is_maximizing) {
                if (eval > alpha) alpha = eval;
            } else {
                if (eval < beta) beta = eval;
            }

            if (beta <= alpha) {
                break;
            }
        }
        delete it;

        return best_eval;
    }

    bool FindWinningMove(Board& board, Cell player, const ListSequence<Point>& candidates, Point& winning_move) {
        IEnumerator<Point>* it = candidates.GetEnumerator();

        while (it->MoveNext()) {
            Point pt = it->GetCurrent();

            board.MakeMove(pt, player);
            bool win = board.CheckWin(pt, player);
            board.UndoMove(pt);

            if (win) {
                winning_move = pt;
                delete it;
                return true;
            }
        }
        delete it;
        return false;
    }

public:
    MinimaxEngine() = default;

    SearchResult FindBestMove(Board& board, Cell ai_player, int depth = 3) {
        node_counter = 0;
        SearchResult result;

        ListSequence<Point> candidates = board.GetCandidateMoves();
        if (candidates.GetLength() == 0) {
            result.best_move = Point(0, 0);
            return result;
        }

        Cell human_player = GetOpponent(ai_player);
        Point forced_move;

        if (FindWinningMove(board, ai_player, candidates, forced_move)) {
            return { forced_move, EvalWeights::WIN_SCORE, ++node_counter };
        }

        if (FindWinningMove(board, human_player, candidates, forced_move)) {
            return { forced_move, EvalWeights::WIN_SCORE / 2, ++node_counter };
        }

        Point best_pt = candidates.Get(0);
        int best_score = -2000000;
        int alpha = -2000000;
        int beta = 2000000;

        IEnumerator<Point>* it = candidates.GetEnumerator();
        while (it->MoveNext()) {
            Point pt = it->GetCurrent();
            board.MakeMove(pt, ai_player);

            int score = AlphaBeta(board, depth - 1, alpha, beta, false, ai_player, pt);
            board.UndoMove(pt);

            if (score > best_score) {
                best_score = score;
                best_pt = pt;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
        delete it;

        result.best_move = best_pt;
        result.score = best_score;
        result.evaluated_nodes = node_counter;

        return result;
    }
};

#endif // _MINIMAX_ENGINE_HPP_