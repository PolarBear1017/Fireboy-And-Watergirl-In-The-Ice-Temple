# 2026 OOPL Final Report

## 組別資訊

組別：第61組<br>
組員：113590028 黃暉帆、113590001 楊竣升<br>
復刻遊戲：Fireboy & Watergirl In The Ice Temple（冰火姊弟3）

## 專案簡介

### 遊戲簡介
《冰火姊弟》（或稱森林冰火人、Fireboy & Watergirl）是一系列經典的雙人協作冒險遊戲。玩家需同時操作火男孩（Fireboy）與冰女孩（Watergirl），利用兩人對應元素池（冰、火）的抗性，合作收集寶石並規避毒沼，共同解開機關順利到達出口。

關鍵要素： 角色特性： 火男孩不怕火但怕水，冰女孩不怕水但怕火，且兩人都不能觸碰毒沼。 遊戲目標： 合作操作、解開機關，收集相應顏色的寶石並抵達最終出口。 操作方式： 通常為雙人對戰設計，一位玩家使用 WASD 鍵，另一位使用方向鍵。 遊戲系列： 包括森林冒險、光明神殿、寒冰宮殿等不同主題的關卡。（我們製作的是第三版的冰宮） 總而言之是一款非常考驗雙人配合默契的益智遊戲。

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


## 遊戲介紹

### 遊戲規則
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
|  角色泡水顯示  |  <img src="FinalProjectImg/character_in_pool.png" width="400">  |
| 角色死亡 | <img src="FinalProjectImg/character_died_animation.gif" width="400"> |
|  搖桿觸發  |  <img src="FinalProjectImg/trigger_lever.gif" width="400">  |
|  按鈕觸發  |  <img src="FinalProjectImg/trigger_button.gif" width="400">  |
|  白色按鈕觸發  |  <img src="FinalProjectImg/white_button.gif" width="400">  |
|  倒計時按鈕觸發   |  <img src="FinalProjectImg/timed_button_trigger.gif" width="400">   |
|  雪地效果   |  <img src="FinalProjectImg/snow_feature.gif" width="400">   |
| 過關動畫 | <img src="FinalProjectImg/pass_level_animation.gif" width="400"> |
| 開發者外掛介面 | <img src="FinalProjectImg/dev_tools.png" width="400"> |
| 碰撞箱顯示 | <img src="FinalProjectImg/show_colliders.png" width="400"> |


## 程式設計

### 程式架構
### 程式技術
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

### 貢獻比例