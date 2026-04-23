#include "Mechanics/Button.hpp"
#include "Util/Time.hpp"


Button::Button(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId)
    : Activator(groupId) {
    
    // 1. Base Structure
    m_BaseSprite = std::make_shared<AtlasSprite>(atlas, "pusher_block0000");
    SetDrawable(m_BaseSprite);
    
    // 2. Glow Overlay
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "pusher_block_light0000");
    // 設定發光層的 Z-Index 為 -0.05f，略高於底座 (-0.1f)，但依然低於地面 (0.0f)
    m_PusherObject = std::make_shared<Util::GameObject>(m_Sprite, -0.05f); 
    AddChild(m_PusherObject);
    m_PusherObject->SetVisible(false);
    
    // 3. Layout and Scale
    m_Transform.scale = {0.6f, 0.6f}; 
    
    // 修正漂浮問題：
    // 根據您第二次的截圖，偏移 5.0f 時仍然有大約半格（約 16 像素）的懸空。
    // 這表示 Sprite 圖片（pusher_block0000）的下半部其實包含了大量的透明空白。
    // 為了將「視覺上的底邊」貼齊地面，我們需要將整個物件大幅度向下移。
    m_InitialPosition = pos + glm::vec2(0.0f, 20.0f); 
    m_Transform.translation = m_InitialPosition;
    m_CurrentYOffset = 0.0f;
    
    m_PusherObject->m_Transform.translation = m_Transform.translation;
    m_PusherObject->m_Transform.scale = m_Transform.scale;
    
    // 將整個按鈕底座設定為永遠在地磚 (0.0f) 後方
    SetZIndex(-0.1f); 
}

std::optional<Collider> Button::GetCollider() const {
    glm::vec2 collPos = m_Transform.translation;
    collPos.y += 10.0f; 
    
    return Collider{
        collPos,
        glm::vec2(66.0f, 15.0f)
    };
}

void Button::Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) {
    // 1. Detection Logic
    auto checkPressed = [&](const glm::vec2& charPos) {
        // charPos.y 已經是角色腳底的座標 (由 CollisionSystem 賦值)
        float charBottom = charPos.y;
        float charLeft = charPos.x - 16.0f;
        float charRight = charPos.x + 16.0f;
        
        float btnTop = m_InitialPosition.y + 15.0f; 
        float btnLeft = m_InitialPosition.x - 33.0f;
        float btnRight = m_InitialPosition.x + 33.0f;
        
        // 增加向下容錯空間到 -40.0f，確保按鈕下沉時不會脫離判定
        return (charBottom >= btnTop - 40.0f && charBottom <= btnTop + 10.0f) &&
               (charRight > btnLeft && charLeft < btnRight);
    };

    m_IsPressed = checkPressed(fireboyPos) || checkPressed(watergirlPos);

    // 2. Animation Logic (Sinking)
    // 將下沉深度從 -24.0f 改為 -20.0f。
    // 如果下沉過深（例如 -24），角色的腳底會低於周圍地磚的表面（形成一個坑），
    // 導致左右移動時踢到旁邊地磚的側面牆壁（AABB 水平碰撞），從而卡住。
    // -20.0f 可以確保按鈕被壓下時，其物理表面剛好切齊或微高於地面，讓角色能順利走出去。
    float targetOffset = m_IsPressed ? -20.0f : 0.0f;
    float lerpSpeed = 5.0f; // 降低下沉速度，避免按鈕下沉比重力快導致角色懸空抖動
    float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;
    
    m_CurrentYOffset += (targetOffset - m_CurrentYOffset) * std::min(1.0f, lerpSpeed * dt);
    m_Transform.translation.y = m_InitialPosition.y + m_CurrentYOffset;

    // 3. Visual Feedback
    m_PusherObject->SetVisible(m_IsPressed);
    m_PusherObject->m_Transform.translation = m_Transform.translation;
    
    // Z-Index 已經在建構子設定為永遠在地面之後 (-0.1f)
    // 利用地面作為天然遮罩，完美呈現「陷入」的視覺效果，不需再動態切換
}
