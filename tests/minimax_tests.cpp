#include <gtest/gtest.h>

#include <utility>

#include "game/Board.hpp"
#include "game/MinimaxEngine.hpp"

namespace {

int CountPoint(
    const ListSequence<Point>& points,
    const Point& target
) {
    int count = 0;
    auto* it = points.GetEnumerator();

    while (it->MoveNext()) {
        if (std::as_const(*it).GetCurrent() == target) {
            ++count;
        }
    }

    delete it;
    return count;
}

void ExpectSamePosition(const Board& actual, const Board& expected) {
    EXPECT_EQ(actual.GetMoveCount(), expected.GetMoveCount());

    auto entries = expected.GetOccupiedCellsList();
    auto* it = entries.GetEnumerator();

    while (it->MoveNext()) {
        auto entry = std::as_const(*it).GetCurrent();

        EXPECT_EQ(actual.GetCell(entry.key), entry.value)
            << "Changed cell: (" << entry.key.x
            << ", " << entry.key.y << ")";
    }

    delete it;
}

void ExpectSameCandidates(
    const ListSequence<Point>& actual,
    const ListSequence<Point>& expected
) {
    EXPECT_EQ(actual.GetLength(), expected.GetLength());

    auto* it = expected.GetEnumerator();

    while (it->MoveNext()) {
        Point point = std::as_const(*it).GetCurrent();

        EXPECT_EQ(
            CountPoint(actual, point),
            CountPoint(expected, point)
        ) << "Candidate: (" << point.x << ", " << point.y << ")";
    }

    delete it;
}

} // namespace

TEST(MinimaxTests, EmptyBoardSelectsOrigin) {
    Board board;
    MinimaxEngine engine;

    SearchResult result = engine.FindBestMove(board, Cell::O, 1);

    EXPECT_EQ(result.best_move, Point(0, 0));
    EXPECT_EQ(board.GetMoveCount(), 0);
    EXPECT_TRUE(board.IsEmpty(Point(0, 0)));
}

TEST(MinimaxTests, FindsImmediateWinningMove) {
    Board board;
    MinimaxEngine engine;

    // X O O O O _
    ASSERT_TRUE(board.MakeMove(Point(-1, 0), Cell::X));

    for (int x = 0; x < 4; ++x) {
        ASSERT_TRUE(board.MakeMove(Point(x, 0), Cell::O));
    }

    Board before = board;
    auto beforeCandidates = board.GetCandidateMoves();

    SearchResult result = engine.FindBestMove(board, Cell::O, 2);

    EXPECT_EQ(result.best_move, Point(4, 0));
    ExpectSamePosition(board, before);

    auto afterCandidates = board.GetCandidateMoves();
    ExpectSameCandidates(afterCandidates, beforeCandidates);
}

TEST(MinimaxTests, BlocksImmediateOpponentWinningMove) {
    Board board;
    MinimaxEngine engine;

    // O X X X X _
    ASSERT_TRUE(board.MakeMove(Point(-1, 0), Cell::O));

    for (int x = 0; x < 4; ++x) {
        ASSERT_TRUE(board.MakeMove(Point(x, 0), Cell::X));
    }

    Board before = board;
    auto beforeCandidates = board.GetCandidateMoves();

    SearchResult result = engine.FindBestMove(board, Cell::O, 2);

    EXPECT_EQ(result.best_move, Point(4, 0));
    ExpectSamePosition(board, before);

    auto afterCandidates = board.GetCandidateMoves();
    ExpectSameCandidates(afterCandidates, beforeCandidates);
}

TEST(MinimaxTests, PrefersOwnWinOverBlockingOpponent) {
    Board board;
    MinimaxEngine engine;

    // AI can win on row 0.
    ASSERT_TRUE(board.MakeMove(Point(-1, 0), Cell::X));

    // Human threatens to win on row 5.
    ASSERT_TRUE(board.MakeMove(Point(-1, 5), Cell::O));

    for (int x = 0; x < 4; ++x) {
        ASSERT_TRUE(board.MakeMove(Point(x, 0), Cell::O));
        ASSERT_TRUE(board.MakeMove(Point(x, 5), Cell::X));
    }

    Board before = board;

    SearchResult result = engine.FindBestMove(board, Cell::O, 2);

    EXPECT_EQ(result.best_move, Point(4, 0));
    ExpectSamePosition(board, before);
}

TEST(MinimaxTests, SupportsXAsAiPlayer) {
    Board board;
    MinimaxEngine engine;

    ASSERT_TRUE(board.MakeMove(Point(-1, 0), Cell::O));

    for (int x = 0; x < 4; ++x) {
        ASSERT_TRUE(board.MakeMove(Point(x, 0), Cell::X));
    }

    Board before = board;

    SearchResult result = engine.FindBestMove(board, Cell::X, 2);

    EXPECT_EQ(result.best_move, Point(4, 0));
    ExpectSamePosition(board, before);
}

TEST(MinimaxTests, FindsDiagonalWinningMoveAtNegativeCoordinates) {
    Board board;
    MinimaxEngine engine;

    ASSERT_TRUE(board.MakeMove(Point(-5, -5), Cell::X));

    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(board.MakeMove(
            Point(-4 + i, -4 + i),
            Cell::O
        ));
    }

    Board before = board;

    SearchResult result = engine.FindBestMove(board, Cell::O, 2);

    EXPECT_EQ(result.best_move, Point(0, 0));
    ExpectSamePosition(board, before);
}

TEST(MinimaxTests, RecursiveSearchReturnsLegalMoveAndRestoresBoard) {
    Board board;
    MinimaxEngine engine;

    ASSERT_TRUE(board.MakeMove(Point(0, 0), Cell::X));
    ASSERT_TRUE(board.MakeMove(Point(1, 0), Cell::O));
    ASSERT_TRUE(board.MakeMove(Point(0, 1), Cell::X));

    Board before = board;
    auto beforeCandidates = board.GetCandidateMoves();

    SearchResult result = engine.FindBestMove(board, Cell::O, 2);

    EXPECT_TRUE(board.IsEmpty(result.best_move));
    EXPECT_GT(CountPoint(beforeCandidates, result.best_move), 0);
    EXPECT_GT(result.evaluated_nodes, 0);

    ExpectSamePosition(board, before);

    auto afterCandidates = board.GetCandidateMoves();
    ExpectSameCandidates(afterCandidates, beforeCandidates);
}

TEST(MinimaxTests, SameEngineCanSearchDifferentBoards) {
    MinimaxEngine engine;

    Board first;
    ASSERT_TRUE(first.MakeMove(Point(-1, 0), Cell::X));

    for (int x = 0; x < 4; ++x) {
        ASSERT_TRUE(first.MakeMove(Point(x, 0), Cell::O));
    }

    SearchResult firstResult =
        engine.FindBestMove(first, Cell::O, 2);

    EXPECT_EQ(firstResult.best_move, Point(4, 0));

    Board second;
    ASSERT_TRUE(second.MakeMove(Point(0, -1), Cell::O));

    for (int y = 0; y < 4; ++y) {
        ASSERT_TRUE(second.MakeMove(Point(0, y), Cell::X));
    }

    SearchResult secondResult =
        engine.FindBestMove(second, Cell::O, 2);

    EXPECT_EQ(secondResult.best_move, Point(0, 4));
}