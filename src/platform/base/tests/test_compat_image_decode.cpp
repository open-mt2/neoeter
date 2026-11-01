#include <support/TestAssert.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include <Platform/DirectXMathCompat.h>
#include <Platform/RenderBackend.h>

// The D3DX compat shim must decode non-DDS images (TGA/JPG/BMP) on every
// target that uses the GL renderer, not only on Linux. On Windows the decode was
// guarded by OPENMT2_PLATFORM_LINUX, so every such texture became an empty 64x64
// image and the call still returned S_OK (open-mt2/devkit#136).
//
// The shim uploads through the process-wide render backend. A recording backend
// stands in for GL here, so the test needs no GL context and also proves the
// decoded pixels reach the backend.

namespace {

struct Upload {
    int count = 0;
    uint32_t format = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> data;
};

class RecordingBackend : public IRenderBackend {
public:
    Upload upload;

    RenderCaps Caps() const override { return RenderCaps{}; }
    RenderHandle CreateTexture() override { return RenderHandle{1, 1}; }
    RenderHandle CreateDepthStencil(uint32_t, uint32_t) override { return RenderHandle{2, 1}; }
    void Destroy(RenderHandle) override {}
    void UploadTextureLevel(RenderHandle, uint32_t level, uint32_t format, uint32_t width,
                            uint32_t height, const void* data, size_t size) override {
        if (level != 0) return;
        ++upload.count;
        upload.format = format;
        upload.width = width;
        upload.height = height;
        upload.data.assign(static_cast<const uint8_t*>(data),
                           static_cast<const uint8_t*>(data) + size);
    }
    void FinalizeTexture(RenderHandle, uint32_t) override {}
    void BindDefaultRenderTarget() override {}
    void BindRenderTarget(RenderHandle) override {}
    void BindDepthStencil(RenderHandle) override {}
    void SetViewport(int32_t, int32_t, uint32_t, uint32_t) override {}
    void Clear(uint32_t, uint32_t, float, uint32_t) override {}
    void BeginFrame() override {}
    void Present() override {}
    void BindStageTexture(uint32_t, RenderHandle) override {}
    void ApplyState(const RenderState&) override {}
    void Draw(const DrawCall&) override {}
    bool Readback(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t*) override { return false; }
};

RecordingBackend g_backend;

} // namespace

IRenderBackend& Platform_GetRenderBackend()
{
    return g_backend;
}

// Uncompressed 32-bit TGA, top-left origin, 3x2:
//   row 0: red,   green, blue
//   row 1: white, translucent grey, black
static std::vector<unsigned char> MakeTga3x2()
{
    std::vector<unsigned char> tga(18, 0);
    tga[2] = 2;          // uncompressed true-colour
    tga[12] = 3;         // width
    tga[14] = 2;         // height
    tga[16] = 32;        // bits per pixel
    tga[17] = 0x28;      // top-left origin, 8 alpha bits

    const unsigned char bgra[6][4] = {
        {0x00, 0x00, 0xff, 0xff}, // red
        {0x00, 0xff, 0x00, 0xff}, // green
        {0xff, 0x00, 0x00, 0xff}, // blue
        {0xff, 0xff, 0xff, 0xff}, // white
        {0x40, 0x80, 0xc0, 0x80}, // translucent
        {0x00, 0x00, 0x00, 0xff}, // black
    };
    for (const auto& px : bgra)
        tga.insert(tga.end(), px, px + 4);
    return tga;
}

static const unsigned char kExpectedBgra[6][4] = {
    {0x00, 0x00, 0xff, 0xff},
    {0x00, 0xff, 0x00, 0xff},
    {0xff, 0x00, 0x00, 0xff},
    {0xff, 0xff, 0xff, 0xff},
    {0x40, 0x80, 0xc0, 0x80},
    {0x00, 0x00, 0x00, 0xff},
};

int main()
{
    const std::vector<unsigned char> tga = MakeTga3x2();

    TEST_SECTION("D3DXGetImageInfoFromFileInMemory reads the real TGA size");
    {
        D3DXIMAGE_INFO info;
        TEST_ASSERT(SUCCEEDED(D3DXGetImageInfoFromFileInMemory(tga.data(), (UINT) tga.size(), &info)));
        TEST_ASSERT(info.Width == 3);
        TEST_ASSERT(info.Height == 2);
        TEST_ASSERT(info.Format == D3DFMT_A8R8G8B8);
        TEST_PASS("3x2 A8R8G8B8, not the 64x64 placeholder");
    }

    TEST_SECTION("D3DXCreateTextureFromFileInMemoryEx decodes the TGA pixels");
    {
        IDirect3DTexture9* texture = nullptr;
        TEST_ASSERT(SUCCEEDED(D3DXCreateTextureFromFileInMemoryEx(
            nullptr, tga.data(), (UINT) tga.size(), 3, 2, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0xffff00ff, nullptr, nullptr, &texture)));
        TEST_ASSERT(texture != nullptr);
        TEST_ASSERT(texture->GetWidth() == 3);
        TEST_ASSERT(texture->GetHeight() == 2);

        D3DLOCKED_RECT lr;
        TEST_ASSERT(SUCCEEDED(texture->LockRect(0, &lr, nullptr, 0)));
        TEST_ASSERT(lr.pBits != nullptr);
        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 3; ++x) {
                const unsigned char* got = static_cast<const unsigned char*>(lr.pBits) + y * lr.Pitch + x * 4;
                TEST_ASSERT(std::memcmp(got, kExpectedBgra[y * 3 + x], 4) == 0);
            }
        }
        texture->UnlockRect(0);
        TEST_PASS("every pixel matches, in BGRA order");

        // The upload is what ends up on screen.
        TEST_ASSERT(g_backend.upload.count > 0);
        TEST_ASSERT(g_backend.upload.width == 3);
        TEST_ASSERT(g_backend.upload.height == 2);
        TEST_ASSERT(g_backend.upload.format == D3DFMT_A8R8G8B8);
        TEST_ASSERT(g_backend.upload.data.size() == 3u * 2u * 4u);
        TEST_ASSERT(std::memcmp(g_backend.upload.data.data(), kExpectedBgra, sizeof(kExpectedBgra)) == 0);
        TEST_PASS("the decoded pixels reach the render backend");

        texture->Release();
    }

    TEST_SECTION("an undecodable image fails instead of returning an empty texture");
    {
        const std::vector<unsigned char> junk(64, 0x7f);
        IDirect3DTexture9* texture = nullptr;
        TEST_ASSERT(FAILED(D3DXCreateTextureFromFileInMemoryEx(
            nullptr, junk.data(), (UINT) junk.size(), 64, 64, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0xffff00ff, nullptr, nullptr, &texture)));
        TEST_ASSERT(texture == nullptr);
        TEST_PASS("E_FAIL, no texture handed out");
    }

    return 0;
}
