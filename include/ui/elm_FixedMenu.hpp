
/*

    Plutonium library

    @file FixedMenu.hpp
    @brief A FixedMenu is a very useful Element for option browsing or selecting.
    @author XorTroll

    @copyright Plutonium project - an easy-to-use UI framework for Nintendo Switch homebrew

*/

#pragma once
#include <pu/pu_String.hpp>
#include <pu/ui/elm/elm_Element.hpp>
#include <vector>
#include <chrono>
#include <functional>

namespace pu::ui::elm
{
    class FixedMenuItem
    {
        public:
            FixedMenuItem(String Name);
            PU_SMART_CTOR(FixedMenuItem)

            String GetName();
            void SetName(String Name);
            Color GetColor();
            void SetColor(Color Color);
            void AddOnClick(std::function<void()> Callback, u64 Key = KEY_A);
            s32 GetCallbackCount();
            std::function<void()> GetCallback(s32 Index);
            u64 GetCallbackKey(s32 Index);
            std::string GetIcon();
            void SetIcon(std::string Icon);
            bool HasIcon();
        private:
            String name;
            Color clr;
            bool hasicon;
            std::string icon;
            std::vector<std::function<void()>> cbs;
            std::vector<u64> cbipts;
    };

    class FixedMenu : public Element
    {
        public:
            FixedMenu(s32 X, s32 Y, s32 Width, Color OptionColor, s32 ItemSize, s32 ItemsToShow, s32 fontSize = 25);
            PU_SMART_CTOR(FixedMenu)

            s32 GetX();
            void SetX(s32 X);
            s32 GetY();
            void SetY(s32 Y);
            s32 GetWidth();
            void SetWidth(s32 Width);
            s32 GetHeight();
            s32 GetItemSize();
            void SetItemSize(s32 ItemSize);
            s32 GetNumberOfItemsToShow();
            void SetNumberOfItemsToShow(s32 ItemsToShow);
            Color GetColor();
            void SetColor(Color Color);
            Color GetOnFocusColor();
            void SetOnFocusColor(Color Color);
            Color GetScrollbarColor();
            void SetScrollbarColor(Color Color);
            void SetOnSelectionChanged(std::function<void()> Callback);
            void AddItem(FixedMenuItem::Ref &Item);
            void ClearItems();
            void SetCooldownEnabled(bool Cooldown);
            FixedMenuItem::Ref &GetSelectedItem();
            std::vector<FixedMenuItem::Ref> &GetItems();
            s32 GetSelectedIndex();
            void SetSelectedIndex(s32 Index);
            void OnRender(render::Renderer::Ref &Drawer, s32 X, s32 Y);
            void OnInput(u64 Down, u64 Up, u64 Held, Touch Pos);
        private:
            void ReloadItemRenders();
            bool dtouch;
            s32 x;
            s32 y;
            s32 w;
            s32 isize;
            s32 ishow;
            s32 previsel;
            s32 fisel;
            s32 isel;
            s32 pselfact;
            s32 selfact;
            Color scb;
            Color clr;
            Color fcs;
            bool icdown;
            int basestatus;
            std::chrono::time_point<std::chrono::steady_clock> basetime;
            std::function<void()> onselch;
            std::vector<FixedMenuItem::Ref> itms;
            render::NativeFont font;
            std::vector<render::NativeTexture> loadednames;
            std::vector<render::NativeTexture> loadedicons;
    };
}
