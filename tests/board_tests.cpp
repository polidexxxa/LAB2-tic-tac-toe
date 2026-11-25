#include <gtest/gtest.h>

#include <utility>

#include "game/Board.hpp"

namespace {

bool HasDuplicatePoints(const ListSequence<Point>& points) {
    auto* outer = points.GetEnumerator();
    int index = 0;

    while (outer->MoveNext()) {
        Point target = std::as_const(*outer).GetCurrent();
        auto* inner = points.GetEnumerator();
        int otherIndex = 0;

        while (inner->MoveNext()) {
            Point point = std::as_const(*inner).GetCurrent();

            if (otherIndex > index && point == target) {
                delete inner;
                delete outer;
                return true;
            }

            ++otherIndex;
        }

        delete inner;
        ++index;
    }

    delete outer;
    return false;
}

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

bool SameCandidates(
    const ListSequence<Point>& first,
    const ListSequence<Point>& second
) {
    if (first.GetLength() != second.GetLength()) {
        return false;
    }

    auto* it = first.GetEnumerator();

    while (it->MoveNext()) {
        Point target = std::as_const(*it).GetCurrent();

        if (CountPoint(first, target) != CountPoint(second, target)) {
            delete it;
            return false;
        }
    }

    delete it;
    return true;
}

bool SamePosition(const Board& first, const Board& second) {
    if (first.GetMoveCount() != second.GetMoveCount()) {
        return false;
    }

    auto entries = first.GetOccupiedCellsList();
    auto* it = entries.GetEnumerator();

    while (it->MoveNext()) {
        auto entry = std::as_const(*it).GetCurrent();

        if (second.GetCell(entry.key) != entry.value) {
            delete it;
            return false;
        }
    }

    delete it;
    return true;
}

} // namespace

TEST(BoardTests, GetOpponentReturnsOppositePlayer) {
    EXPECT_EQ(GetOpponent(Cell::X), Cell::O);
    EXPECT_EQ(GetOpponent(Cell::O), Cell::X);
    EXPECT_EQ(GetOpponent(Cell::Empty), Cell::Empty);
}

TEST(BoardTests, NewBoardIsEmpty) {
    Board board;

    EXPECT_EQ(board.GetMoveCount(), 0);
    EXPECT_TRUE(board.IsEmpty(Point(0, 0)));
    EXPECT_EQ(board.GetCell(Point(100, -100)), Cell::Empty);
}

TEST(BoardTests, MakeMoveStoresPlayer) {
    Board board;

    ASSERT_TRUE(board.MakeMove(Point(-3, 8), Cell::X));

    EXPECT_EQ(board.GetMoveCount(), 1);
    EXPECT_EQ(board.GetCell(Point(-3, 8)), Cell::X);
    EXPECT_FALSE(board.IsEmpty(Point(-3, 8)));
}

TEST(BoardTests, OccupiedCellIsRejectedWithoutChangingPlayer) {
    Board board;
    ASSERT_TRUE(board.MakeMove(Point(0, 0), Cell::X));

    EXPECT_FALSE(board.MakeMove(Point(0, 0), Cell::O));
    EXPECT_EQ(board.GetCell(Point(0, 0)), Cell::X);
    EXPECT_EQ(board.GetMoveCount(), 1);
}

TEST(BoardTests, EmptyPlayerIsRejected) {
    Board board;

    EXPECT_FALSE(board.MakeMove(Point(0, 0), Cell::Empty));
    EXPECT_EQ(board.GetMoveCount(), 0);
    EXPECT_TRUE(board.IsEmpty(Point(0, 0)));
}

TEST(BoardTests, UndoRemovesExistingMove) {
    Board board;

    ASSERT_TRUE(board.MakeMove(Point(2, 3), Cell::O));
    ASSERT_TRUE(board.UndoMove(Point(2, 3)));

    EXPECT_EQ(board.GetMoveCount(), 0);
    EXPECT_EQ(board.GetCell(Point(2, 3)), Cell::Empty);
    EXPECT_TRUE(board.IsEmpty(Point(2, 3)));
}

TEST(BoardTests, UndoMissingCellReturnsFalse) {
    Board board;

    EXPECT_FALSE(board.UndoMove(Point(2, 3)));
    EXPECT_EQ(board.GetMoveCount(), 0);
}

TEST(BoardTests, EmptyBoardOffersOrigin) {
    Board board;
    auto candidates = board.GetCandidateMoves();

    ASSERT_EQ(candidates.GetLength(), 1);
    EXPECT_EQ(candidates.GetFirst(), Point(0, 0));
}

TEST(BoardTests, CandidatesAreUniqueAndEmpty) {
    Board board;

    ASSERT_TRUE(board.MakeMove(Point(0, 0), Cell::X));
    ASSERT_TRUE(board.MakeMove(Point(1, 0), Cell::O));
    ASSERT_TRUE(board.MakeMove(Point(0, 1), Cell::X));

    auto candidates = board.GetCandidateMoves();

    EXPECT_GT(candidates.GetLength(), 0);
    EXPECT_FALSE(HasDuplicatePoints(candidates));

    auto* it = candidates.GetEnumerator();

    while (it->MoveNext()) {
        Point point = std::as_const(*it).GetCurrent();

        EXPECT_TRUE(board.IsEmpty(point))
            << "Occupied candidate: (" << point.x
            << ", " << point.y << ")";
    }

    delete it;
}

TEST(BoardTests, MakeAndUndoRestorePositionAndCandidates) {
    Board board;
    ASSERT_TRUE(board.MakeMove(Point(0, 0), Cell::X));

    Board before = board;
    auto beforeCandidates = board.GetCandidateMoves();

    ASSERT_TRUE(board.MakeMove(Point(1, 0), Cell::O));
    ASSERT_TRUE(board.UndoMove(Point(1, 0)));

    auto afterCandidates = board.GetCandidateMoves();

    EXPECT_TRUE(SamePosition(board, before));
    EXPECT_TRUE(SameCandidates(beforeCandidates, afterCandidates));
}

TEST(BoardTests, UndoAllMovesRestoresEmptyBoardCandidates) {
    Board board;

    ASSERT_TRUE(board.MakeMove(Point(0, 0), Cell::X));
    ASSERT_TRUE(board.MakeMove(Point(1, 0), Cell::O));

    ASSERT_TRUE(board.UndoMove(Point(1, 0)));
    ASSERT_TRUE(board.UndoMove(Point(0, 0)));

    auto candidates = board.GetCandidateMoves();

    EXPECT_EQ(board.GetMoveCount(), 0);
    ASSERT_EQ(candidates.GetLength(), 1);
    EXPECT_EQ(candidates.GetFirst(), Point(0, 0));
}

TEST(BoardTests, BoundsIncludeAddedMoves) {
    Board board;

    ASSERT_TRUE(board.MakeMove(Point(-4, 7), Cell::X));
    ASSERT_TRUE(board.MakeMove(Point(6, -3), Cell::O));

    EXPECT_EQ(board.GetMinX(), -4);
    EXPECT_EQ(board.GetMaxX(), 6);
    EXPECT_EQ(board.GetMinY(), -3);
    EXPECT_EQ(board.GetMaxY(), 7);
}

TEST(BoardTests, CopyIsIndependent) {
    Board original;
    ASSERT_TRUE(original.MakeMove(Point(0, 0), Cell::X));

    auto originalCandidates = original.GetCandidateMoves();
    Board copy = original;

    ASSERT_TRUE(copy.MakeMove(Point(1, 0), Cell::O));
    ASSERT_TRUE(copy.UndoMove(Point(0, 0)));

    EXPECT_EQ(original.GetMoveCount(), 1);
    EXPECT_EQ(original.GetCell(Point(0, 0)), Cell::X);
    EXPECT_TRUE(original.IsEmpty(Point(1, 0)));

    auto candidatesAfterCopyChanges = original.GetCandidateMoves();

    EXPECT_TRUE(SameCandidates(
        originalCandidates,
        candidatesAfterCopyChanges
    ));
}

class BoardWinTests : public ::testing::TestWithParam<Point> {};

TEST_P(BoardWinTests, DetectsFiveForBothPlayers) {
    Point direction = GetParam();

    for (Cell player : {Cell::X, Cell::O}) {
        Board board;

        for (int i = 0; i < 5; ++i) {
            ASSERT_TRUE(board.MakeMove(
                Point(-10 + i * direction.x,
                      -10 + i * direction.y),
                player
            ));
        }

        Point middle(
            -10 + 2 * direction.x,
            -10 + 2 * direction.y
        );

        EXPECT_TRUE(board.CheckWin(middle, player));
    }
}

TEST_P(BoardWinTests, FourIsNotVictory) {
    Board board;
    Point direction = GetParam();

    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(board.MakeMove(
            Point(i * direction.x, i * direction.y),
            Cell::X
        ));
    }

    EXPECT_FALSE(board.CheckWin(Point(0, 0), Cell::X));
}

TEST_P(BoardWinTests, GapBreaksWinningLine) {
    Board board;
    Point direction = GetParam();

    for (int i : {0, 1, 3, 4, 5}) {
        ASSERT_TRUE(board.MakeMove(
            Point(i * direction.x, i * direction.y),
            Cell::X
        ));
    }

    Point last(4 * direction.x, 4 * direction.y);

    EXPECT_FALSE(board.CheckWin(last, Cell::X));
}

TEST_P(BoardWinTests, SixInARowIsVictory) {
    Board board;
    Point direction = GetParam();

    for (int i = 0; i < 6; ++i) {
        ASSERT_TRUE(board.MakeMove(
            Point(i * direction.x, i * direction.y),
            Cell::O
        ));
    }

    Point middle(2 * direction.x, 2 * direction.y);

    EXPECT_TRUE(board.CheckWin(middle, Cell::O));
}

INSTANTIATE_TEST_SUITE_P(
    AllDirections,
    BoardWinTests,
    ::testing::Values(
        Point(1, 0),
        Point(0, 1),
        Point(1, 1),
        Point(1, -1)
    )
);