#include "Evaluators.h"

#include "GameState.h"
#include <array>
#include <iostream>

Score MaterialEvaluator::getScore(const GameState& gameState) const {
    return getMaterialWorth(gameState.getNextPlayer(), gameState.getChessBoard());
}

using PieceSquareTable = std::array<Score, NUM_FIELDS>;

namespace {
/**
* @brief Piece square table as proposed by http://chessprogramming.wikispaces.com/Simplified+evaluation+function#Piece-Square
* @note Two dimensional. Indexable by Piece then Field from blacks POV.
*/
    const std::array<PieceSquareTable, NUM_PIECETYPES> PIECE_SQUARE_TABLE = { {
    {   // King
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        20, 20, 0, 0, 0, 0, 20, 20,
        20, 30, 10, 0, 0, 10, 30, 20
    },

    {   // Queen
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 5, 5, 5, 0, -10,
        -5, 0, 5, 5, 5, 5, 0, -5,
        0, 0, 5, 5, 5, 5, 0, -5,
        -10, 5, 5, 5, 5, 5, 0, -10,
        -10, 0, 5, 0, 0, 0, 0, -10,
        -20, -10, -10, -5, -5, -10, -10, -20
    },

    {   // Bishop
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 10, 10, 5, 0, -10,
        -10, 5, 5, 10, 10, 5, 5, -10,
        -10, 0, 10, 10, 10, 10, 0, -10,
        -10, 10, 10, 10, 10, 10, 10, -10,
        -10, 5, 0, 0, 0, 0, 5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20,
    },

    { // Knight
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50,
    },

    {   // Rooks
        0, 0, 0, 0, 0, 0, 0, 0,
        5, 10, 10, 10, 10, 10, 10, 5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        0, 0, 0, 5, 5, 0, 0, 0
    },

    {   // Pawn
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5, 5, 10, 25, 25, 10, 5, 5,
        0, 0, 0, 20, 20, 0, 0, 0,
        5, -5, -10, 0, 0, -10, -5, 5,
        5, 10, 10, -20, -20, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0
    }
}};

/**
* @brief Piece values as proposed by http://chessprogramming.wikispaces.com/Simplified+evaluation+function#Piece-Square
* @note Adjusted for indexing with Piece enum type
*/
const std::array<Score, NUM_PIECETYPES> PIECE_VALUES = {
    20000,  // king
    900,    // queen
    330,    // bishop
    320,    // knight
    500,    // rook
    100,    // pawn
};

} // namespace


Score MaterialEvaluator::getMaterialWorth(const PlayerColor player, const ChessBoard& board) const {
    // Uses fixed scoring for pieces. Kings are set
    // at a value higher than all other pieces combined to make
    // their loss unacceptable
    Score score = 0;

    // Calculate from white's perspective
    Field position = A1;
    for (Piece& piece: board.getBoard()) {
        if (piece.type <= Pawn) {
            const Field psqPosition = (piece.player == Black) ? position : flipHorizontal(position);

            const Score pieceSquareScore = PIECE_SQUARE_TABLE[piece.type][psqPosition];
            const Score pieceOverallScore = PIECE_VALUES[piece.type] + pieceSquareScore;
            
            if (piece.player == player) {
                score += pieceOverallScore;

            }
            else {
                score -= pieceOverallScore;
            }
        }
        position = nextField(position);
    }

    return score;
}


IncrementalBoardEvaluator::IncrementalBoardEvaluator()
    : m_estimatedScore(0) {
    // Empty
}

IncrementalBoardEvaluator::IncrementalBoardEvaluator(const std::array<Piece, 64> &board)
    : m_estimatedScore(estimateFullBoard(board)) {
}

Score IncrementalBoardEvaluator::estimateFullBoard(const std::array<Piece, 64> &board) {
    // Calculate from white's perspective
    Score score = 0;

    Field position = A1;
    for (const Piece& piece : board) {
        if (piece.type <= Pawn) {
            const Field psqPosition = (piece.player == Black) ? position : flipHorizontal(position);

            const Score pieceSquareScore = PIECE_SQUARE_TABLE[piece.type][psqPosition];
            const Score pieceOverallScore = PIECE_VALUES[piece.type] + pieceSquareScore;

            if (piece.player == White) {
                score += pieceOverallScore;
            }
            else {
                score -= pieceOverallScore;
            }
        }
        position = nextField(position);
    }

    return score;
}

void IncrementalBoardEvaluator::moveIncrement(const Turn& turn) {
    // Only effects PSQT evaluation due to the move. Always from white's perspective.

    if (turn.piece.player == Black) {
        m_estimatedScore += PIECE_SQUARE_TABLE[turn.piece.type][turn.from];
        m_estimatedScore -= PIECE_SQUARE_TABLE[turn.piece.type][turn.to];
    }
    else {
        m_estimatedScore -= PIECE_SQUARE_TABLE[turn.piece.type][flipHorizontal(turn.from)];
        m_estimatedScore += PIECE_SQUARE_TABLE[turn.piece.type][flipHorizontal(turn.to)];
    }
}

void IncrementalBoardEvaluator::captureIncrement(Field field, const Piece& piece) {
    // Effects material and PSQT evaluation
    if (piece.player == Black) {
        m_estimatedScore += PIECE_SQUARE_TABLE[piece.type][field];
        m_estimatedScore += PIECE_VALUES[piece.type];
    }
    else {
        m_estimatedScore -= PIECE_SQUARE_TABLE[piece.type][flipHorizontal(field)];
        m_estimatedScore -= PIECE_VALUES[piece.type];
    }
}

Score IncrementalBoardEvaluator::getScore(PlayerColor color) const {
    return color == White ? m_estimatedScore : -m_estimatedScore;
}

bool IncrementalBoardEvaluator::operator == (const IncrementalBoardEvaluator& other) const {
    return m_estimatedScore == other.m_estimatedScore;
}