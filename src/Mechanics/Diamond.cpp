#include "Mechanics/Diamond.hpp"

Diamond::Diamond(const std::shared_ptr<Core::Drawable>& drawable, float zIndex, Element element)
    : GameObject(drawable, zIndex), m_Element(element) {}

void Diamond::Collect() {
    m_Collected = true;
    SetVisible(false); // 收集後隱藏
}
