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
        mPositionAttribLocation = glGetAttribLocation(mShader.getNativeHandle(), "position");
        glCheck();
        mTexCoordsAttribLocation = glGetAttribLocation(mShader.getNativeHandle(), "texCoords");
        glCheck();
        mSpriteNumberAttribLocation = glGetAttribLocation(mShader.getNativeHandle(), "spriteNumber");
        glCheck();
        mTextureUniformLocation = glGetUniformLocation(mShader.getNativeHandle(), "texture");
        glCheck();
        mTransformsUniformLocation = glGetUniformLocation(mShader.getNativeHandle(), "transforms");
        glCheck();
        mNumTransformsUniformLocation = glGetUniformLocation(mShader.getNativeHandle(), "numTransforms");
        glCheck();

        glCheck(glGenBuffers(1, &mVerticesVBO));
        glCheck(glGenBuffers(1, &mSpriteNumberVBO));
        glCheck(glGenTextures(1, &mTransformsTexture));
        
        glCheck(glActiveTexture(GL_TEXTURE1));
        glCheck(glBindTexture(GL_TEXTURE_1D, mTransformsTexture));
        glCheck(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        glCheck(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        glCheck(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    }

    ~SpriteManager() {
       glDeleteTextures(1, &mVerticesVBO);
       glDeleteTextures(1, &mSpriteNumberVBO);
       glDeleteTextures(1, &mTransformsTexture); 
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
        mSpriteNumbers.push_back(mNumSprites);
        mSpriteNumbers.push_back(mNumSprites);
        mSpriteNumbers.push_back(mNumSprites);
        mSpriteNumbers.push_back(mNumSprites);
        mSpriteNumbers.push_back(mNumSprites);
        mSpriteNumbers.push_back(mNumSprites);
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

    void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        sf::VertexArray temp(mVertices);
        for (int i = 0; i < mNumSprites; ++i) {
            for (int j = 0; j < 6; ++j) {
                temp[i * 6 + j].position = mTransforms[i].applyPoint(temp[i * 6 + j].position);
            }
        }

        states.texture = &mAtlas.getTexture();
        //https://www.sfml-dev.org/tutorials/3.0/graphics/shader/#fragment-shader
        mShader.setUniform("texture", sf::Shader::CurrentTexture);


        states.shader = &mShader;
        target.draw(temp, states);
    };

    // TODO
    void drawGL() {
        // Must specify current program in use
        glUseProgram(mShader.getNativeHandle());

        glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVerticesVBO));
        glCheck(glBufferData(GL_ARRAY_BUFFER, mVertices.getVertexCount() * sizeof(sf::Vertex), &mVertices[0],
                                GL_STATIC_DRAW));      
        // set position attribute layout
        glCheck(glVertexAttribPointer(mPositionAttribLocation, 2, GL_FLOAT, GL_FALSE,
                                      sizeof(sf::Vertex), (const void*)0));
        glEnableVertexAttribArray(mPositionAttribLocation);
        // set texCoordsIn attribute layout
        glCheck(glVertexAttribPointer(mTexCoordsAttribLocation, 2, GL_FLOAT, GL_FALSE,
                                      sizeof(sf::Vertex),
                                      (const void*)(sizeof(sf::Vector2f) + sizeof(sf::Color))));
        glEnableVertexAttribArray(mTexCoordsAttribLocation);
        glCheck();


        glCheck(glBindBuffer(GL_ARRAY_BUFFER, mSpriteNumberVBO));
        glCheck(glBufferData(GL_ARRAY_BUFFER, mSpriteNumbers.size() * sizeof(float), mSpriteNumbers.data(),
                     GL_STATIC_DRAW));
        glCheck(glVertexAttribPointer(mSpriteNumberAttribLocation, 1, GL_FLOAT, GL_FALSE,
                                      sizeof(float),
                                      (const void*)0));
        glEnableVertexAttribArray(mSpriteNumberAttribLocation);

        glCheck(glActiveTexture(GL_TEXTURE0));
        // Bind texture to GL_TEXTURE_2D target of texture unit 0
        glCheck(glBindTexture(GL_TEXTURE_2D, mAtlas.getTexture().getNativeHandle()));
        glCheck(glUniform1i(mTextureUniformLocation, 0));

        glCheck(glActiveTexture(GL_TEXTURE1));
        glCheck(glBindTexture(GL_TEXTURE_1D, mTransformsTexture));
        glCheck(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F_ARB, mNumSprites, 0, GL_RGBA, GL_FLOAT, mTransforms.data()));
        glCheck(glUniform1i(mTransformsUniformLocation, 1));

        glCheck(glUniform1i(mNumTransformsUniformLocation, mNumSprites));
        glCheck(glDrawArrays(GL_TRIANGLES, 0, mVertices.getVertexCount()));


    };

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
