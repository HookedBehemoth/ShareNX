#include "ui/elm_FixedMenu.hpp"
#include "util/caps.hpp"

namespace pu::ui::elm
{
    FixedMenuItem::FixedMenuItem(const CapsAlbumEntry& entry) : m_entry(entry)
    {
        this->color = { 10, 10, 10, 255 };
        this->height = 0;
    }

    Color FixedMenuItem::GetColor() const
    {
        return this->color;
    }

    void FixedMenuItem::SetColor(Color Color)
    {
        this->color = Color;
    }

    void FixedMenuItem::AddOnClick(std::function<void()> Callback, u64 Key)
    {
        this->cbs.push_back(Callback);
        this->cbipts.push_back(Key);
    }

    s32 FixedMenuItem::GetCallbackCount() const
    {
        return this->cbs.size();
    }

    std::function<void()> FixedMenuItem::GetCallback(s32 Index) const
    {
        if(this->cbs.empty()) return [&](){};
        return this->cbs[Index];
    }

    u64 FixedMenuItem::GetCallbackKey(s32 Index) const
    {
        return this->cbipts[Index];
    }

    void FixedMenuItem::SetHeight(s32 Height)
    {
        this->height = Height;
    }

    s32 FixedMenuItem::GetHeight() const
    {
        return this->height;
    }
    
    FixedMenu::FixedMenu(s32 X, s32 Y, s32 Width, Color OptionColor, s32 ItemSize, s32 ItemsToShow, s32 fontSize)
        : Element::Element(), x(X), y(Y), w(Width), clr(OptionColor), isize(ItemSize), ishow(ItemsToShow)
    {
        this->scb = { 110, 110, 110, 255 };
        this->previsel = 0;
        this->isel = 0;
        this->fisel = 0;
        this->selfact = 255;
        this->pselfact = 0;
        this->onselch = [&](){};
        this->icdown = false;
        this->dtouch = false;
        this->fcs = { 40, 40, 40, 255 };
        this->basestatus = 0;
        this->font = render::LoadDefaultFont(fontSize);
        this->meme = render::LoadSharedFont(render::SharedFont::NintendoExtended, fontSize);
    }

    s32 FixedMenu::GetX()
    {
        return this->x;
    }

    void FixedMenu::SetX(s32 X)
    {
        this->x = X;
    }

    s32 FixedMenu::GetY()
    {
        return this->y;
    }

    void FixedMenu::SetY(s32 Y)
    {
        this->y = Y;
    }

    s32 FixedMenu::GetWidth()
    {
        return this->w;
    }

    void FixedMenu::SetWidth(s32 Width)
    {
        this->w = Width;
    }

    s32 FixedMenu::GetHeight()
    {
        return (this->isize * this->ishow);
    }

    s32 FixedMenu::GetItemSize() const
    {
        return this->isize;
    }

    void FixedMenu::SetItemSize(s32 ItemSize)
    {
        this->isize = ItemSize;
    }

    s32 FixedMenu::GetNumberOfItemsToShow() const
    {
        return this->ishow;
    }

    void FixedMenu::SetNumberOfItemsToShow(s32 ItemsToShow)
    {
        this->ishow = ItemsToShow;
    }

    Color FixedMenu::GetColor() const
    {
        return this->clr;
    }

    void FixedMenu::SetColor(Color Color)
    {
        this->clr = Color;
    }
    
    Color FixedMenu::GetOnFocusColor() const
    {
        return this->fcs;
    }

    void FixedMenu::SetOnFocusColor(Color Color)
    {
        this->fcs = Color;
    }

    Color FixedMenu::GetScrollbarColor() const
    {
        return this->scb;
    }

    void FixedMenu::SetScrollbarColor(Color Color)
    {
        this->scb = Color;
    }

    void FixedMenu::SetOnSelectionChanged(std::function<void()> Callback)
    {
        this->onselch = Callback;
    }

    void FixedMenu::AddItem(FixedMenuItem::Ref &Item)
    {
        this->itms.push_back(Item);
    }

    void FixedMenu::ClearItems()
    {
        this->itms.clear();
        for (auto [icon,text]: this->ntexs) {
            render::DeleteTexture(icon);
            render::DeleteTexture(text);
        }
        this->ntexs.clear();
    }

    void FixedMenu::SetCooldownEnabled(bool Cooldown)
    {
        this->icdown = Cooldown;
    }

    FixedMenuItem::Ref FixedMenu::GetSelectedItem() const
    {
        return this->itms[this->isel];
    }

    std::vector<FixedMenuItem::Ref> FixedMenu::GetItems() const
    {
        return this->itms;
    }

    s32 FixedMenu::GetSelectedIndex() const
    {
        return this->isel;
    }

    void FixedMenu::SetSelectedIndex(s32 Index)
    {
        if(this->itms.size() > Index)
        {
            this->isel = Index;
            this->fisel = 0;
            if(this->isel >= (this->itms.size() - this->ishow)) this->fisel = this->itms.size() - this->ishow;
            else if(this->isel < this->ishow) this->fisel = 0;
            else this->fisel = this->isel;

            for(s32 i = this->fisel; i < (this->ishow + this->fisel); i++)
                ReloadItemRender(i);
            this->selfact = 255;
            this->pselfact = 0;
        }
    }

    void FixedMenu::OnRender(render::Renderer::Ref &Drawer, s32 X, s32 Y)
    {
        if(!this->itms.empty())
        {
            s32 cx = X;
            s32 cy = Y;
            s32 cw = this->w;
            s32 ch = this->isize;
            s32 its = this->ishow;
            if(its > this->itms.size()) its = this->itms.size();
            if((its + this->fisel) > this->itms.size()) its = this->itms.size() - this->fisel;
            if(this->ntexs.empty()) {
                for (s32 i = this->fisel; i < (its + this->fisel); i++) {
                    std::pair<render::NativeTexture,render::NativeTexture> pair = std::make_pair(nullptr, nullptr);
                    this->ntexs.push_back(pair);
                    this->ReloadItemRender(i);
                }
            }
            for(s32 i = this->fisel; i < (its + this->fisel); i++)
            {
                s32 clrr = this->clr.R;
                s32 clrg = this->clr.G;
                s32 clrb = this->clr.B;
                s32 nr = clrr - 70;
                if(nr < 0) nr = 0;
                s32 ng = clrg - 70;
                if(ng < 0) ng = 0;
                s32 nb = clrb - 70;
                if(nb < 0) nb = 0;
                Color nclr(nr, ng, nb, this->clr.A);
                auto [curicon,curname] = this->ntexs[i%this->ishow];
                if(this->isel == i)
                {
                    Drawer->RenderRectangleFill(this->clr, cx, cy, cw, ch);
                    if(this->selfact < 255)
                    {
                        Drawer->RenderRectangleFill(Color(this->fcs.R, this->fcs.G, this->fcs.B, this->selfact), cx, cy, cw, ch);
                        this->selfact += 48;
                    }
                    else Drawer->RenderRectangleFill(this->fcs, cx, cy, cw, ch);
                }
                else if(this->previsel == i)
                {
                    Drawer->RenderRectangleFill(this->clr, cx, cy, cw, ch);
                    if(this->pselfact > 0)
                    {
                        Drawer->RenderRectangleFill(Color(this->fcs.R, this->fcs.G, this->fcs.B, this->pselfact), cx, cy, cw, ch);
                        this->pselfact -= 48;
                    }
                    else Drawer->RenderRectangleFill(this->clr, cx, cy, cw, ch);
                }
                else Drawer->RenderRectangleFill(this->clr, cx, cy, cw, ch);
                auto itm = this->itms[i];
                if (itm->GetHeight() == 0)
                    itm->SetHeight(render::GetTextureHeight(curname));
                s32 xh = itm->GetHeight();
                s32 tx = (cx + 25);
                s32 ty = ((ch - xh) / 2) + cy;
                s32 ich = (this->isize - 10);
                s32 icw = ((ich * 16) / 9);
                s32 icx = (cx + 10);
                s32 icy = (cy + 5);
                tx = (icx + icw + 25);
                Drawer->RenderTexture(curicon, icx, icy, { -1, icw, ich, -1.0f });
                Drawer->RenderTexture(curname, tx, ty);
                cy += ch;
            }
            if(this->ishow < this->itms.size())
            {
                s32 sccr = this->scb.R;
                s32 sccg = this->scb.G;
                s32 sccb = this->scb.B;
                s32 snr = sccr - 30;
                if(snr < 0) snr = 0;
                s32 sng = sccg - 30;
                if(sng < 0) sng = 0;
                s32 snb = sccb - 30;
                if(snb < 0) snb = 0;
                Color sclr(snr, sng, snb, this->scb.A);
                s32 scx = X + (this->w - 20);
                s32 scy = Y;
                s32 scw = 20;
                s32 sch = (this->ishow * this->isize);
                Drawer->RenderRectangleFill(this->scb, scx, scy, scw, sch);
                s32 fch = ((this->ishow * sch) / this->itms.size());
                s32 fcy = scy + (this->fisel * (sch / this->itms.size()));
                Drawer->RenderRectangleFill(sclr, scx, fcy, scw, fch);
            }
            Drawer->RenderShadowSimple(cx, cy, cw, 5, 160);
        }
    }

    void FixedMenu::OnInput(u64 Down, u64 Up, u64 Held, Touch Pos)
    {
        if(GetItems().size() == 0) return;
        if(basestatus == 1)
        {
            auto curtime = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(curtime - basetime).count();
            if(diff >= 150)
            {
                basestatus = 2;
            }
        }
        if(!Pos.IsEmpty())
        {
            touchPosition tch;
            hidTouchRead(&tch, 0);
            s32 cx = this->GetProcessedX();
            s32 cy = this->GetProcessedY();
            s32 cw = this->w;
            s32 ch = this->isize;
            s32 its = this->ishow;
            if(its > this->itms.size()) its = this->itms.size();
            if((its + this->fisel) > this->itms.size()) its = this->itms.size() - this->fisel;
            for(s32 i = this->fisel; i < (this->fisel + its); i++)
            {
                if(((cx + cw) > Pos.X) && (Pos.X > cx) && ((cy + ch) > Pos.Y) && (Pos.Y > cy))
                {
                    this->dtouch = true;
                    this->previsel = this->isel;
                    this->isel = i;
                    (this->onselch)();
                    if(i == this->isel) this->selfact = 255;
                    else if(i == this->previsel) this->pselfact = 0;
                    break;
                }
                cy += this->isize;
            }
        }
        else if(this->dtouch)
        {
            if((this->selfact >= 255) && (this->pselfact <= 0))
            {
                if(this->icdown) this->icdown = false;
                else (this->itms[this->isel]->GetCallback(0))();
                this->dtouch = false;
            }
        }
        else
        {
            if((Down & KEY_DOWN) || (Held & KEY_RSTICK_DOWN))
            {
                bool move = true;
                if(Held & KEY_RSTICK_DOWN)
                {
                    move = false;
                    if(basestatus == 0)
                    {
                        basetime = std::chrono::steady_clock::now();
                        basestatus = 1;
                    }
                    else if(basestatus == 2)
                    {
                        basestatus = 0;
                        move = true;
                    }
                }
                if(move)
                {
                    if(this->isel < (this->itms.size() - 1))
                    {
                        if((this->isel - this->fisel) == (this->ishow - 1))
                        {
                            this->fisel++;
                            this->isel++;
                            (this->onselch)();
                            ReloadItemRender(this->isel);
                        }
                        else
                        {
                            this->previsel = this->isel;
                            this->isel++;
                            (this->onselch)();
                            if(!this->itms.empty()) for(s32 i = 0; i < this->itms.size(); i++)
                            {
                                if(i == this->isel) this->selfact = 0;
                                else if(i == this->previsel) this->pselfact = 255;
                            }
                        }
                    }
                    else
                    {
                        this->isel = 0;
                        this->fisel = 0;
                        if(this->itms.size() > this->ishow)
                        {
                            for(s32 i = this->fisel; i < (this->ishow + this->fisel); i++)
                                ReloadItemRender(i);
                        }
                    }
                }
            }
            else if((Down & KEY_UP) || (Held & KEY_RSTICK_UP))
            {
                bool move = true;
                if(Held & KEY_RSTICK_UP)
                {
                    move = false;
                    if(basestatus == 0)
                    {
                        basetime = std::chrono::steady_clock::now();
                        basestatus = 1;
                    }
                    else if(basestatus == 2)
                    {
                        basestatus = 0;
                        move = true;
                    }
                }
                if(move)
                {
                    if(this->isel > 0)
                    {
                        if(this->isel == this->fisel)
                        {
                            this->fisel--;
                            this->isel--;
                            (this->onselch)();
                            ReloadItemRender(this->isel);
                        }
                        else
                        {
                            this->previsel = this->isel;
                            this->isel--;
                            (this->onselch)();
                            if(!this->itms.empty()) for(s32 i = 0; i < this->itms.size(); i++)
                            {
                                if(i == this->isel) this->selfact = 0;
                                else if(i == this->previsel) this->pselfact = 255;
                            }
                        }
                    }
                    else
                    {
                        this->isel = this->itms.size() - 1;
                        this->fisel = 0;
                        if(this->itms.size() > this->ishow) {
                            this->fisel = this->itms.size() - this->ishow;
                            for(s32 i = this->fisel; i < (this->ishow + this->fisel); i++)
                                ReloadItemRender(i);
                        }
                    }
                }
            }
            else
            {
                s32 ipc = this->itms[this->isel]->GetCallbackCount();
                if(ipc > 0) for(s32 i = 0; i < ipc; i++)
                {
                    if(Down & this->itms[this->isel]->GetCallbackKey(i))
                    {
                        if(this->icdown) this->icdown = false;
                        else (this->itms[this->isel]->GetCallback(i))();
                    }
                }
            }
        }
    }

    void FixedMenu::ReloadItemRender(int i)
    {
        std::pair<render::NativeTexture,render::NativeTexture>& item = this->ntexs[i%this->ishow];
        if (item.first != nullptr) render::DeleteTexture(item.first);
        if (item.second != nullptr) render::DeleteTexture(item.second);
        item.second = render::RenderText(this->font, this->meme, caps::dateToString(this->itms[i]->m_entry.file_id.datetime), this->itms[i]->GetColor());

        u64 img_size = 320*180*4;
        void* img = malloc(img_size);
        u64 width, height;
        Result rc = caps::getThumbnail(&width, &height, this->itms[i]->m_entry, img, img_size);
        if (width != 320 && height != 180) {
            printf("wrong thumbnail size");
        }
        if (R_SUCCEEDED(rc)) {
            item.first = render::LoadRgbImage(img, width, height, 4);
        } else {
            printf("Failed loading thumbnail with 0x%x\n", rc);
            item.first = render::LoadImage("romfs:/video.png");
        }
        free(img);
    }
}
