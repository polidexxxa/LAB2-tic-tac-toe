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

    constexpr int EVAL_LIMIT = WIN_SCORE / 2;

    constexpr int OPPONENT_FACTOR = 2;
}

struct SearchResult {
    Point best_move;
    int score = 0;
    long long evaluated_nodes = 0;
};

class MinimaxEngine {
private:
    static constexpr int INF = 2000000;
    static constexpr int MIN_DEPTH = 2;
    static constexpr int DIRECTIONS[4][2] = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };

    long long node_counter = 0;

    int EvaluateWindow(int my_count, int opp_count) const {
        if (my_count > 0 && opp_count > 0) {
            return 0;
        }
        static constexpr int scores[5] = {
            0, 10, EvalWeights::OPEN_TWO, EvalWeights::OPEN_THREE, EvalWeights::OPEN_FOUR
        };

        if (my_count > 0) {
            return scores[my_count < 4 ? my_count : 4];
        }
        if (opp_count > 0) {
            return -scores[opp_count < 4 ? opp_count : 4] * EvalWeights::OPPONENT_FACTOR;
        }
        return 0;
    }

    int ClampScore(int score) const {
        if (score > EvalWeights::EVAL_LIMIT) return EvalWeights::EVAL_LIMIT;
        if (score < -EvalWeights::EVAL_LIMIT) return -EvalWeights::EVAL_LIMIT;
        return score;
    }

    
    int EvaluateBoard(const Board& board, Cell ai_player) const {
        auto entries = board.GetOccupiedCellsList();
        auto* it = entries.GetEnumerator();
        int total_score = 0;

        while (it->MoveNext()) {
            Point stone = it->GetCurrent().key;

            for (const auto& dir : DIRECTIONS) {
                for (int k = 0; k < 5; ++k) {
                    bool is_first_stone = true;
                    int my_count = 0;
                    int opp_count = 0;

                    for (int j = 0; j < 5; ++j) {
                        Point p(stone.x + (j - k) * dir[0],
                                stone.y + (j - k) * dir[1]);
                        Cell c = board.GetCell(p);

                        if (c != Cell::Empty && j < k) {
                            is_first_stone = false;
                            break;
                        }
                        if (c == ai_player) my_count++;
                        else if (c != Cell::Empty) opp_count++;
                    }

                    if (is_first_stone) {
                        total_score += EvaluateWindow(my_count, opp_count);
                    }
                }
            }
        }

        delete it;
        return total_score;
    }

    
    int MoveDelta(const Board& board, const Point& pt, bool by_ai, Cell ai_player) const {
        int delta = 0;

        for (const auto& dir : DIRECTIONS) {
            for (int k = 0; k < 5; ++k) {
                int my_count = 0;
                int opp_count = 0;

                for (int j = 0; j < 5; ++j) {
                    if (j == k) continue;

                    Point p(pt.x + (j - k) * dir[0],
                            pt.y + (j - k) * dir[1]);
                    Cell c = board.GetCell(p);

                    if (c == ai_player) my_count++;
                    else if (c != Cell::Empty) opp_count++;
                }

                int before = EvaluateWindow(my_count, opp_count);

                if (by_ai) my_count++;
                else opp_count++;

                delta += EvaluateWindow(my_count, opp_count) - before;
            }
        }
        return delta;
    }

    
    int AlphaBeta(Board& board, int depth, int alpha, int beta, bool is_maximizing,
                    Cell ai_player, int score, int ply, Point* best_out) {

        ListSequence<Point> candidates = board.GetCandidateMoves();
        if (candidates.GetLength() == 0) {
            return ClampScore(score);
        }

        Cell current_player = is_maximizing ? ai_player : GetOpponent(ai_player);
        int win_value = EvalWeights::WIN_SCORE - (ply + 1);
        int best_eval = is_maximizing ? -INF : INF;

        IEnumerator<Point>* it = candidates.GetEnumerator();
        while (it->MoveNext()) {
            Point pt = it->GetCurrent();
            ++node_counter;

            int eval;

            if (board.CheckWin(pt, current_player)) {
                eval = is_maximizing ? win_value : -win_value;
            } else {
                int child_score = score + MoveDelta(board, pt, is_maximizing, ai_player);

                if (depth == 1) {
                    eval = ClampScore(child_score);
                } else {
                    board.MakeMove(pt, current_player);
                    eval = AlphaBeta(board, depth - 1, alpha, beta, !is_maximizing,
                                    ai_player, child_score, ply + 1, nullptr);
                    board.UndoMove(pt);
                }
            }

            if (is_maximizing) {
                if (eval > best_eval) {
                    best_eval = eval;
                    if (best_out) *best_out = pt;
                }
                if (best_eval > alpha) alpha = best_eval;
                if (best_eval >= win_value) break;
            } else {
                if (eval < best_eval) best_eval = eval;
                if (best_eval < beta) beta = best_eval;
                if (best_eval <= -win_value) break;
            }

            if (beta <= alpha) {
                break;
            }
        }
        delete it;

        return best_eval;
    }

public:
    MinimaxEngine() = default;

    SearchResult FindBestMove(Board& board, Cell ai_player, int depth = 3) {
        node_counter = 0;

        if (depth < MIN_DEPTH) {
            depth = MIN_DEPTH;
        }

        Point best_pt(0, 0);
        int start_score = EvaluateBoard(board, ai_player);

        int best_score = AlphaBeta(board, depth, -INF, INF, true,
                                   ai_player, start_score, 0, &best_pt);

        SearchResult result;
        result.best_move = best_pt;
        result.score = best_score;
        result.evaluated_nodes = node_counter;

        return result;
    }
};

#endif // _MINIMAX_ENGINE_HPP_
