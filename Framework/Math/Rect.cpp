#include "Framework.h"
#include "Rect.h"

Rect::Rect()
    : x(0.0f)
    , y(0.0f)
    , width(0.0f)
    , height(0.0f)
{
}

Rect::Rect(const float & x, const float & y, const float & width, const float & height)
    : x(x)
    , y(y)
    , width(width)
    , height(height)
{
}

Rect::Rect(const Rect & rhs)
    : x(rhs.x)
    , y(rhs.y)
    , width(rhs.width)
    , height(rhs.height)
{
}

void Rect::CreateBuffers(Renderer * renderer)
{
    if (!renderer)
    {
        LOG_ERROR("Invalid parameter. renderer is nullptr");
        return;
    }

    auto resolution     = renderer->GetResolution();
    float screen_left   = -(resolution.x * 0.5f) + x;
    float screen_right  = screen_left + width;
    float screen_top    = (resolution.y * 0.5f) - y;
    float screen_bottom = screen_top - height;

    std::vector<VertexTexture> vertices
    {
        VertexTexture(D3DXVECTOR3(screen_left, screen_bottom, 0.0f), D3DXVECTOR2(0.0f, 1.0f)), //bl
        VertexTexture(D3DXVECTOR3(screen_left, screen_top, 0.0f), D3DXVECTOR2(0.0f, 0.0f)), //tl
        VertexTexture(D3DXVECTOR3(screen_right, screen_bottom, 0.0f), D3DXVECTOR2(1.0f, 1.0f)), //br
        VertexTexture(D3DXVECTOR3(screen_right, screen_top, 0.0f), D3DXVECTOR2(1.0f, 0.0f)), //tr
    };

    std::vector<uint> indices{ 0, 1, 2, 2, 1, 3 };

    vertex_buffer = std::make_shared<VertexBuffer>(renderer->GetContext());
    vertex_buffer->Create(vertices);

    index_buffer = std::make_shared<IndexBuffer>(renderer->GetContext());
    index_buffer->Create(indices);
}
