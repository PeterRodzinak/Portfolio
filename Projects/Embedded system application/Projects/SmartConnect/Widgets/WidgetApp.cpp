#include "WidgetApp.hpp"

AppState WidgetApp::getState() {
    return state;
}

void WidgetApp::setState(AppState s) {
    state = s;
}
