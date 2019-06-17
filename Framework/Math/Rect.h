#pragma once

class Rect final
{
public:
    Rect();
    Rect(const float& x, const float& y, const float& width, const float& height);
    Rect(const Rect& rhs);
    ~Rect() = default;

    const bool operator==(const Rect& rhs) const
    {
        return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
    }

    const bool operator!=(const Rect& rhs) const
    {
        return !(*this == rhs);
    }

    auto GetIndexCount() const -> const uint { return 6; }
    auto GetVertexBuffer() const -> const std::shared_ptr<class VertexBuffer>& { return vertex_buffer; }
    auto GetIndexBuffer() const -> const std::shared_ptr<class IndexBuffer>& { return index_buffer; }

    void CreateBuffers(class Renderer* renderer);

public:
    float x;
    float y;
    float width;
    float height;

private:
    std::shared_ptr<class VertexBuffer> vertex_buffer;
    std::shared_ptr<class IndexBuffer> index_buffer;
};