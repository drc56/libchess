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

[[nodiscard]] std::string piecetostring(const Piece& p){
    switch (p){
        case Piece::Pawn:
            return "P";
        case Piece::Knight:
            return "N";
        case Piece::Bishop:
            return "B";
        case Piece::Rook:
            return "R"; 
        case Piece::Queen:
            return "Q";
        case Piece::King:
            return "K";
        case Piece::None:
            return "";
        default:
            return "";
    }

}

}  // namespace libchess

#endif
