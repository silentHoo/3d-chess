#include <gtest/gtest.h>

#include <random>

#include "logic/GameState.h"
#include "logic/IncrementalMaterialAndPSTEvaluator.h"

using namespace std;

TEST(GameState, defaultState) {
    GameState s;
    EXPECT_EQ(PlayerColor::White, s.getNextPlayer());
}

TEST(GameState, equality) {
    GameState a, b;
    EXPECT_EQ(a, b);

    Turn t = Turn::move(Piece(White, Pawn), B2, B4);
    a.applyTurn(t);
    EXPECT_NE(a.getNextPlayer(), b.getNextPlayer());
    EXPECT_NE(a.getChessBoard(), b.getChessBoard());

    b.applyTurn(t);
    EXPECT_EQ(a.getNextPlayer(), b.getNextPlayer());
    EXPECT_EQ(a.getChessBoard(), b.getChessBoard());
}

TEST(GameState, fiftyMoveRule) {
    GameState gs(ChessBoard::fromFEN("8/k7/8/8/8/8/K7/8 b - - 99 90"));
    ASSERT_FALSE(gs.isGameOver());
    ASSERT_FALSE(gs.isDrawDueTo50MovesRule());

    gs.applyTurn(Turn::move(Piece(Black, King), A7, A6));
    EXPECT_TRUE(gs.isGameOver()) << gs;
    EXPECT_TRUE(gs.isDrawDueTo50MovesRule()) << gs;
    EXPECT_EQ(NoPlayer, gs.getWinner()) << gs;
}
