#include <wx/wx.h>
#include "gui/MainWindow.hpp"

class TicTacToeApp : public wxApp {
public:
    bool OnInit() override {
        MainFrame* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(TicTacToeApp);