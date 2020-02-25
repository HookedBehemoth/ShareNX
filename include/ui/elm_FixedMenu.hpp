
/*

    Plutonium library

    @file FixedMenu.hpp
    @brief A FixedMenu is a very useful Element for option browsing or selecting.
    @author XorTroll

    @copyright Plutonium project - an easy-to-use UI framework for Nintendo Switch homebrew

*/

#pragma once
#include <chrono>
#include <functional>
#include <pu/ui/elm/elm_Element.hpp>
#include <vector>

#include "util/caps.hpp"

namespace pu::ui::elm {
	class FixedMenuItem {

	public:
		FixedMenuItem(const CapsAlbumEntry &entry);
		PU_SMART_CTOR(FixedMenuItem)
		const CapsAlbumEntry m_entry;
		Color GetColor() const;
		void SetColor(Color Color);
		void AddOnClick(std::function<void()> Callback, u64 Key = KEY_A);
		s32 GetCallbackCount() const;
		std::function<void()> GetCallback(s32 Index) const;
		u64 GetCallbackKey(s32 Index) const;
		void SetHeight(s32 Height);
		s32 GetHeight() const;

	private:
		Color color;
		std::vector<std::function<void()>> cbs;
		std::vector<u64> cbipts;
		s32 height;
	};

	class FixedMenu : public Element {
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
		s32 GetItemSize() const;
		void SetItemSize(s32 ItemSize);
		s32 GetNumberOfItemsToShow() const;
		void SetNumberOfItemsToShow(s32 ItemsToShow);
		Color GetColor() const;
		void SetColor(Color Color);
		Color GetOnFocusColor() const;
		void SetOnFocusColor(Color Color);
		Color GetScrollbarColor() const;
		void SetScrollbarColor(Color Color);
		void SetOnSelectionChanged(std::function<void()> Callback);
		void AddItem(FixedMenuItem::Ref &Item);
		void ClearItems();
		void SetCooldownEnabled(bool Cooldown);
		FixedMenuItem::Ref GetSelectedItem() const;
		std::vector<FixedMenuItem::Ref> GetItems() const;
		s32 GetSelectedIndex() const;
		void SetSelectedIndex(s32 Index);
		void OnRender(render::Renderer::Ref &Drawer, s32 X, s32 Y);
		void OnInput(u64 Down, u64 Up, u64 Held, Touch Pos);

	private:
		void ReloadItemRender(int i);
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
		render::NativeFont meme;
		std::vector<std::pair<render::NativeTexture, render::NativeTexture>> ntexs;
	};

}
