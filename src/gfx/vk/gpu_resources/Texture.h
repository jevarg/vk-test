#pragma once

#include <memory>

#include "../types/VulkanContext.h"
#include "Image.h"

class Buffer;

class Texture {
   public:
    typedef size_t ID;

    explicit Texture(const char* filename);
    Texture(Texture&& other) noexcept = default;

    void destroy() const;

    [[nodiscard]]
    const Image& getImage() const;

    [[nodiscard]]
    ID getID() const;

   private:
    inline static ID lastID = 0;
    static ID nextID() { return lastID++; }

    const ID m_id = nextID();

    std::unique_ptr<Buffer> m_stagingBuffer;
    std::unique_ptr<Image> m_image;
};
