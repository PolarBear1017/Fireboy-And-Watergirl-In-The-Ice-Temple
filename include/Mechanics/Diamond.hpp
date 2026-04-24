#ifndef MECHANICS_DIAMOND_HPP
#define MECHANICS_DIAMOND_HPP

#include "Util/GameObject.hpp"
#include "Level/LevelDefinition.hpp"

class Diamond : public Util::GameObject {
public:
    Diamond(const std::shared_ptr<Core::Drawable>& drawable, float zIndex, Element element);

    Element GetElement() const { return m_Element; }
    bool IsCollected() const { return m_Collected; }
    
    void Collect();

private:
    Element m_Element;
    bool m_Collected = false;
};

#endif
