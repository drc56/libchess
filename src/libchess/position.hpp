#ifndef LIBCHESS_POSITION_HPP
#define LIBCHESS_POSITION_HPP

#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "side.hpp"
#include "zobrist.hpp"

namespace libchess {

namespace {

enum Castling : int
{
    usKSC,
    usQSC,
    themKSC,
    themQSC
};

constexpr const Square ksc_rook_fr[] = {squares::H1, squares::H8};
constexpr const Square qsc_rook_fr[] = {squares::A1, squares::A8};
constexpr const Square ksc_rook_to[] = {squares::F1, squares::F8};
constexpr const Square qsc_rook_to[] = {squares::D1, squares::D8};

}  // namespace

class Position {
   public:
    [[nodiscard]] Position() = default;

    [[nodiscard]] explicit Position(const std::string &fen) {
        set_fen(fen);
    }

    [[nodiscard]] constexpr Side turn() const noexcept {
        return to_move_;
    }

    [[nodiscard]] constexpr Bitboard occupancy(const Side s) const noexcept {
        return colours_[s];
    }

    [[nodiscard]] constexpr Bitboard occupancy(const Piece p) const noexcept {
        return pieces_[p];
    }

    [[nodiscard]] constexpr Bitboard pieces(const Side s, const Piece p) const noexcept {
        return occupancy(s) & occupancy(p);
    }

    [[nodiscard]] constexpr Bitboard occupied() const noexcept {
        return occupancy(Side::White) | occupancy(Side::Black);
    }

    [[nodiscard]] constexpr Bitboard empty() const noexcept {
        return ~occupied();
    }

    [[nodiscard]] constexpr std::uint64_t hash() const noexcept {
        return hash_;
    }

    void set_fen(const std::string &fen) noexcept;

    [[nodiscard]] std::string get_fen() const noexcept;

    [[nodiscard]] bool is_legal(const Move &m) const noexcept;

    [[nodiscard]] bool is_terminal() const noexcept {
        return legal_moves().empty() || is_draw();
    }

    [[nodiscard]] bool is_checkmate() const noexcept {
        return legal_moves().empty() && in_check();
    }

    [[nodiscard]] bool is_stalemate() const noexcept {
        return legal_moves().empty() && !in_check();
    }
    
    [[nodiscard]] bool is_draw() const noexcept {
        return (threefold() || fiftymoves()) && !is_checkmate();
    }
    
    [[nodiscard]] bool threefold() const noexcept {
        if (halfmove_clock_ < 8) {
            return false;
        }

        int repeats = 0;
        for (std::size_t i = 2; i <= history_.size() && i <= halfmoves(); i += 2) {
            if (history_[history_.size() - i].hash == hash_) {
                repeats++;
                if (repeats >= 2) {
                    return true;
                }
            }
        }
        return false;
    }

    [[nodiscard]] constexpr bool fiftymoves() const noexcept {
        return halfmove_clock_ >= 100;
    }

    [[nodiscard]] constexpr std::size_t halfmoves() const noexcept {
        return halfmove_clock_;
    }

    [[nodiscard]] constexpr std::size_t fullmoves() const noexcept {
        return fullmove_clock_;
    }

    [[nodiscard]] constexpr Square king_position(const Side s) const noexcept {
        return pieces(s, Piece::King).lsb();
    }

    [[nodiscard]] bool square_attacked(const Square sq, const Side s) const noexcept;

    [[nodiscard]] Bitboard squares_attacked(const Side s) const noexcept;

    [[nodiscard]] Bitboard checkers() const noexcept;

    [[nodiscard]] Bitboard attackers(const Square sq, const Side s) const noexcept;

    [[nodiscard]] bool in_check() const noexcept {
        return square_attacked(king_position(turn()), !turn());
    }

    [[nodiscard]] Bitboard king_allowed() const noexcept;

    [[nodiscard]] Bitboard king_allowed(const Side s) const noexcept;

    [[nodiscard]] Bitboard pinned() const noexcept;

    [[nodiscard]] Bitboard pinned(const Side s) const noexcept;

    [[nodiscard]] Bitboard pinned(const Side s, const Square sq) const noexcept;

    [[nodiscard]] std::vector<Move> check_evasions() const noexcept;

    [[nodiscard]] std::vector<Move> legal_moves() const noexcept;

    [[nodiscard]] std::vector<Move> legal_captures() const noexcept;

    [[nodiscard]] std::vector<Move> legal_noncaptures() const noexcept;

    void legal_captures(std::vector<Move> &moves) const noexcept;

    void legal_noncaptures(std::vector<Move> &moves) const noexcept;

    [[nodiscard]] constexpr Bitboard passed_pawns() const noexcept {
        return passed_pawns(turn());
    }

    [[nodiscard]] constexpr Bitboard passed_pawns(const Side s) const noexcept {
        auto mask = pieces(!s, Piece::Pawn);
        if (s == Side::White) {
            mask |= mask.south().east();
            mask |= mask.south().west();
            mask |= mask.south();
            mask |= mask.south();
            mask |= mask.south();
            mask |= mask.south();
            mask |= mask.south();
        } else {
            mask |= mask.north().east();
            mask |= mask.north().west();
            mask |= mask.north();
            mask |= mask.north();
            mask |= mask.north();
            mask |= mask.north();
            mask |= mask.north();
        }
        return pieces(s, Piece::Pawn) & ~mask;
    }

    [[nodiscard]] std::size_t count_moves() const noexcept;

    [[nodiscard]] std::uint64_t perft(const int depth) noexcept;

    [[nodiscard]] constexpr bool can_castle(const Side s, const MoveType mt) const noexcept {
        if (s == Side::White) {
            if (mt == MoveType::ksc) {
                return castling_[0];
            } else {
                return castling_[1];
            }
        } else {
            if (mt == MoveType::ksc) {
                return castling_[2];
            } else {
                return castling_[3];
            }
        }
        return true;
    }

    [[nodiscard]] std::uint64_t predict_hash(const Move &move) const noexcept;

    [[nodiscard]] Move parse_move(const std::string &str) const {
        const auto moves = legal_moves();
        for (const auto &move : moves) {
            if (static_cast<std::string>(move) == str) {
                return move;
            }
        }

        throw std::invalid_argument("Illegal move string");
    }

    [[nodiscard]] std::string parse_move_to_san(const Move& move) const noexcept{
        std::string san_string;
        if(move.type() == MoveType::ksc){
            return "O-O";
        }
        if(move.type() == MoveType::qsc){
            return "O-O-O";
        }
        
        if(move.piece() != Piece::Pawn)
        {
            san_string += piecetostring(move.piece());
        }
        // Need to the clarity if another piece of that type could make the move..

        if(move.type() == MoveType::Capture || move.type() == MoveType::promo_capture || move.type() == MoveType::enpassant){
            if(move.piece() == Piece::Pawn){
                san_string += static_cast<std::string>(move.from())[0];
            }
            san_string += "x";
        }

        san_string += static_cast<std::string>(move.to());
        if (move.promotion() != Piece::None) {
            const char asd[] = {'n', 'b', 'r', 'q'};
            san_string += asd[move.promotion() - 1];
        }

        // Add the check testing..
        return san_string;
    }

    void makemove(const Move &move) noexcept;

    void makemove(const std::string &str) {
        const auto move = parse_move(str);
        makemove(move);
    }

    void undomove() noexcept;

    void makenull() noexcept {
        history_.push_back(meh{
            hash(),
            {},
            ep_,
            halfmoves(),
            {},
        });

#ifndef NO_HASH
        if (ep_ != squares::OffSq) {
            hash_ ^= zobrist::ep_key(ep_);
        }
        hash_ ^= zobrist::turn_key();
#endif

        to_move_ = !to_move_;
        ep_ = squares::OffSq;
        halfmove_clock_ = 0;
    }

    void undonull() noexcept {
        hash_ = history_.back().hash;
        ep_ = history_.back().ep;
        halfmove_clock_ = history_.back().halfmove_clock;
        to_move_ = !to_move_;
        history_.pop_back();
    }

    [[nodiscard]] constexpr std::uint64_t calculate_hash() const noexcept {
        std::uint64_t hash = 0;

        // Turn
        if (turn() == Side::Black) {
            hash ^= zobrist::turn_key();
        }

        // Pieces
        for (const auto s : {Side::White, Side::Black}) {
            for (const auto &sq : pieces(s, Piece::Pawn)) {
                hash ^= zobrist::piece_key(Piece::Pawn, s, sq);
            }
            for (const auto &sq : pieces(s, Piece::Knight)) {
                hash ^= zobrist::piece_key(Piece::Knight, s, sq);
            }
            for (const auto &sq : pieces(s, Piece::Bishop)) {
                hash ^= zobrist::piece_key(Piece::Bishop, s, sq);
            }
            for (const auto &sq : pieces(s, Piece::Rook)) {
                hash ^= zobrist::piece_key(Piece::Rook, s, sq);
            }
            for (const auto &sq : pieces(s, Piece::Queen)) {
                hash ^= zobrist::piece_key(Piece::Queen, s, sq);
            }
            for (const auto &sq : pieces(s, Piece::King)) {
                hash ^= zobrist::piece_key(Piece::King, s, sq);
            }
        }

        // Castling
        if (can_castle(Side::White, MoveType::ksc)) {
            hash ^= zobrist::castling_key(usKSC);
        }
        if (can_castle(Side::White, MoveType::qsc)) {
            hash ^= zobrist::castling_key(usQSC);
        }
        if (can_castle(Side::Black, MoveType::ksc)) {
            hash ^= zobrist::castling_key(themKSC);
        }
        if (can_castle(Side::Black, MoveType::qsc)) {
            hash ^= zobrist::castling_key(themQSC);
        }

        // EP
        if (ep_ != squares::OffSq) {
            hash ^= zobrist::ep_key(ep_);
        }

        return hash;
    }

    [[nodiscard]] auto &history() const noexcept {
        return history_;
    }

    [[nodiscard]] constexpr Piece piece_on(const Square sq) const noexcept {
        for (int i = 0; i < 6; ++i) {
            if (pieces_[i] & Bitboard{sq}) {
                return Piece(i);
            }
        }
        return Piece::None;
    }

    [[nodiscard]] constexpr Square ep() const noexcept {
        return ep_;
    }

    void clear() noexcept {
        colours_[0].clear();
        colours_[1].clear();
        pieces_[0].clear();
        pieces_[1].clear();
        pieces_[2].clear();
        pieces_[3].clear();
        pieces_[4].clear();
        pieces_[5].clear();
        halfmove_clock_ = 0;
        fullmove_clock_ = 0;
        ep_ = squares::OffSq;
        hash_ = 0x0;
        castling_[0] = false;
        castling_[1] = false;
        castling_[2] = false;
        castling_[3] = false;
        to_move_ = Side::White;
        history_.clear();
    }

    [[nodiscard]] bool valid() const noexcept;

   private:
    void set(const Square sq, const Side s, const Piece p) noexcept {
        colours_[s] |= sq;
        pieces_[p] |= sq;
    }

    struct meh {
        std::uint64_t hash = 0;
        Move move;
        Square ep;
        std::size_t halfmove_clock = 0;
        bool castling[4] = {};
    };

    Bitboard colours_[2] = {};
    Bitboard pieces_[6] = {};
    std::size_t halfmove_clock_ = 0;
    std::size_t fullmove_clock_ = 0;
    Square ep_ = squares::OffSq;
    std::uint64_t hash_ = 0;
    bool castling_[4] = {};
    Side to_move_ = Side::White;
    std::vector<meh> history_;
};

inline std::ostream &operator<<(std::ostream &os, const Position &pos) noexcept {
    int i = 56;
    while (i >= 0) {
        const auto sq = Square(i);
        const auto bb = Bitboard{sq};
        if (pos.pieces(Side::White, Piece::Pawn) & bb) {
            os << 'P';
        } else if (pos.pieces(Side::White, Piece::Knight) & bb) {
            os << 'N';
        } else if (pos.pieces(Side::White, Piece::Bishop) & bb) {
            os << 'B';
        } else if (pos.pieces(Side::White, Piece::Rook) & bb) {
            os << 'R';
        } else if (pos.pieces(Side::White, Piece::Queen) & bb) {
            os << 'Q';
        } else if (pos.pieces(Side::White, Piece::King) & bb) {
            os << 'K';
        } else if (pos.pieces(Side::Black, Piece::Pawn) & bb) {
            os << 'p';
        } else if (pos.pieces(Side::Black, Piece::Knight) & bb) {
            os << 'n';
        } else if (pos.pieces(Side::Black, Piece::Bishop) & bb) {
            os << 'b';
        } else if (pos.pieces(Side::Black, Piece::Rook) & bb) {
            os << 'r';
        } else if (pos.pieces(Side::Black, Piece::Queen) & bb) {
            os << 'q';
        } else if (pos.pieces(Side::Black, Piece::King) & bb) {
            os << 'k';
        } else {
            os << '-';
        }

        if (i % 8 == 7) {
            os << '\n';
            i -= 16;
        }

        i++;
    }

    os << "Castling: ";
    os << (pos.can_castle(Side::White, MoveType::ksc) ? "K" : "");
    os << (pos.can_castle(Side::White, MoveType::qsc) ? "Q" : "");
    os << (pos.can_castle(Side::Black, MoveType::ksc) ? "k" : "");
    os << (pos.can_castle(Side::Black, MoveType::qsc) ? "q" : "");
    os << '\n';
    if (pos.ep() == squares::OffSq) {
        os << "EP: -\n";
    } else {
        os << "EP: " << pos.ep() << '\n';
    }
    os << "Turn: " << (pos.turn() == Side::White ? 'w' : 'b');

    return os;
}

}  // namespace libchess

#endif
