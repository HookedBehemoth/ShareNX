
/*

    Plutonium library

    @file Image.hpp
    @brief An Image is an Element showing a picture. (JPEG, PNG, TGA, BMP)
    @author XorTroll

    @copyright Plutonium project - an easy-to-use UI framework for Nintendo Switch homebrew

*/

#pragma once
#include <pu/Plutonium>

namespace pu::ui::elm
{
    class MImage : public Element
    {
        public:
            MImage(s32 X, s32 Y, const std::string& Image);
            MImage(s32 X, s32 Y, void* buffer, s32 size);
            PU_SMART_CTOR(MImage)
            ~MImage();

            s32 GetX();
            void SetX(s32 X);
            s32 GetY();
            void SetY(s32 Y);
            s32 GetWidth();
            void SetWidth(s32 Width);
            s32 GetHeight();
            void SetHeight(s32 Height);
            float GetRotation();
            void SetRotation(float Angle);
            void SetImage(const std::string& Image);
            void SetImage(void* buffer, s32 size);
            void SetRawImage(void* buffer, u64 width, u64 height);
            bool IsImageValid();
            void OnRender(render::Renderer::Ref &Drawer, s32 X, s32 Y);
            void OnInput(u64 Down, u64 Up, u64 Held, Touch Pos);
        private:
            render::NativeTexture ntex;
            render::NativeTextureRenderOptions rendopts;
            s32 x;
            s32 y;
    };
}