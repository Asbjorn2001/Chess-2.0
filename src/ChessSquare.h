#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <variant>

class ChessSquare {
   public:
    constexpr ChessSquare(size_t index = 0) : m_index(index) {
        if (index > 63) {
            throw std::out_of_range(
                "Index initialized chess square must be 0-63");
        }
    }

    constexpr ChessSquare(int row, int col) {
        if (row < 0 || row > 7 || col < 0 || col > 7) {
            throw std::out_of_range(
                "Row/col initialized chess square must be 0-7");
        }
        m_index = static_cast<size_t>(row * 8 + col);
    }

    constexpr ChessSquare(char file, char rank) {
        int col = file - 'a';
        if (col < 0 || col > 7) {
            throw std::out_of_range(
                "Char initialized chess file must be \'a-h\'");
        }

        int row = rank - '1';
        if (row < 0 || row > 7) {
            throw std::out_of_range(
                "Char initialized chess rank must be \'1-8\'");
        }

        m_index = static_cast<size_t>(row * 8 + col);
    }

    constexpr int row() const { return static_cast<int>(m_index / 8); }
    constexpr int col() const { return static_cast<int>(m_index % 8); }
    constexpr size_t index() const { return m_index; }
    constexpr size_t offset_with(int offset) const {
        return static_cast<size_t>(static_cast<int>(m_index) + offset);
    }

    constexpr operator size_t() const { return m_index; }

    constexpr char rank() const { return static_cast<char>(row()) + '1'; }
    constexpr char file() const { return static_cast<char>(col()) + 'a'; }

    friend std::ostream& operator<<(std::ostream& os,
                                    const ChessSquare& square) {
        return os << square.file() << square.rank();
    }

   private:
    size_t m_index;
};

enum class ChessColor {
    White,
    Black,
    Neutral,
};

enum class OccupantType {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None,
};

class SquareOccupant {
   public:
    constexpr SquareOccupant() : m_value(' ') {};
    constexpr SquareOccupant(char c) : m_value(c) {};

    constexpr OccupantType type() const {
        switch (m_value) {
            case 'p':
            case 'P':
                return OccupantType::Pawn;
            case 'n':
            case 'N':
                return OccupantType::Knight;
            case 'b':
            case 'B':
                return OccupantType::Bishop;
            case 'r':
            case 'R':
                return OccupantType::Rook;
            case 'q':
            case 'Q':
                return OccupantType::Queen;
            case 'k':
            case 'K':
                return OccupantType::King;
            default:
                return OccupantType::None;
        }
    }

    constexpr ChessColor piece_color() const {
        if (m_value >= 'A' && m_value <= 'Z')
            return ChessColor::White;
        if (m_value >= 'a' && m_value <= 'z')
            return ChessColor::Black;
        return ChessColor::Neutral;
    }

    constexpr explicit operator char() const { return m_value; }

    constexpr bool is_empty() const { return m_value == ' '; }
    constexpr bool is_piece() const { return m_value != ' '; }

    constexpr bool is_sliding_piece() const {
        switch (type()) {
            case OccupantType::Bishop:
            case OccupantType::Rook:
            case OccupantType::Queen:
                return true;
            default:
                return false;
        }
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    const SquareOccupant& sq) {
        return os << sq.m_value;
    }

   private:
    char m_value;
};

struct SqOccPair {
    ChessSquare square;
    SquareOccupant occ;

    bool operator<(const SqOccPair& rhs) const {
        return this->square < rhs.square;
    }
};

struct CastleMove {
    ChessSquare rook_start;
    ChessSquare rook_target;
    ChessSquare king_target;
};

struct ChessMove {
    ChessSquare start_square;
    ChessSquare target_square;
    std::optional<SqOccPair> capture = std::nullopt;
    std::optional<SquareOccupant> promotion = std::nullopt;
    std::optional<ChessSquare> en_passant_target = std::nullopt;
    std::optional<CastleMove> castle_move = std::nullopt;

    friend std::ostream& operator<<(std::ostream& os, const ChessMove& move) {
        return os << move.start_square << "->" << move.target_square;
    }
};
