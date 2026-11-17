#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include "basic_transform.h"
#include "texture_atlas.h"
#include "vector_util.h"
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#define GL_SILENCE_DEPRECATION  // silences macOS OpenGL deprecation warnings

#define glCheck(expr)                                                                              \
    expr;                                                                                          \
    if (int e = glGetError()) {                                                                    \
        std::cerr << "OpenGL error: " << e << " in file " __FILE__ << ", line number " << __LINE__ \
                  << '\n';                                                                         \
        abort();                                                                                   \
    }

using SpriteID = size_t;

class SpriteManager {
public:
    SpriteManager(const sf::Vector2u& atlasSize, unsigned padding)
        : mVertices(sf::PrimitiveType::Triangles),
          mNumSprites{0},
          mAtlas(atlasSize, padding),
          mShader(std::filesystem::path{"main.vert"}, std::filesystem::path{"main.frag"}) {
        // TODO
    }

    TextureID registerTexture(const std::string& filepath) {
        return mAtlas.registerTexture(filepath);
    };

    SpriteID createSprite(TextureID textureID) {
        sf::Vector2u pSize = mAtlas.getSizePixels(textureID);
        sf::FloatRect uvBounds = mAtlas.getBoundsUV(textureID);
        mVertices.append(sf::Vertex{{0.f, 0.f}, sf::Color::Red, uvBounds.position});
        mVertices.append(sf::Vertex{{0.f, static_cast<float>(pSize.y)}, sf::Color::Red, uvBounds.position + sf::Vector2f{0.f, uvBounds.size.y}});
        mVertices.append(sf::Vertex{{static_cast<float>(pSize.x), 0.f}, sf::Color::Red, uvBounds.position + sf::Vector2f{uvBounds.size.x, 0.f}});
        mVertices.append(sf::Vertex{{static_cast<float>(pSize.x), 0.f}, sf::Color::Red, uvBounds.position + sf::Vector2f{uvBounds.size.x, 0.f}});
        mVertices.append(sf::Vertex{{0.f, static_cast<float>(pSize.y)}, sf::Color::Red, uvBounds.position + sf::Vector2f{0.f, uvBounds.size.y}});
        mVertices.append(sf::Vertex{{static_cast<float>(pSize.x), static_cast<float>(pSize.y)}, sf::Color::Red, uvBounds.position + uvBounds.size});

        mTransforms.emplace_back(BasicTransform{});
        mNumSprites += 1;
        return mNumSprites - 1;
    };

    void setTexture(SpriteID sprite, TextureID newTexture) {
        assert(sprite < mNumSprites);
        sf::Vector2u pSize = mAtlas.getSizePixels(newTexture);
        sf::FloatRect uvBounds = mAtlas.getBoundsUV(newTexture);

        mVertices[sprite * 6] = sf::Vertex{{0.f, 0.f}, sf::Color::Red, uvBounds.position};
        mVertices[sprite * 6 + 1] = sf::Vertex{{0.f, static_cast<float>(pSize.y)}, sf::Color::Red, uvBounds.position + sf::Vector2f{0.f, uvBounds.size.y}};
        mVertices[sprite * 6 + 2] = sf::Vertex{{static_cast<float>(pSize.x), 0.f}, sf::Color::Red, uvBounds.position + sf::Vector2f{uvBounds.size.x, 0.f}};
        mVertices[sprite * 6 + 3] = sf::Vertex{{static_cast<float>(pSize.x), 0.f}, sf::Color::Red, uvBounds.position + sf::Vector2f{uvBounds.size.x, 0.f}};
        mVertices[sprite * 6 + 4] = sf::Vertex{{0.f, static_cast<float>(pSize.y)}, sf::Color::Red, uvBounds.position + sf::Vector2f{0.f, uvBounds.size.y}};
        mVertices[sprite * 6 + 5] = sf::Vertex{{static_cast<float>(pSize.x), static_cast<float>(pSize.y)}, sf::Color::Red, uvBounds.position + uvBounds.size};
    };

    // TODO
    void draw(sf::RenderTarget& target, sf::RenderStates states) const {};

    // TODO
    void drawGL() {};

    BasicTransform& getTransform(SpriteID id) {
        return mTransforms.at(id);
    }

    const TextureAtlas& getAtlas() const {
        return mAtlas;
    };

private:
    sf::VertexArray mVertices;
    std::vector<BasicTransform> mTransforms;
    size_t mNumSprites;
    TextureAtlas mAtlas;
    mutable sf::Shader mShader;

    // Use the below only in Batched rendering (part 2):
    std::vector<float> mSpriteNumbers;

    GLint mTextureUniformLocation;
    GLint mTransformsUniformLocation;
    GLint mNumTransformsUniformLocation;

    GLint mPositionAttribLocation;
    GLint mTexCoordsAttribLocation;
    GLint mSpriteNumberAttribLocation;

    GLuint mVerticesVBO;
    GLuint mSpriteNumberVBO;
    GLuint mTransformsTexture;
};

#endif  // SPRITE_MANAGER_H
