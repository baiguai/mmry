// ./src/scrolling/scrolling.cpp
#include "scrolling.h"

void ScrollingComponent::updateHelpDialogScrollOffset(int adjustment) {
    scrollPosition += adjustment;
    if (scrollPosition < 0) {
        scrollPosition = 0;
    } else if (scrollPosition > content.size() - viewportSize) {
        scrollPosition = content.size() - viewportSize;
    }
}
