#ifndef LIBCHESS_PIECE_HPP
#define LIBCHESS_PIECE_HPP

#include <array>
#include <string>

namespace libchess {

enum Piece : int
{
    Pawn = 0,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None,
};

inline constexpr std::array<Piece, 6> pieces = {{
    Piece::Pawn,
    Piece::Knight,
    Piece::Bishop,
    Piece::Rook,
    Piece::Queen,
    Piece::King,
}};

std::string piecetostring(const Piece& p);

}  // namespace libchess

#endif
