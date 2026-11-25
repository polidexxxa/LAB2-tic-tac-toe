#ifndef _MAIN_WINDOW_HPP_
#define _MAIN_WINDOW_HPP_

#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

#include "game/Board.hpp"
#include "game/MinimaxEngine.hpp"
#include "common/Option.hpp"

// Интерактивная доска.
class GameCanvas : public wxPanel {
public:
    GameCanvas(
        wxWindow* parent,
        Board* board,
        MinimaxEngine* ai,
        wxStatusBar* statusBar
    )
        : wxPanel(
              parent,
              wxID_ANY,
              wxDefaultPosition,
              wxDefaultSize,
              wxFULL_REPAINT_ON_RESIZE
          ),
          m_board(board),
          m_ai(ai),
          m_statusBar(statusBar) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);

        Bind(wxEVT_PAINT, &GameCanvas::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &GameCanvas::OnLeftDown, this);
        Bind(wxEVT_RIGHT_DOWN, &GameCanvas::OnRightDown, this);
        Bind(wxEVT_RIGHT_UP, &GameCanvas::OnRightUp, this);
        Bind(wxEVT_MOTION, &GameCanvas::OnMouseMove, this);
        Bind(wxEVT_MOUSEWHEEL, &GameCanvas::OnMouseWheel, this);
        Bind(
            wxEVT_MOUSE_CAPTURE_LOST,
            &GameCanvas::OnMouseCaptureLost,
            this
        );

        UpdateStatus(
            "Your turn (X). Drag with RMB to pan, Scroll to zoom."
        );
    }

    void ResetGame() {
        if (HasCapture()) {
            ReleaseMouse();
        }

        m_isDragging = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));

        *m_board = Board();

        m_gameOver = false;

        m_lastMove = Option<Point>();

        m_hasWinningLine = false;

        m_offsetX = 0.0;
        m_offsetY = 0.0;
        m_zoom = 1.0;

        UpdateStatus("New game started. Your turn (X).");
        Refresh();
    }

private:
    Board* m_board;
    MinimaxEngine* m_ai;
    wxStatusBar* m_statusBar;

    double m_offsetX = 0.0;
    double m_offsetY = 0.0;
    double m_zoom = 1.0;

    const double m_baseCellSize = 44.0;

    bool m_isDragging = false;
    wxPoint m_dragStartMouse;

    double m_dragStartOffsetX = 0.0;
    double m_dragStartOffsetY = 0.0;

    bool m_gameOver = false;

    Option<Point> m_lastMove;

    // Концы выигрышной пятерки в координатах доски.
    bool m_hasWinningLine = false;
    Point m_winningStart;
    Point m_winningEnd;

    void UpdateStatus(const wxString& message) {
        if (m_statusBar) {
            m_statusBar->SetStatusText(message);
        }
    }

    Point ScreenToBoard(int sx, int sy) const {
        wxSize size = GetClientSize();

        double cx = size.GetWidth() / 2.0 + m_offsetX;
        double cy = size.GetHeight() / 2.0 + m_offsetY;

        double currentCellSize = m_baseCellSize * m_zoom;

        int bx = static_cast<int>(
            std::floor((sx - cx) / currentCellSize)
        );

        int by = static_cast<int>(
            std::floor((sy - cy) / currentCellSize)
        );

        return Point(bx, by);
    }

    void SaveWinningLine(const Point& lastMove, Cell player) {
        m_hasWinningLine = false;

        if (player == Cell::Empty ||
            m_board->GetCell(lastMove) != player) {
            return;
        }

        const Point directions[] = {
            Point(1, 0),
            Point(0, 1),
            Point(1, 1),
            Point(1, -1)
        };

        for (const Point& direction : directions) {
            int backward = 0;
            int forward = 0;

            // Непрерывная цепочка перед последним ходом.
            for (int step = 1; step <= 4; ++step) {
                Point point(
                    lastMove.x - step * direction.x,
                    lastMove.y - step * direction.y
                );

                if (m_board->GetCell(point) != player) {
                    break;
                }

                ++backward;
            }

            // Непрерывная цепочка после последнего хода.
            for (int step = 1; step <= 4; ++step) {
                Point point(
                    lastMove.x + step * direction.x,
                    lastMove.y + step * direction.y
                );

                if (m_board->GetCell(point) != player) {
                    break;
                }

                ++forward;
            }

            if (backward + 1 + forward >= 5) {
                m_winningStart = Point(
                    lastMove.x - backward * direction.x,
                    lastMove.y - backward * direction.y
                );

                m_winningEnd = Point(
                    m_winningStart.x + 4 * direction.x,
                    m_winningStart.y + 4 * direction.y
                );

                m_hasWinningLine = true;
                return;
            }
        }
    }

    void OnPaint(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();

        std::unique_ptr<wxGraphicsContext> gc(
            wxGraphicsContext::Create(dc)
        );

        if (!gc) {
            return;
        }

        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

        wxSize size = GetClientSize();

        int width = size.GetWidth();
        int height = size.GetHeight();

        // Фон.
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(wxColour(248, 249, 250)));
        gc->DrawRectangle(0, 0, width, height);

        double cx = width / 2.0 + m_offsetX;
        double cy = height / 2.0 + m_offsetY;
        double currentCell = m_baseCellSize * m_zoom;

        // Границы видимой части поля.
        int minX = static_cast<int>(
            std::floor(-cx / currentCell)
        ) - 1;

        int maxX = static_cast<int>(
            std::ceil((width - cx) / currentCell)
        ) + 1;

        int minY = static_cast<int>(
            std::floor(-cy / currentCell)
        ) - 1;

        int maxY = static_cast<int>(
            std::ceil((height - cy) / currentCell)
        ) + 1;

        // Сетка.
        gc->SetPen(wxPen(wxColour(226, 232, 240), 1));

        for (int x = minX; x <= maxX; ++x) {
            double px = cx + x * currentCell;
            gc->StrokeLine(px, 0, px, height);
        }

        for (int y = minY; y <= maxY; ++y) {
            double py = cy + y * currentCell;
            gc->StrokeLine(0, py, width, py);
        }

        // Подсветка последнего хода.
        if (m_lastMove.IsSome()) {
            Point lastMove = m_lastMove.GetValue();

            double lx = cx + lastMove.x * currentCell;
            double ly = cy + lastMove.y * currentCell;

            gc->SetBrush(
                wxBrush(wxColour(254, 240, 138, 140))
            );

            gc->SetPen(
                wxPen(wxColour(234, 179, 8), 1)
            );

            gc->DrawRoundedRectangle(
                lx + 2,
                ly + 2,
                currentCell - 4,
                currentCell - 4,
                4.0
            );
        }

        // Крестики и нолики.
        auto occupied = m_board->GetOccupiedCellsList();
        auto* it = occupied.GetEnumerator();

        while (it->MoveNext()) {
            auto pair = std::as_const(*it).GetCurrent();

            double px = cx + pair.key.x * currentCell;
            double py = cy + pair.key.y * currentCell;
            double padding = currentCell * 0.22;

            if (pair.value == Cell::X) {
                int penWidth = std::max(
                    2,
                    static_cast<int>(
                        std::lround(3.2 * m_zoom)
                    )
                );

                wxPen xPen(
                    wxColour(37, 99, 235),
                    penWidth
                );

                xPen.SetCap(wxCAP_ROUND);
                gc->SetPen(xPen);

                gc->StrokeLine(
                    px + padding,
                    py + padding,
                    px + currentCell - padding,
                    py + currentCell - padding
                );

                gc->StrokeLine(
                    px + currentCell - padding,
                    py + padding,
                    px + padding,
                    py + currentCell - padding
                );
            } else if (pair.value == Cell::O) {
                int penWidth = std::max(
                    2,
                    static_cast<int>(
                        std::lround(3.0 * m_zoom)
                    )
                );

                wxPen oPen(
                    wxColour(220, 38, 38),
                    penWidth
                );

                gc->SetPen(oPen);
                gc->SetBrush(*wxTRANSPARENT_BRUSH);

                double radius = currentCell / 2.0 - padding;

                gc->DrawEllipse(
                    px + currentCell / 2.0 - radius,
                    py + currentCell / 2.0 - radius,
                    radius * 2.0,
                    radius * 2.0
                );
            }
        }

        delete it;

        //Выигрышная линия
        if (m_hasWinningLine) {
            double startX =
                cx + (m_winningStart.x + 0.5) * currentCell;

            double startY =
                cy + (m_winningStart.y + 0.5) * currentCell;

            double endX =
                cx + (m_winningEnd.x + 0.5) * currentCell;

            double endY =
                cy + (m_winningEnd.y + 0.5) * currentCell;

            int penWidth = std::max(
                3,
                static_cast<int>(
                    std::lround(5.0 * m_zoom)
                )
            );

            wxPen winningPen(
                wxColour(22, 163, 74),
                penWidth
            );

            winningPen.SetCap(wxCAP_ROUND);
            gc->SetPen(winningPen);

            gc->StrokeLine(startX, startY, endX, endY);
        }
    }

    void OnLeftDown(wxMouseEvent& event) {
        if (m_gameOver) {
            UpdateStatus(
                "Game over. Start a new game from the Game menu."
            );
            return;
        }

        Point playerMove = ScreenToBoard(
            event.GetX(),
            event.GetY()
        );

        if (!m_board->MakeMove(playerMove, Cell::X)) {
            return;
        }

        m_lastMove = Option<Point>(playerMove);
        Refresh();

        //Победа игрока
        if (m_board->CheckWin(playerMove, Cell::X)) {
            m_gameOver = true;

            SaveWinningLine(playerMove, Cell::X);

            UpdateStatus("Victory! You won (5-in-a-row).");

            Refresh();
            Update();

            wxMessageBox(
                "Congratulations! You have won the game!",
                "Game Over",
                wxOK | wxICON_INFORMATION,
                this
            );

            return;
        }

        //Ход игрока перед
        UpdateStatus("AI is calculating the best move...");
        Update();

        SearchResult aiResult = m_ai->FindBestMove(
            *m_board,
            Cell::O,
            2
        );

        if (!m_board->MakeMove(aiResult.best_move, Cell::O)) {
            UpdateStatus("AI returned an invalid move.");
            return;
        }

        m_lastMove = Option<Point>(aiResult.best_move);
        Refresh();

        // Победа ИИ.
        if (m_board->CheckWin(aiResult.best_move, Cell::O)) {
            m_gameOver = true;

            SaveWinningLine(aiResult.best_move, Cell::O);

            UpdateStatus("AI won! Better luck next time.");

            Refresh();
            Update();

            wxMessageBox(
                "AI has won the match.",
                "Game Over",
                wxOK | wxICON_EXCLAMATION,
                this
            );
        } else {
            UpdateStatus("Your turn (X).");
        }
    }

    void OnRightDown(wxMouseEvent& event) {
        if (m_isDragging) {
            return;
        }

        m_isDragging = true;
        m_dragStartMouse = event.GetPosition();

        m_dragStartOffsetX = m_offsetX;
        m_dragStartOffsetY = m_offsetY;

        if (!HasCapture()) {
            CaptureMouse();
        }

        SetCursor(wxCursor(wxCURSOR_SIZING));
    }

    void OnRightUp(wxMouseEvent&) {
        if (!m_isDragging) {
            return;
        }

        m_isDragging = false;

        if (HasCapture()) {
            ReleaseMouse();
        }

        SetCursor(wxCursor(wxCURSOR_ARROW));
    }

    void OnMouseCaptureLost(wxMouseCaptureLostEvent&) {
        m_isDragging = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }

    void OnMouseMove(wxMouseEvent& event) {
        if (!m_isDragging) {
            return;
        }

        wxPoint delta =
            event.GetPosition() - m_dragStartMouse;

        m_offsetX = m_dragStartOffsetX + delta.x;
        m_offsetY = m_dragStartOffsetY + delta.y;

        Refresh();
    }

    void OnMouseWheel(wxMouseEvent& event) {
        int rotation = event.GetWheelRotation();

        if (rotation == 0) {
            return;
        }

        double factor =
            rotation > 0 ? 1.12 : (1.0 / 1.12);

        double newZoom = m_zoom * factor;

        if (newZoom >= 0.35 && newZoom <= 3.5) {
            m_zoom = newZoom;
            Refresh();
        }
    }
};

// Главное окно приложения.
class MainFrame : public wxFrame {
public:
    MainFrame()
        : wxFrame(
              nullptr,
              wxID_ANY,
              "Infinite Tic-Tac-Toe",
              wxDefaultPosition,
              wxSize(960, 720)
          ) {
        CreateStatusBar(1);

        m_canvas = new GameCanvas(
            this,
            &m_board,
            &m_ai,
            GetStatusBar()
        );

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_canvas, 1, wxEXPAND);
        SetSizer(sizer);

        // Меню игры.
        wxMenu* gameMenu = new wxMenu();

        gameMenu->Append(
            wxID_NEW,
            "&New Game\tCtrl+N",
            "Start a fresh game"
        );

        gameMenu->AppendSeparator();

        gameMenu->Append(
            wxID_EXIT,
            "E&xit\tCtrl+Q",
            "Quit application"
        );

        // Меню справки.
        wxMenu* helpMenu = new wxMenu();

        helpMenu->Append(
            wxID_ABOUT,
            "&About\tF1",
            "Information about the lab"
        );

        wxMenuBar* menuBar = new wxMenuBar();
        menuBar->Append(gameMenu, "&Game");
        menuBar->Append(helpMenu, "&Help");

        SetMenuBar(menuBar);

        Bind(
            wxEVT_MENU,
            &MainFrame::OnNewGame,
            this,
            wxID_NEW
        );

        Bind(
            wxEVT_MENU,
            &MainFrame::OnExit,
            this,
            wxID_EXIT
        );

        Bind(
            wxEVT_MENU,
            &MainFrame::OnAbout,
            this,
            wxID_ABOUT
        );

        Center();
    }

private:
    Board m_board;
    MinimaxEngine m_ai;
    GameCanvas* m_canvas = nullptr;

    void OnNewGame(wxCommandEvent&) {
        m_canvas->ResetGame();
    }

    void OnExit(wxCommandEvent&) {
        Close(true);
    }

    void OnAbout(wxCommandEvent&) {
        wxMessageBox(
            "Infinite Field Tic-Tac-Toe (5-in-a-row)\n"
            "Laboratory Work #2\n\n"
            "Controls:\n"
            " - Left Click: Place 'X'\n"
            " - Right Click + Drag: Pan the board\n"
            " - Mouse Wheel: Zoom in / Zoom out\n\n"
            "Features:\n"
            " - Dynamic associative board powered by Binary Search Tree\n"
            " - Minimax Engine with Alpha-Beta Pruning\n"
            " - Winning five highlighted with a green line",
            "About Project",
            wxOK | wxICON_INFORMATION,
            this
        );
    }
};

#endif // _MAIN_WINDOW_HPP_