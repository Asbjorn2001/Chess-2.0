#include <SDL2/SDL_events.h>
#include <algorithm>
#include <optional>
#include <ranges>
#include "ChessController.h"

ChessController::ChessController(ChessModel& model) : m_model(model) {}

void ChessController::handle_event(SDL_Event event) {
    const auto square{get_square_from(event.motion.x, event.motion.y)};
    const auto& selected{m_model.selected_square};
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            if (!square.has_value()) {
                return;
            }
            if (selected.has_value()) {
                if (try_move(selected.value(), square.value())) {
                }
            }
            try_select(square.value());
        case SDL_MOUSEBUTTONUP:
            break;
        case SDL_MOUSEMOTION:
            break;
        default:
            break;
    }
}

std::optional<ChessSquare> ChessController::get_square_from(int mouse_x,
                                                            int mouse_y) {
    const auto sq_size{m_model.settings.square_size()};
    const auto row{mouse_y / sq_size};
    const auto col{mouse_x / sq_size};

    if (row < 0 || row > 7 || col < 0 || col > 7) {
        return std::nullopt;
    }
    return ChessSquare{7 - row, col};
}

bool ChessController::try_select(ChessSquare square) {
    const auto occ{m_model.board[square]};
    if (occ.is_piece() && occ.piece_color() == m_model.board.active_color()) {
        m_model.selected_square = square;
        return true;
    } else {
        m_model.selected_square = std::nullopt;
        return false;
    }
}

bool ChessController::try_move(ChessSquare start_square,
                               ChessSquare target_square) {
    for (const auto& move : m_model.get_legal_moves_at(start_square)) {
        if (move.target_square == target_square) {
            make_move(move);
            m_model.selected_square = std::nullopt;
            return true;
        }
    }

    return false;
}

void ChessController::make_move(const ChessMove& move) {
    auto& board = m_model.board;
    board.move(move);
    m_model.position_data = m_move_generator.generate_position_data(board);
    if (m_model.position_data.legal_moves.empty()) {
    }
}

ChessModel::ChessModel(ChessBoard board, Settings settings)
    : board(board), settings(settings) {
    MoveGenerator generator{};
    position_data = generator.generate_position_data(board);
    selected_square = std::nullopt;
}

std::vector<ChessMove> ChessModel::get_legal_moves_at(
    ChessSquare square) const {
    auto it = position_data.legal_moves.find(square);
    if (it == position_data.legal_moves.end()) {
        return {};
    }
    return it->second;
}
