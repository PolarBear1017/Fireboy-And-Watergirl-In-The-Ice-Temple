# 2026 OOPL Final Report

## 組別資訊

組別：第61組<br>
組員：113590028 黃暉帆、113590001 楊竣升<br>
復刻遊戲：Fireboy & Watergirl In The Ice Temple（冰火姊弟3）

## 專案簡介

### 遊戲簡介
《冰火姊弟》（或稱冰火人、Fireboy & Watergirl）是一系列經典的雙人協作冒險遊戲。玩家需同時操作火男孩（Fireboy）與冰女孩（Watergirl），利用兩人對應元素池（冰、火）的抗性，合作收集寶石並規避毒沼，共同解開機關順利到達出口。

關鍵要素： 
- 角色特性： 火娃不怕火但怕水，水娃不怕水但怕火，且兩人都不能觸碰毒沼。
- 遊戲目標： 合作操作、解開機關，收集相應顏色的寶石並抵達最終出口。 
- 操作方式： 為雙人合作闖關遊戲，一位玩家使用 WAD 鍵，另一位使用方向鍵。 
- 遊戲系列： 包括森林冒險、光明神殿、寒冰宮殿等不同主題的關卡。（我們製作的是第三版的冰宮）

總而言之是一款非常考驗雙人配合默契的益智遊戲。

參考影片： https://www.youtube.com/watch?v=4mnXDSAYWRY


### 組別分工
113590028 黃暉帆：
- 碰撞系統 Collision System 實作 (碰撞箱)
- 原版地圖轉換程式 (convert)
- Sprite Sheet 精靈圖素材圖片切割
- 外掛系統 develop mode（顯示碰撞箱、直接通關、無敵星星...）
- 遊戲地圖地形場景實作
- menu關卡選單實作
- 機關 UI 與動畫實作（按鈕、搖桿、平台、方塊、倒計時按鈕...）
- 藍、紅寶石與特殊寶石實作
- 開門動畫與UI、過關機制
- 遊戲邏輯實作（撿寶石、角色死亡動畫與重生機制）
- 遊戲視窗縮放功能
- 機關染色調色功能
  

113590001 楊竣升：
- 角色操作、動畫與摩擦碰撞
- 地圖場景實作（地圖素材切割、地圖資料讀取與顯示）
- 斜坡顯示與碰撞
- 水池動畫、碰撞與凍結融化機制
- 雪面地形碰撞


## 遊戲介紹

### 遊戲規則

- **遊戲目的**
  - 利用機關、躲避陷阱危害，走到各自的終點。
  - 獲取所有寶石。
  - 任一角色死亡，視為關卡失敗，立即重置關卡。
- **操作按鍵**
  - 方向鍵（左右上）操作火娃水平移動與跳躍
  - 方向鍵（W,A,D）操作水娃水平移動與跳躍
- **地圖場景與物件**
  - **終點門**
  <br>水娃與火娃各有一扇專屬的終點門。兩人都走到對應的門前方並站定，才能通關。
  - **寶石**
    - 分為藍紅兩色。通關過程中水娃與火娃只能蒐集與自身同色的寶石。
    - 未蒐集所有寶石不影響通關。
    - 備註：原作中在通關時會得到評級（由高到低分為綠、橙、紫三種評級），是否蒐集完所有寶石只會影響評級。因時間有限，本組未實作通關評級機制。
  - **元素池（陷阱）**
    - 水池：水娃可以正常通行；火娃接觸則死亡。
    - 岩漿：火娃可以正常通行；水娃接觸則死亡。
    - 毒沼：水娃與火娃接觸毒沼皆會死亡。
  - **機關**
    - **普通按鈕**：可控制移動平台或閘門。
  <br>　　　　　按住時啟動，鬆開時解除啟動。
    - **搖桿**：可控制移動平台或閘門。
  <br>　　　可左右推動並於啟動與非啟動之間轉換，朝左為啟動，朝右為非啟動。
    - **定時按鈕**：可控制移動平台或閘門。
  <br>　　　　　按住時啟動，鬆開時開始倒計時，期間維持啟動，計時結束後解除啟動。
    - **白色按鈕**：形狀與普通按鈕相同，為白色。
  <br>　　　　　可控制水元素池凍結或融化。按下時若目標元素池為水，將其凍結；反之（為冰時）則將其融化。
  <br>　　　　　轉換結束後按下按鈕才會再次觸發凍結或融化效果。
  <br>　　　　　備註：原作的水池凍融機制本來是由光束（藍光束與紅光束）觸發，考量到時間有限與實作上過於複雜，故妥協更改為以按鈕觸發。
    - **移動平台與閘門**：啟動時，沿固定路線移往特定位置；非啟動時，沿固定路線移回原位。
  <br>　　　　　　　　分為水平與垂直，可承載角色移動或阻擋角色行進，視關卡設計而定。
    - **方形石塊**：可推移的墊腳石。
  - **冰雪地形**
    - 角色在踩踏到積雪或冰面（凍結水池）時，移動速度和跳躍與爬坡能力會受到影響。
    - 水娃：無法跳躍，左右移動變得極為緩慢，仍可爬上斜坡。
    - 火娃：無法跳躍，左右移動稍微加速（打滑），無法正常爬上斜坡（快速滑落）。

### 遊戲畫面
|   階段   |                        遊戲畫面                        |
|:------:|:--------------------------------------------------:|
|  開始畫面  |  <img src="FinalProjectImg/game_cover.png" width="400">  |
|  關卡選單  |  <img src="FinalProjectImg/level_menu.png" width="400">  |
|  第一關  |  <img src="FinalProjectImg/level0.png" width="400">  |
| 第二關  | <img src="FinalProjectImg/level2.png" width="400">  |
|  第三關  |  <img src="FinalProjectImg/level6.png" width="400">  |
|  第四關  |  <img src="FinalProjectImg/level7.png" width="400">  |
|  第五關  |  <img src="FinalProjectImg/level21.png" width="400">  |
| 角色死亡 | <img src="FinalProjectImg/character_died_animation.gif" width="400"> |
|  搖桿觸發  |  <img src="FinalProjectImg/trigger_lever.gif" width="400">  |
|  按鈕觸發  |  <img src="FinalProjectImg/trigger_button.gif" width="400">  |
|  白色按鈕觸發  |  <img src="FinalProjectImg/white_button.gif" width="400">  |
|  倒計時按鈕觸發   |  <img src="FinalProjectImg/timed_button_trigger.gif" width="400">   |
|  雪地效果   |  <img src="FinalProjectImg/snow_feature.gif" width="400">   |
| 過關動畫 | <img src="FinalProjectImg/pass_level_animation.gif" width="400"> |
| 開發者外掛介面 | <img src="FinalProjectImg/dev_tools.png" width="400"> |
| 碰撞箱顯示 | <img src="FinalProjectImg/show_colliders.png" width="400"> |
|  角色泡水顯示  |  <img src="FinalProjectImg/character_in_pool.png" width="400">  |


## 程式設計

### 程式架構

```mermaid
graph TD


    Util::GameObject --> Character
    Util::GameObject --> Door
    Util::GameObject --> Overlay
    Util::GameObject --> BaseMechanism
    Util::GameObject --> Diamond
    
    BaseMechanism --> Activator
    BaseMechanism --> Receiver
    BaseMechanism --> Block
    
    Activator --> Button
    Activator --> Lever
    Activator --> TimedButton
    Receiver --> Elevator
    
    std::enable_shared_from_this<TriggerMediator> --> TriggerMediator
    
    Core::Drawable --> AtlasSprite
    
    IInputController --> KeyboardInputController
    
    LevelManager
    CollisionSystem
    SpriteAtlas
    App

```

### 程式技術
- 機關物件繼承鏈
    - 在實作機關時，我們發現各個機關有很多重複的地方，所以我們拉出了 `BaseMechanism` 繼承`Game Object`。然後因為機關有分為兩類：觸發器 `Activator` 和接收器 `Reciver`，我們將他們繼承 `BaseMechanism`來實現物件的基本機制。然後在 `Activator` 和 `Reciver` 的子類別各自放該功能的機關Object。
- 從原版地圖 convert 成我們地圖的格式
    - 在製作地圖時，因為考慮到需要批量產出地圖，所以我們有寫了一個 `convert.py` 的轉換程式，能夠將原版的地圖格式架構轉換成我們遊戲中的格式架構，幫助我們減少了很多需要自行製作還原地圖的時間。
- Spirit Sheet 大圖切圖功能
    - 在一開始搜集素材時，因為我們找到的素材都是使用 `Spirit Sheet` （整張大圖有很多小圖素材，需要使用附檔的.json來切圖，這樣在讀檔時會比需要連續讀好幾十張圖的動話還方便快速），但是我們有詢問過製作 PTSD 架構的學長有沒有支援 Spirit Sheet 的功能，不過很可惜沒有，所以我們自行撰寫實作了 Spirit Sheet 大圖切圖的邏輯並應用在我們的專案中。
- 介面使用
    - `IScene`：將各個場景都會用到的基礎部分寫在`IScene`讓其他的Scene當作介面引入。實作了像是場景的初始化、更新、以及畫上DevMenu(開發者外掛)的純虛擬函數。
    - `IInputController`：將按鍵輸入的控制獨立拉出成介面，設定左右方向以及是否跳起的純虛擬函式，之後再做擴充時，像是如果要換成使用遊戲搖桿來玩，擴充就更方便。

### 使用到 AI/AI Agent 的部分 (沒有用到者，不需要寫這篇)

## 結語

### 問題與解決方法
### 自評

| 項次 | 項目                   | 完成 |
|------|------------------------|-------|
| 1    | 這是範例 |  V  |
| 2    | 完成專案權限改為 public |    |
| 3    | 具有 debug mode 的功能  |    |
| 4    | 解決專案上所有 Memory Leak 的問題  |    |
| 5    | 報告中沒有任何錯字，以及沒有任何一項遺漏  |    |
| 6    | 報告至少保持基本的美感，人類可讀  |    |

### 心得

- 113590028 黃暉帆

- 113590001 楊竣升

### 貢獻比例

|      組員       | 貢獻度 |
|:-------------:|:---:|
| 113590028 黃暉帆 | 50% |
| 113590001 楊竣升 | 50% |
