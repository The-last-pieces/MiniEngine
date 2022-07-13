#include "view/gui.hpp"

// 窗口尺寸
const int SCR_WIDTH  = 800;
const int SCR_HEIGHT = 693; // =800/2*sqrt(3)

int main() {
    mne::MainWindow window(
        "MnZn's Graphics Engine",
        SCR_WIDTH, SCR_HEIGHT
    );
    window.show();
    return 0;
}