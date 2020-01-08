#include "ui/elm_MemImage.hpp"

namespace pu::ui::elm
{
    MImage::MImage(s32 X, s32 Y, const std::string& Image) : Element::Element()
    {
        this->x = X;
        this->y = Y;
        this->ntex = NULL;
        this->rendopts = render::NativeTextureRenderOptions::Default;
        this->SetImage(Image);
    }

    MImage::MImage(s32 X, s32 Y, void* buffer, s32 size) : Element::Element()
    {
        this->x = X;
        this->y = Y;
        this->ntex = NULL;
        this->rendopts = render::NativeTextureRenderOptions::Default;
        this->SetImage(buffer, size);
    }

    MImage::~MImage()
    {
        if(this->ntex != NULL)
        {
            render::DeleteTexture(this->ntex);
            this->ntex = NULL;
        }
    }

    s32 MImage::GetX()
    {
        return this->x;
    }

    void MImage::SetX(s32 X)
    {
        this->x = X;
    }

    s32 MImage::GetY()
    {
        return this->y;
    }

    void MImage::SetY(s32 Y)
    {
        this->y = Y;
    }

    s32 MImage::GetWidth()
    {
        return this->rendopts.Width;
    }

    void MImage::SetWidth(s32 Width)
    {
        this->rendopts.Width = Width;
    }

    s32 MImage::GetHeight()
    {
        return this->rendopts.Height;
    }

    void MImage::SetHeight(s32 Height)
    {
        this->rendopts.Height = Height;
    }

    float MImage::GetRotation()
    {
        return this->rendopts.Angle;
    }

    void MImage::SetRotation(float Angle)
    {
        this->rendopts.Angle = Angle;
    }

    void MImage::SetImage(const std::string& Image)
    {
        if(this->ntex != NULL) render::DeleteTexture(this->ntex);
        this->ntex = NULL;
        std::ifstream ifs(Image);
        bool ok = ifs.good();
        ifs.close();
        if(ok)    
        {
            this->ntex = render::LoadImage(Image);
            this->rendopts.Width = render::GetTextureWidth(this->ntex);
            this->rendopts.Height = render::GetTextureHeight(this->ntex);
        }
    }

    void MImage::SetImage(void* buffer, s32 size)
    {
        if(this->ntex != NULL) render::DeleteTexture(this->ntex);
        this->ntex = NULL;
        this->ntex = render::ConvertToTexture(IMG_Load_RW(SDL_RWFromMem(buffer, size), size));
        this->rendopts.Width = render::GetTextureWidth(this->ntex);
        this->rendopts.Height = render::GetTextureHeight(this->ntex);
    }

    void MImage::SetRawImage(void* buffer, u64 width, u64 height)
    {
        if(this->ntex != NULL) render::DeleteTexture(this->ntex);
        this->ntex = NULL;
        this->ntex = render::ConvertToTexture(SDL_CreateRGBSurfaceFrom(buffer, width, height, 32, 4*width, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000));
        this->rendopts.Width = render::GetTextureWidth(this->ntex);
        this->rendopts.Height = render::GetTextureHeight(this->ntex);
    }

    bool MImage::IsImageValid()
    {
        return (ntex != NULL);
    }

    void MImage::OnRender(render::Renderer::Ref &Drawer, s32 X, s32 Y)
    {
        Drawer->RenderTexture(this->ntex, X, Y, this->rendopts);
    }

    void MImage::OnInput(u64 Down, u64 Up, u64 Held, Touch Pos)
    {
    }
}