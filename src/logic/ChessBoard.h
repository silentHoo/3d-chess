/*
    Copyright (c) 2013-2014, Max Stark <max.stark88@googlemail.com>

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <stdarg.h>
#include <array>
#include <cmath>
#include <string>

#include "Turn.h"
#include "IncrementalMaterialAndPSTEvaluator.h"
#include "IncrementalZobristHasher.h"

#ifdef _MSC_VER
#include <intrin.h>

#ifdef _WIN64
#pragma intrinsic(_BitScanReverse64)
inline Field getFirstOccupiedField(BitBoard bb) {
    assert(bb != 0);
    assert(sizeof(Field) == sizeof(unsigned long));
    unsigned long result;
    _BitScanReverse64(&result, bb);
    return static_cast<Field>(result);
}

#else //_WIN32
#pragma intrinsic(_BitScanReverse)
inline Field getFirstOccupiedField(BitBoard bb) {
    assert(bb != 0);
    assert(sizeof(Field) == sizeof(long));
    unsigned long result;
    if (_BitScanReverse(&result, bb >> 32) == 0) {
        _BitScanReverse(&result, bb & 0xFFFFFFFF);
        return static_cast<Field>(result);
    }
    return static_cast<Field>(result + 32);
}

#endif //_WIN64
#elif defined(__GNUC__) || defined(__clang__)
inline Field getFirstOccupiedField(BitBoard bb) {
    assert(bb != 0);
    assert(sizeof(Field) == sizeof(int));

    return static_cast<Field>(63 - __builtin_clzll(bb));
}
#else
// This shouldn't be used really. Too slow.
inline Field getFirstOccupiedField(BitBoard bb) {
    return static_cast<Field>(static_cast<int>(std::log2((double)bb)));
}
#endif

/* Some helpful macros for bit pushing */
#define BB_SCAN(   bb)         getFirstOccupiedField(bb) /* returns the field of MS1B */
#define BIT_SET(   bb, field) (bb |=   (BitBoard)1 << (field))
#define BIT_CLEAR( bb, field) (bb &= ~((BitBoard)1 << (field)))
#define BIT_TOGGLE(bb, field) (bb ^=   (BitBoard)1 << (field))
#define BIT_ISSET( bb, field) (bb & (  (BitBoard)1 << (field)))



/**
 * @brief Chessboard representation and logic implementation.
 */
class ChessBoard {
    friend class TurnGenerator;
    friend class IncrementalZobristHasher;

public:
    ChessBoard();
    ChessBoard(std::array<Piece, 64> board,
               PlayerColor nextPlayer,
               std::array<bool, NUM_PLAYERS> shortCastleRight,
               std::array<bool, NUM_PLAYERS> longCastleRight,
               Field enPassantSquare,
               int halfMoveClock,
               int fullMoveClock);
    
    //! Applies the given turn on current chessboard.
    void applyTurn(const Turn& t);
    //! Returns the chessboard in array representation.
    std::array<Piece, 64> getBoard() const;

    //! Returns true if black pieces are on the board.
    bool hasBlackPieces() const;
    //! Returns true if white pieces are on the board.
    bool hasWhitePieces() const;
    //! Return next player to make a turn
    PlayerColor getNextPlayer() const;

    //! Returns the current estimated score.
    Score getScore(PlayerColor color, size_t depth = 0) const;
    //! Returns hash for current position
    Hash getHash() const;
    //! Returns half move clock
    int getHalfMoveClock() const;
    //! Returns full move clock
    int getFullMoveClock() const;
    
    /**
     * @brief Create a chessboard from a Forsyth–Edwards Notation string.
     * http://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
     * @warning This function does no validation. Do not pass invalid FEN.
     * @param fen FEN String.
     */
    static ChessBoard fromFEN(const std::string& fen);
    
    /**
     * @brief Converts the current board state into FEN notation.
     * @return State in FEN notation.
     */
    std::string toFEN() const;

    //! Returns the field where en-passant rights exist. ERR if none.
    Field getEnPassantSquare() const;
    //! Returns short castle rights for players.
    std::array<bool, NUM_PLAYERS> getShortCastleRights() const;
    //! Returns long castle rights for players.
    std::array<bool, NUM_PLAYERS> getLongCastleRights() const;

    //! Returns whether the king of the player is in check or not.
    std::array<bool, NUM_PLAYERS> getKingInCheck() const;

    //! Gameover-Flag for stalemate position (gameover, no winner).
    bool isStalemate() const;
    //! Gameover-Flag for checkmate.
    std::array<bool, NUM_PLAYERS> getCheckmate() const;
    //! Returns true if the game is over
    bool isGameOver() const;
    //! Returns true if the game is draw due to the 50 moves rule
    bool isDrawDueTo50MovesRule() const;
    /**
    * @brief Returns the winner of the game.
    * Returns Player color or NoPlayer on draw.
    */
    PlayerColor getWinner() const;

    /**
     * @brief Returns the captured piece from the last turn or
     * Piece(NoPlayer, NoType) if no piece was captured
     */
    Piece getLastCapturedPiece() const;

    bool operator==(const ChessBoard& other) const;
    bool operator!=(const ChessBoard& other) const;
    std::string toString() const;

protected:
    /**
     * @brief We use bit boards for internal turn generation. At least
     * twelve bit boards are needed for complete board representation + some
     * additional helper boards.
     */
    std::array<std::array<BitBoard,NUM_PIECETYPES+1>, NUM_PLAYERS> m_bb;
    //! Updates the helper bit boards.
    void updateBitBoards();

    //! Set or unset the kingInCheck-Flag.
    void setKingInCheck(PlayerColor player, bool kingInCheck);
    //! Set the stalemate-Flag
    void setStalemate();
    //! Set the checkmate-Flag
    void setCheckmate(PlayerColor player);

private:
    //! Init the bit boards from the given chess board in array presentation.
    void initBitBoards(std::array<Piece, 64> board);

    //! Applies a "simple" move turn.
    void applyMoveTurn(const Turn& turn);
    //! Performs a long/short castle turn
    void applyCastleTurn(const Turn& turn);
    //! Promotes a pawn to a given piece type (Queen | Bishop | Rook | Knight).
    void applyPromotionTurn(const Turn& turn, const PieceType pieceType);

    //! Determines the type of a captured piece and takes it from the board.
    void capturePiece(const Turn& turn);
    //! Takes a Piece from the board and adds it to the captured piece list.
    void addCapturedPiece(const Piece capturedPiece, Field field);
    //! Resets the enPassantSquare or sets it to the possible field.
    void updateEnPassantSquare(const Turn& turn);
    //! Checks whether the given turn affects castling rights and updates them accordingly.
    void updateCastlingRights(const Turn& turn);

    //! King of player in check postion.
    std::array<bool, NUM_PLAYERS> m_kingInCheck;
    //! King of player is checkmate.
    std::array<bool, NUM_PLAYERS> m_checkmate;
    //! Game is stalemate.
    bool m_stalemate;

    //! Short castle rights for players.
    std::array<bool, NUM_PLAYERS> m_shortCastleRight;
    //! Long castle rights for players.
    std::array<bool, NUM_PLAYERS> m_longCastleRight;
    //! En passant square
    Field m_enPassantSquare;

    //! Half-move clock
    int m_halfMoveClock;
    //! Full move clock
    int m_fullMoveClock;

    //! Player doing the next turn
    PlayerColor m_nextPlayer;
    //! Capured piece from last turn
    Piece m_lastCapturedPiece;

    IncrementalMaterialAndPSTEvaluator m_evaluator;
    IncrementalZobristHasher m_hasher;
};

/* for debug purposes only */
#define BB_SET( field) static_cast<BitBoard>(std::pow(2, (int)field)) /* returns the value 2^field */

struct PoF {
    Piece piece;
    Field field;

    PoF(Piece piece, Field field)
        : piece(piece), field(field) {}
};
ChessBoard generateChessBoard(std::vector<PoF> pieces, PlayerColor nextPlayer = White);
std::string bitBoardToString(BitBoard b);
BitBoard generateBitBoard(Field f1, ...);

#endif // CHESSBOARD_H
