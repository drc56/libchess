#include "libchess/piece.hpp"

namespace libchess
{
    std::string piecetostring(const Piece& p){
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
} // namespace libchess
