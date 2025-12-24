#include "ChessBoard.h"
#include "utils.h"

CastleRights::CastleRights(std::string fen_castle_str) {
    w_k_side = fen_castle_str.contains('K');
    w_q_side = fen_castle_str.contains('Q');
    b_k_side = fen_castle_str.contains('k');
    b_q_side = fen_castle_str.contains('q');
}

std::string CastleRights::as_fen_string() const {
    std::string res {};
    if (!w_k_side && !w_q_side) {
        res.push_back('-');
    } else {
        if (w_k_side) res.push_back('K');
        if (w_q_side) res.push_back('Q');
    }
    if (!b_k_side && !b_q_side) {
        res.push_back('-');
    } else {
        if (b_k_side) res.push_back('k');
        if (b_q_side) res.push_back('q');
    }

    return res;
}

ChessBoard::ChessBoard(const std::string& fen_string) {
    try {
        auto tokens = split_string(fen_string, " ");
        if (tokens.size() < 5 || tokens.size() > 6) {
            throw std::logic_error(std::format("Expected 5 or 6 arguments, but got {}", tokens.size()));
        }
        size_t index = 0;
        auto ranks = split_string(tokens.at(0), "/");
        if (ranks.size() != 8) {
            throw std::logic_error(std::format("Expected 8 ranks, but got {}", ranks.size()));
        }
        for (auto it = ranks.rbegin(); it != ranks.rend(); ++it) {
            for (const auto& c : *it) {
                if (c >= '1' && c <= '8') {
                    size_t stop_index = index + static_cast<size_t>(c - '0');
                    while (index < stop_index) {
                        m_position.at(index++) = ' ';
                    }
                } else {
                    m_position.at(index++) = c;
                }
            }
        }

        auto c_color = tokens.at(1).at(0);
        if (c_color != 'w' && c_color != 'b') {
            throw std::logic_error(std::format("Expected color \'w\' or \'b\', but got: \'{}\'", c_color));
        }
        m_active_color = c_color == 'w' ? ChessColor::White : ChessColor::Black;
        m_castle_rights = CastleRights(tokens.at(2));

        if (tokens.size() == 6) {
            auto en_passant_str = tokens.at(3);
            m_en_passant_target = { en_passant_str.at(0), en_passant_str.at(1) }; 
        } else {
            m_en_passant_target = {};
        }

        m_half_move_clock = static_cast<size_t>(std::stoi(tokens.at(tokens.size() - 2)));
        m_full_move_count = static_cast<size_t>(std::stoi(tokens.at(tokens.size() - 1))); 
    } catch (std::logic_error e) {
        std::cerr << e.what() << "\n";
        throw std::logic_error("Something went wrong while parsing the FEN string: " + fen_string);
    } 
}

char color_to_char(ChessColor color) {
    switch (color) {
        case ChessColor::White: return 'w';
        case ChessColor::Black: return 'b';
        case ChessColor::Neutral: return 'n';
    }
}

std::string ChessBoard::as_fen_string() const {
    std::string fen_string;
    auto board = *this;
    for (auto it = ranks.rbegin(); it != ranks.rend(); ++it) {
        char rank = *it;
        size_t empty_square_counter = 0;
        for (auto file : files) {
            SquareOccupant p = board[{ file, rank }];
            if (p.is_empty()) {
                ++empty_square_counter;
            } else {
                if (empty_square_counter > 0) {
                    fen_string.append(std::to_string(empty_square_counter));
                    empty_square_counter = 0;
                }
                fen_string.push_back(static_cast<char>(p));
            }
        }
        if (empty_square_counter > 0) {
            fen_string.append(std::to_string(empty_square_counter));
        }
        if (it + 1 != ranks.rend()) {
            fen_string.push_back('/');
        }
    }
    
    fen_string.push_back(' ');
    fen_string.push_back(color_to_char(m_active_color));

    fen_string.push_back(' ');
    fen_string.append(m_castle_rights.as_fen_string());
    
    if (m_en_passant_target) {
        fen_string.push_back(' ');
        fen_string.push_back(m_en_passant_target->file());
        fen_string.push_back(m_en_passant_target->rank());
    }

    fen_string.push_back(' ');
    fen_string.append(std::to_string(m_half_move_clock));

    fen_string.push_back(' ');
    fen_string.append(std::to_string(m_full_move_count));

    return fen_string;
}

const SquareOccupant& ChessBoard::operator[](ChessSquare square) const {
    return this->m_position[square.index()];
}

std::vector<std::pair<ChessSquare, SquareOccupant>> ChessBoard::get_pieces(ChessColor piece_color) const {
    std::vector<std::pair<ChessSquare, SquareOccupant>> pieces;
    for (size_t i = 0; i < m_position.size(); ++i) {
        const SquareOccupant occ = m_position[i];
        if (occ.is_piece()) {
            const ChessSquare square { i };
            if (piece_color == ChessColor::Neutral) {
                pieces.push_back({ square, occ });
            } else if (occ.piece_color() == piece_color) {
                pieces.push_back({ square, occ });
            }
        }
    }

    return pieces;
}

void ChessBoard::move(ChessMove move) {
    if (move.capture.has_value()) {
        m_position[move.capture->first] = ' ';
    }
    m_position[move.target_square] = m_position[move.start_square];
    m_position[move.start_square] = ' ';

    m_active_color = m_active_color == ChessColor::White ? 
        ChessColor::Black : ChessColor::White;

    m_en_passant_target = move.en_passant_target;
    m_history.push_back(as_fen_string());
}

std::ostream& operator<<(std::ostream& os, const ChessBoard& board) {
    for (auto it = ranks.rbegin(); it != ranks.rend(); ++it) {
        char rank = *it;
        os << rank << "|";
        for (auto file : files) {
            os << board[{ file, rank }] << " ";
        }
        os << "\n";
    }
    os << " |---------------\n  a b c d e f g h";
    return os;
}
