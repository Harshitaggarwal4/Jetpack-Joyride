#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <bits/stdc++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

using namespace std;
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;
int score = 0;

struct Character
{
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

std::map<GLchar, Character> Characters;
unsigned int vao_text, vbo_text;
unsigned int shaderProgram_text;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec3 aCol;\n"
                                 "layout (location = 2) in vec2 aTexCoord;\n"
                                 "uniform mat4 trans;\n"
                                 "uniform float glow;\n"
                                 "out float gloww;\n"
                                 "out vec3 outCol;\n"
                                 "out vec2 TexCoord;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = trans*vec4(aPos.x,aPos.y,aPos.z,1.0);\n"
                                 "   outCol = aCol;\n"
                                 "   TexCoord = aTexCoord;\n"
                                 "   gloww = glow;\n"
                                 "}\n\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "in vec3 outCol;\n"
                                   "in vec2 TexCoord;\n"
                                   "in float gloww;\n"
                                   "out vec4 FragColor;\n"
                                   "uniform sampler2D ourTexture;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   vec4 harshit = texture(ourTexture, TexCoord);\n"
                                   "   if(harshit.a<1)\n"
                                   "   {\n"
                                   "      if(gloww == 1){\n"
                                   "         float dist = distance(TexCoord, vec2(0.5,0.5));\n"
                                   "         if(dist > 0.5){\n"
                                   "            discard;\n"
                                   "         }\n"
                                   "         else{\n"
                                   "            harshit = mix(harshit, vec4(1.0,1.0,0.0,1.0), 1-smoothstep(0,1,dist));\n"
                                   "         }\n"
                                   "      }\n"
                                   "      else{\n"
                                   "         discard;\n"
                                   "      }\n"
                                   "   }\n"
                                   "   FragColor=harshit;\n"
                                   "}\n\0";

const char *vertexShaderSource_text = "#version 330 core\n"
                                      "layout (location = 0) in vec4 vertex; \n"
                                      "out vec2 TexCoords;\n"
                                      "uniform mat4 projection;\n"
                                      "void main()\n"
                                      "{\n"
                                      "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
                                      "    TexCoords = vertex.zw;\n"
                                      "}\n\0";
const char *fragmentShaderSource_text = "#version 330 core\n"
                                        "in vec2 TexCoords;\n"
                                        "out vec4 color;\n"
                                        "uniform sampler2D text;\n"
                                        "uniform vec3 textColor;\n"
                                        "void main()\n"
                                        "{  \n"
                                        "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
                                        "    color = vec4(textColor, 1.0) * sampled;\n"
                                        "}\n\0";

void jetpack_joyrider(int level, float speed_of_background, int number_of_zappers, GLFWwindow *window, unsigned int shaderProgram, int number_of_levels, int lives)
{
    stbi_set_flip_vertically_on_load(true);
    float init_player_x = -0.8;
    float init_player_y = -0.63;
    float init_player_max_y = 0.53;
    float size_of_player = 0.175;
    float gravity_acceleration = -2.5;
    float jetpack_acceleration = 2.25;
    float collision_distance = 0.003;
    float size_of_youloose = 0.25;
    float distance_between_zappers = 0.65f;
    float size_of_coin = 0.1;
    float collision_distance_coins = 0.01;
    float invincibility_period = 1.0f;
    int frames_for_walking = 10;
    int frames_for_coins = 4;
    float width_of_zapper = 0.1;
    float time_for_level = 2.0f;

    float vertices_of_background[] = {-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 3.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
    unsigned int indices_of_background[] = {0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7};

    float vertices_of_zappers[number_of_zappers * 8 * 4];
    unsigned int indices_of_zappers[number_of_zappers * 6];
    for (int i = 0; i < number_of_zappers; i++)
    {
        float ytranslate = (float)rand();
        ytranslate /= 72452;
        while (ytranslate > (1 - distance_between_zappers / 2))
        {
            ytranslate /= 2;
        }
        ytranslate *= pow(-1, rand());
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(-1 * (distance_between_zappers * i + distance_between_zappers * 2), -1 * ytranslate, 0.0f));
        glm::mat4 rotate = glm::mat4(1.0f);
        rotate = glm::rotate(rotate, glm::radians((float)rand()), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 trans2 = glm::mat4(1.0f);
        trans2 = glm::translate(trans2, glm::vec3((distance_between_zappers * i + distance_between_zappers * 2), ytranslate, 0.0f));
        glm::vec4 vector = glm::vec4((distance_between_zappers * i - width_of_zapper + distance_between_zappers * 2), (-1 * (distance_between_zappers / 2) + ytranslate), 0, 1);
        vector = trans2 * rotate * trans * vector;
        vertices_of_zappers[i * 32 + 0] = vector[0];
        vertices_of_zappers[i * 32 + 1] = vector[1];
        vertices_of_zappers[i * 32 + 2] = vector[2];
        vertices_of_zappers[i * 32 + 3] = 1;
        vertices_of_zappers[i * 32 + 4] = 0;
        vertices_of_zappers[i * 32 + 5] = 0;
        vertices_of_zappers[i * 32 + 6] = 0;
        vertices_of_zappers[i * 32 + 7] = 0;
        vector = glm::vec4((distance_between_zappers * i - width_of_zapper + distance_between_zappers * 2), ((distance_between_zappers / 2) + ytranslate), 0, 1);
        vector = trans2 * rotate * trans * vector;
        vertices_of_zappers[i * 32 + 8] = vector[0];
        vertices_of_zappers[i * 32 + 9] = vector[1];
        vertices_of_zappers[i * 32 + 10] = vector[2];
        vertices_of_zappers[i * 32 + 11] = 1;
        vertices_of_zappers[i * 32 + 12] = 0;
        vertices_of_zappers[i * 32 + 13] = 0;
        vertices_of_zappers[i * 32 + 14] = 0;
        vertices_of_zappers[i * 32 + 15] = 1;
        vector = glm::vec4((distance_between_zappers * i + width_of_zapper + distance_between_zappers * 2), (-1 * (distance_between_zappers / 2) + ytranslate), 0, 1);
        vector = trans2 * rotate * trans * vector;
        vertices_of_zappers[i * 32 + 16] = vector[0];
        vertices_of_zappers[i * 32 + 17] = vector[1];
        vertices_of_zappers[i * 32 + 18] = vector[2];
        vertices_of_zappers[i * 32 + 19] = 1;
        vertices_of_zappers[i * 32 + 20] = 0;
        vertices_of_zappers[i * 32 + 21] = 0;
        vertices_of_zappers[i * 32 + 22] = 1;
        vertices_of_zappers[i * 32 + 23] = 0;
        vector = glm::vec4((distance_between_zappers * i + width_of_zapper + distance_between_zappers * 2), ((distance_between_zappers / 2) + ytranslate), 0, 1);
        vector = trans2 * rotate * trans * vector;
        vertices_of_zappers[i * 32 + 24] = vector[0];
        vertices_of_zappers[i * 32 + 25] = vector[1];
        vertices_of_zappers[i * 32 + 26] = vector[2];
        vertices_of_zappers[i * 32 + 27] = 1;
        vertices_of_zappers[i * 32 + 28] = 0;
        vertices_of_zappers[i * 32 + 29] = 0;
        vertices_of_zappers[i * 32 + 30] = 1;
        vertices_of_zappers[i * 32 + 31] = 1;
        indices_of_zappers[i * 6 + 0] = i * 4 + 0;
        indices_of_zappers[i * 6 + 1] = i * 4 + 1;
        indices_of_zappers[i * 6 + 2] = i * 4 + 2;
        indices_of_zappers[i * 6 + 3] = i * 4 + 1;
        indices_of_zappers[i * 6 + 4] = i * 4 + 2;
        indices_of_zappers[i * 6 + 5] = i * 4 + 3;
    }

    float vertices_of_moving_zapper[] = {
        (distance_between_zappers * number_of_zappers - width_of_zapper + distance_between_zappers * 2),
        (-1 * (distance_between_zappers / 2)),
        0,
        0,
        0,
        0,
        0,
        0,
        (distance_between_zappers * number_of_zappers - width_of_zapper + distance_between_zappers * 2),
        ((distance_between_zappers / 2)),
        0,
        0,
        0,
        0,
        0,
        1,
        (distance_between_zappers * number_of_zappers + width_of_zapper + distance_between_zappers * 2),
        (-1 * (distance_between_zappers / 2)),
        0,
        0,
        0,
        0,
        1,
        0,
        (distance_between_zappers * number_of_zappers + width_of_zapper + distance_between_zappers * 2),
        ((distance_between_zappers / 2)),
        0,
        0,
        0,
        0,
        1,
        1,
    };
    unsigned int indices_of_moving_zapper[] = {0, 1, 2, 1, 2, 3};

    float vertices_of_player[] = {init_player_x, init_player_y, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, init_player_x, init_player_y + size_of_player, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, init_player_x + size_of_player, init_player_y, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, init_player_x + size_of_player, init_player_y + size_of_player, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0};
    unsigned int indices_of_player[] = {0, 1, 2, 1, 2, 3};

    float vertices_of_youloose[] = {-1 * size_of_youloose, -1 * size_of_youloose, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1 * size_of_youloose, size_of_youloose, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, size_of_youloose, -1 * size_of_youloose, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, size_of_youloose, size_of_youloose, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
    unsigned int indices_of_youloose[] = {0, 1, 2, 1, 2, 3};

    float vertices_of_coins[] = {0.0, init_player_y, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, init_player_y + size_of_coin, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, size_of_coin, init_player_y, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, size_of_coin, init_player_y + size_of_coin, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0};
    unsigned int indices_of_coins[] = {0, 1, 2, 1, 2, 3};

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }
    std::string font_name = "../fonts/Antonio-Bold.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
    }
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }
    else
    {
        FT_Set_Pixel_Sizes(face, 0, 48);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (unsigned char c = 0; c < 128; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Character character = {texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), static_cast<unsigned int>(face->glyph->advance.x)};
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    unsigned int vbo_background, ebo_background, vao_background;
    glGenVertexArrays(1, &vao_background);
    glGenBuffers(1, &vbo_background);
    glGenBuffers(1, &ebo_background);
    glBindVertexArray(vao_background);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_background);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_of_background), vertices_of_background, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_background);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_of_background), indices_of_background, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    unsigned int vbo_zappers, ebo_zappers, vao_zappers;
    glGenVertexArrays(1, &vao_zappers);
    glGenBuffers(1, &vbo_zappers);
    glGenBuffers(1, &ebo_zappers);
    glBindVertexArray(vao_zappers);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_zappers);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_of_zappers), vertices_of_zappers, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_zappers);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_of_zappers), indices_of_zappers, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    unsigned int vbo_moving_zapper, ebo_moving_zapper, vao_moving_zapper;
    glGenVertexArrays(1, &vao_moving_zapper);
    glGenBuffers(1, &vbo_moving_zapper);
    glGenBuffers(1, &ebo_moving_zapper);
    glBindVertexArray(vao_moving_zapper);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_moving_zapper);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_of_moving_zapper), vertices_of_moving_zapper, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_moving_zapper);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_of_moving_zapper), indices_of_moving_zapper, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    unsigned int vbo_player, ebo_player, vao_player;
    glGenVertexArrays(1, &vao_player);
    glGenBuffers(1, &vbo_player);
    glGenBuffers(1, &ebo_player);
    glBindVertexArray(vao_player);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_player);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_of_player), vertices_of_player, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_player);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_of_player), indices_of_player, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    unsigned int vbo_youloose, ebo_youloose, vao_youloose;
    glGenVertexArrays(1, &vao_youloose);
    glGenBuffers(1, &vbo_youloose);
    glGenBuffers(1, &ebo_youloose);
    glBindVertexArray(vao_youloose);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_youloose);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_of_youloose), vertices_of_youloose, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_youloose);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_of_youloose), indices_of_youloose, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    unsigned int vbo_coins, ebo_coins, vao_coins;
    glGenVertexArrays(1, &vao_coins);
    glGenBuffers(1, &vbo_coins);
    glGenBuffers(1, &ebo_coins);
    glBindVertexArray(vao_coins);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_coins);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_of_coins), vertices_of_coins, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_coins);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_of_coins), indices_of_coins, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &vao_text);
    glGenBuffers(1, &vbo_text);
    glBindVertexArray(vao_text);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_text);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    unsigned int texture_player_1;
    glGenTextures(1, &texture_player_1);
    glBindTexture(GL_TEXTURE_2D, texture_player_1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_player_1, height_player_1, nrChannels_player_1;
    unsigned char *data_player_1 = stbi_load("../textures/1.png", &width_player_1, &height_player_1, &nrChannels_player_1, 0);
    if (data_player_1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_player_1, height_player_1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_player_1);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_player_1);

    unsigned int texture_player_2;
    glGenTextures(1, &texture_player_2);
    glBindTexture(GL_TEXTURE_2D, texture_player_2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_player_2, height_player_2, nrChannels_player_2;
    unsigned char *data_player_2 = stbi_load("../textures/2.png", &width_player_2, &height_player_2, &nrChannels_player_2, 0);
    if (data_player_2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_player_2, height_player_2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_player_2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_player_2);

    unsigned int texture_player_3;
    glGenTextures(1, &texture_player_3);
    glBindTexture(GL_TEXTURE_2D, texture_player_3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_player_3, height_player_3, nrChannels_player_3;
    unsigned char *data_player_3 = stbi_load("../textures/3.png", &width_player_3, &height_player_3, &nrChannels_player_3, 0);
    if (data_player_3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_player_1, height_player_1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_player_3);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_player_3);

    unsigned int texture_player_4;
    glGenTextures(1, &texture_player_4);
    glBindTexture(GL_TEXTURE_2D, texture_player_4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_player_4, height_player_4, nrChannels_player_4;
    unsigned char *data_player_4 = stbi_load("../textures/4.png", &width_player_4, &height_player_4, &nrChannels_player_4, 0);
    if (data_player_4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_player_4, height_player_4, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_player_4);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_player_4);

    unsigned int texture_player_5;
    glGenTextures(1, &texture_player_5);
    glBindTexture(GL_TEXTURE_2D, texture_player_5);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_player_5, height_player_5, nrChannels_player_5;
    unsigned char *data_player_5 = stbi_load("../textures/5.png", &width_player_5, &height_player_5, &nrChannels_player_5, 0);
    if (data_player_5)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_player_5, height_player_5, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_player_5);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_player_5);

    unsigned int texture_zapper;
    glGenTextures(1, &texture_zapper);
    glBindTexture(GL_TEXTURE_2D, texture_zapper);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_zapper, height_zapper, nrChannels_zapper;
    unsigned char *data_zapper = stbi_load("../textures/zapper.png", &width_zapper, &height_zapper, &nrChannels_zapper, 0);
    if (data_zapper)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_zapper, height_zapper, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_zapper);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_zapper);

    unsigned int texture_background;
    glGenTextures(1, &texture_background);
    glBindTexture(GL_TEXTURE_2D, texture_background);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_background, height_background, nrChannels_background;
    unsigned char *data_background = stbi_load("../textures/background.png", &width_background, &height_background, &nrChannels_background, 0);
    if (data_background)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_background, height_background, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_background);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_background);

    unsigned int texture_youloose;
    glGenTextures(1, &texture_youloose);
    glBindTexture(GL_TEXTURE_2D, texture_youloose);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_youloose, height_youloose, nrChannels_youloose;
    unsigned char *data_youloose = stbi_load("../textures/youloose.png", &width_youloose, &height_youloose, &nrChannels_youloose, 0);
    if (data_youloose)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_youloose, height_youloose, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_youloose);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_youloose);

    unsigned int texture_youwin;
    glGenTextures(1, &texture_youwin);
    glBindTexture(GL_TEXTURE_2D, texture_youwin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_youwin, height_youwin, nrChannels_youwin;
    unsigned char *data_youwin = stbi_load("../textures/youwin.png", &width_youwin, &height_youwin, &nrChannels_youwin, 0);
    if (data_youwin)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_youwin, height_youwin, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_youwin);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_youwin);

    unsigned int texture_coins_1;
    glGenTextures(1, &texture_coins_1);
    glBindTexture(GL_TEXTURE_2D, texture_coins_1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_coins_1, height_coins_1, nrChannels_coins_1;
    unsigned char *data_coins_1 = stbi_load("../textures/coins_1.png", &width_coins_1, &height_coins_1, &nrChannels_coins_1, 0);
    if (data_coins_1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_coins_1, height_coins_1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_coins_1);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_coins_1);

    unsigned int texture_coins_2;
    glGenTextures(1, &texture_coins_2);
    glBindTexture(GL_TEXTURE_2D, texture_coins_2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_coins_2, height_coins_2, nrChannels_coins_2;
    unsigned char *data_coins_2 = stbi_load("../textures/coins_2.png", &width_coins_2, &height_coins_2, &nrChannels_coins_2, 0);
    if (data_coins_2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_coins_2, height_coins_2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_coins_2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_coins_2);

    unsigned int texture_coins_3;
    glGenTextures(1, &texture_coins_3);
    glBindTexture(GL_TEXTURE_2D, texture_coins_3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_coins_3, height_coins_3, nrChannels_coins_3;
    unsigned char *data_coins_3 = stbi_load("../textures/coins_3.png", &width_coins_3, &height_coins_3, &nrChannels_coins_3, 0);
    if (data_coins_3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_coins_3, height_coins_3, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_coins_3);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_coins_3);

    unsigned int texture_coins_4;
    glGenTextures(1, &texture_coins_4);
    glBindTexture(GL_TEXTURE_2D, texture_coins_4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_coins_4, height_coins_4, nrChannels_coins_4;
    unsigned char *data_coins_4 = stbi_load("../textures/coins_4.png", &width_coins_4, &height_coins_4, &nrChannels_coins_4, 0);
    if (data_coins_4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_coins_4, height_coins_4, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_coins_4);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_coins_4);

    unsigned int texture_coins_5;
    glGenTextures(1, &texture_coins_5);
    glBindTexture(GL_TEXTURE_2D, texture_coins_5);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_coins_5, height_coins_5, nrChannels_coins_5;
    unsigned char *data_coins_5 = stbi_load("../textures/coins_5.png", &width_coins_5, &height_coins_5, &nrChannels_coins_5, 0);
    if (data_coins_5)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_coins_5, height_coins_5, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_coins_5);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_coins_5);

    unsigned int texture_coins_6;
    glGenTextures(1, &texture_coins_6);
    glBindTexture(GL_TEXTURE_2D, texture_coins_6);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_coins_6, height_coins_6, nrChannels_coins_6;
    unsigned char *data_coins_6 = stbi_load("../textures/coins_6.png", &width_coins_6, &height_coins_6, &nrChannels_coins_6, 0);
    if (data_coins_6)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_coins_6, height_coins_6, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_coins_6);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_coins_6);

    float make_time_zero = glfwGetTime() + time_for_level;
    int sub = 0;
    float time = (glfwGetTime() - make_time_zero);
    float player_y = 0;
    int acceleration = gravity_acceleration;
    bool space_pressed = false;
    int flag = 0;
    float invincibility_time_starts = 0;

    vector<float> ytrans;
    vector<bool> isalive;
    int coin_number1 = 0;
    while (coin_number1 * size_of_coin < distance_between_zappers * (number_of_zappers + 1))
    {
        float ytranslate = (float)rand();
        while (ytranslate > init_player_y * -1 + init_player_max_y)
        {
            ytranslate /= 7;
        }
        ytrans.push_back(ytranslate);
        glm::vec3 translation_vector = glm::vec3(coin_number1 * size_of_coin, ytrans[coin_number1], 0.0f);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, translation_vector);
        int flagg = 0;
        glm::vec4 coords = glm::vec4(vertices_of_coins[0], vertices_of_coins[1], vertices_of_coins[2], 1.0f);
        coords = trans * coords;
        float x1 = coords[0];
        float y1 = coords[1];
        coords = glm::vec4(vertices_of_coins[8], vertices_of_coins[9], vertices_of_coins[10], 1.0f);
        coords = trans * coords;
        float x2 = coords[0];
        float y2 = coords[1];
        coords = glm::vec4(vertices_of_coins[16], vertices_of_coins[17], vertices_of_coins[18], 1.0f);
        coords = trans * coords;
        float x3 = coords[0];
        float y3 = coords[1];
        coords = glm::vec4(vertices_of_coins[24], vertices_of_coins[25], vertices_of_coins[26], 1.0f);
        coords = trans * coords;
        float x4 = coords[0];
        float y4 = coords[1];
        for (int i = 0; i < number_of_zappers; i++)
        {
            float xx1 = vertices_of_zappers[i * 32 + 0];
            float yy1 = vertices_of_zappers[i * 32 + 1];
            float xx2 = vertices_of_zappers[i * 32 + 8];
            float yy2 = vertices_of_zappers[i * 32 + 9];
            float xx3 = vertices_of_zappers[i * 32 + 16];
            float yy3 = vertices_of_zappers[i * 32 + 17];
            float xx4 = vertices_of_zappers[i * 32 + 24];
            float yy4 = vertices_of_zappers[i * 32 + 25];
            float distance_xx1_x1 = sqrt((x1 - xx1) * (x1 - xx1) + (y1 - yy1) * (y1 - yy1));
            float distance_xx2_x1 = sqrt((x1 - xx2) * (x1 - xx2) + (y1 - yy2) * (y1 - yy2));
            float distance_xx3_x1 = sqrt((x1 - xx3) * (x1 - xx3) + (y1 - yy3) * (y1 - yy3));
            float distance_xx4_x1 = sqrt((x1 - xx4) * (x1 - xx4) + (y1 - yy4) * (y1 - yy4));
            float distance_xx1_x2 = sqrt((x2 - xx1) * (x2 - xx1) + (y2 - yy1) * (y2 - yy1));
            float distance_xx2_x2 = sqrt((x2 - xx2) * (x2 - xx2) + (y2 - yy2) * (y2 - yy2));
            float distance_xx3_x2 = sqrt((x2 - xx3) * (x2 - xx3) + (y2 - yy3) * (y2 - yy3));
            float distance_xx4_x2 = sqrt((x2 - xx4) * (x2 - xx4) + (y2 - yy4) * (y2 - yy4));
            float distance_xx1_x3 = sqrt((x3 - xx1) * (x3 - xx1) + (y3 - yy1) * (y3 - yy1));
            float distance_xx2_x3 = sqrt((x3 - xx2) * (x3 - xx2) + (y3 - yy2) * (y3 - yy2));
            float distance_xx3_x3 = sqrt((x3 - xx3) * (x3 - xx3) + (y3 - yy3) * (y3 - yy3));
            float distance_xx4_x3 = sqrt((x3 - xx4) * (x3 - xx4) + (y3 - yy4) * (y3 - yy4));
            float distance_xx1_x4 = sqrt((x4 - xx1) * (x4 - xx1) + (y4 - yy1) * (y4 - yy1));
            float distance_xx2_x4 = sqrt((x4 - xx2) * (x4 - xx2) + (y4 - yy2) * (y4 - yy2));
            float distance_xx3_x4 = sqrt((x4 - xx3) * (x4 - xx3) + (y4 - yy3) * (y4 - yy3));
            float distance_xx4_x4 = sqrt((x4 - xx4) * (x4 - xx4) + (y4 - yy4) * (y4 - yy4));
            float distance_xx1_xx2 = sqrt((xx1 - xx2) * (xx1 - xx2) + (yy1 - yy2) * (yy1 - yy2));
            float distance_xx1_xx3 = sqrt((xx1 - xx3) * (xx1 - xx3) + (yy1 - yy3) * (yy1 - yy3));
            float distance_xx4_xx2 = sqrt((xx4 - xx2) * (xx4 - xx2) + (yy4 - yy2) * (yy4 - yy2));
            float distance_xx3_xx4 = sqrt((xx3 - xx4) * (xx3 - xx4) + (yy3 - yy4) * (yy3 - yy4));
            float distance_x1_x2 = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
            float distance_x1_x3 = sqrt((x1 - x3) * (x1 - x3) + (y1 - y3) * (y1 - y3));
            float distance_x4_x2 = sqrt((x4 - x2) * (x4 - x2) + (y4 - y2) * (y4 - y2));
            float distance_x3_x4 = sqrt((x3 - x4) * (x3 - x4) + (y3 - y4) * (y3 - y4));
            if (distance_xx1_x1 + distance_xx2_x1 - distance_xx1_xx2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x1 + distance_xx3_x1 - distance_xx1_xx3 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx4_x1 + distance_xx2_x1 - distance_xx4_xx2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx3_x1 + distance_xx4_x1 - distance_xx3_xx4 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x2 + distance_xx2_x2 - distance_xx1_xx2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x2 + distance_xx3_x2 - distance_xx1_xx3 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx4_x2 + distance_xx2_x2 - distance_xx4_xx2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx3_x2 + distance_xx4_x2 - distance_xx3_xx4 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x3 + distance_xx2_x3 - distance_xx1_xx2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x3 + distance_xx3_x3 - distance_xx1_xx3 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx4_x3 + distance_xx2_x3 - distance_xx4_xx2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx3_x3 + distance_xx4_x3 - distance_xx3_xx4 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x4 + distance_xx2_x4 - distance_xx1_xx2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x4 + distance_xx3_x4 - distance_xx1_xx3 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx4_x4 + distance_xx2_x4 - distance_xx4_xx2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx3_x4 + distance_xx4_x4 - distance_xx3_xx4 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x1 + distance_xx1_x2 - distance_x1_x2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x1 + distance_xx1_x3 - distance_x1_x3 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x4 + distance_xx1_x2 - distance_x4_x2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx1_x3 + distance_xx1_x4 - distance_x3_x4 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx2_x1 + distance_xx2_x2 - distance_x1_x2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx2_x1 + distance_xx2_x3 - distance_x1_x3 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx2_x4 + distance_xx2_x2 - distance_x4_x2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx2_x3 + distance_xx2_x4 - distance_x3_x4 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx3_x1 + distance_xx3_x2 - distance_x1_x2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx3_x1 + distance_xx3_x3 - distance_x1_x3 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx3_x4 + distance_xx3_x2 - distance_x4_x2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx3_x3 + distance_xx3_x4 - distance_x3_x4 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx4_x1 + distance_xx4_x2 - distance_x1_x2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx4_x1 + distance_xx4_x3 - distance_x1_x3 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx4_x4 + distance_xx4_x2 - distance_x4_x2 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
            if (distance_xx4_x3 + distance_xx4_x4 - distance_x3_x4 < collision_distance_coins)
            {
                flagg = 1;
                break;
            }
        }
        if (flagg == 0)
        {
            isalive.push_back(true);
        }
        else
        {
            isalive.push_back(false);
        }
        coin_number1++;
    }
    int count_for_walking = 0;
    int walking_number = 1;
    int count_for_coins = 0;
    int coins_number = 1;
    int level_show = 0;
    float start_time = glfwGetTime();
    int sub1 = 0;
    float glow = 0;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        if (level_show == 0)
        {
            if ((speed_of_background * -1 * ((glfwGetTime() - make_time_zero + time_for_level)) + sub1 * 2) < -2)
            {
                sub1++;
            }

            glm::vec3 translation_of_background_vector = glm::vec3((speed_of_background * -1 * ((glfwGetTime() - make_time_zero + time_for_level)) + sub1 * 2), 0.0f, 0.0f);
            glm::mat4 transformation_of_background = glm::mat4(1.0f);
            transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
            int transLoc = glGetUniformLocation(shaderProgram, "trans");
            glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

            glBindTexture(GL_TEXTURE_2D, texture_background);
            glBindVertexArray(vao_background);
            glDrawElements(GL_TRIANGLES, sizeof(indices_of_background) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            RenderText("Level:", 160.0f, 270.0f, 2.0f, glm::vec3(0.3, 0.7f, 0.9f));
            RenderText(to_string(level), 380.0f, 270.0f, 2.0f, glm::vec3(0.3, 0.7f, 0.9f));
            glUseProgram(shaderProgram);

            glfwSwapBuffers(window);
            glfwPollEvents();
            if (glfwGetTime() - start_time > time_for_level)
            {
                level_show = 1;
            }
            continue;
        }

        count_for_coins++;
        if (count_for_coins == frames_for_coins)
        {
            count_for_coins = 0;
            coins_number++;
            coins_number %= 7;
            if (coins_number == 0)
            {
                coins_number++;
            }
        }

        if (speed_of_background * -1 * (glfwGetTime() - make_time_zero) + sub * 2 < -2)
        {
            sub++;
        }

        float displacement = player_y + 0.5f * acceleration * ((glfwGetTime() - make_time_zero) - time) * ((glfwGetTime() - make_time_zero) - time);
        if (displacement <= 0)
        {
            displacement = 0;
            count_for_walking++;
            if (count_for_walking == frames_for_walking)
            {
                count_for_walking = 0;
                walking_number++;
                walking_number %= 5;
                if (walking_number == 0)
                {
                    walking_number++;
                }
            }
        }

        if (displacement > init_player_y * -1 + init_player_max_y)
        {
            displacement = init_player_y * -1 + init_player_max_y;
        }

        if (glfwGetTime() - invincibility_time_starts > invincibility_period)
        {
            glow = 0;
            glm::vec3 displacement_zapper = glm::vec3((speed_of_background * -1 * (glfwGetTime() - make_time_zero)), 0.0f, 0.0f);
            glm::mat4 transformation_of_zapper = glm::mat4(1.0f);
            transformation_of_zapper = glm::translate(transformation_of_zapper, displacement_zapper);
            for (int i = 0; i <= number_of_zappers; i++)
            {
                if (level < 3 && i == number_of_zappers)
                {
                    continue;
                }
                if (i == number_of_zappers)
                {
                    glm::vec3 translation_of_background_vector = glm::vec3((speed_of_background * -1 * (glfwGetTime() - make_time_zero)), 0.0f, 0.0f);
                    glm::mat4 transformation_of_background = glm::mat4(1.0f);
                    transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                    glm::mat4 trans = glm::mat4(1.0f);
                    trans = glm::translate(trans, glm::vec3(-1 * (distance_between_zappers * number_of_zappers + distance_between_zappers * 2), 0.0f, 0.0f));
                    glm::mat4 rotate = glm::mat4(1.0f);
                    rotate = glm::rotate(rotate, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
                    glm::mat4 trans2 = glm::mat4(1.0f);
                    trans2 = glm::translate(trans2, glm::vec3((distance_between_zappers * number_of_zappers + distance_between_zappers * 2), 0.0f, 0.0f));
                    transformation_of_zapper = transformation_of_background * trans2 * rotate * trans;
                }
                float x1 = init_player_x;
                float y1 = init_player_y + displacement;
                float x2 = init_player_x;
                float y2 = init_player_y + displacement + size_of_player;
                float x3 = init_player_x + size_of_player;
                float y3 = init_player_y + displacement;
                float x4 = init_player_x + size_of_player;
                float y4 = init_player_y + displacement + size_of_player;
                glm::vec4 coords_zapper;
                if (i == number_of_zappers)
                {
                    coords_zapper = glm::vec4(vertices_of_moving_zapper[0], vertices_of_moving_zapper[1], 0.0f, 1.0f);
                }
                else
                {
                    coords_zapper = glm::vec4(vertices_of_zappers[i * 32 + 0], vertices_of_zappers[i * 32 + 1], vertices_of_zappers[i * 32 + 2], 1.0f);
                }
                coords_zapper = transformation_of_zapper * coords_zapper;
                float xx1 = coords_zapper[0];
                float yy1 = coords_zapper[1];
                if (i == number_of_zappers)
                {
                    coords_zapper = glm::vec4(vertices_of_moving_zapper[8], vertices_of_moving_zapper[9], 0.0f, 1.0f);
                }
                else
                {
                    coords_zapper = glm::vec4(vertices_of_zappers[i * 32 + 8], vertices_of_zappers[i * 32 + 9], vertices_of_zappers[i * 32 + 10], 1.0f);
                }
                coords_zapper = transformation_of_zapper * coords_zapper;
                float xx2 = coords_zapper[0];
                float yy2 = coords_zapper[1];
                if (i == number_of_zappers)
                {
                    coords_zapper = glm::vec4(vertices_of_moving_zapper[16], vertices_of_moving_zapper[17], 0.0f, 1.0f);
                }
                else
                {

                    coords_zapper = glm::vec4(vertices_of_zappers[i * 32 + 16], vertices_of_zappers[i * 32 + 17], vertices_of_zappers[i * 32 + 18], 1.0f);
                }
                coords_zapper = transformation_of_zapper * coords_zapper;
                float xx3 = coords_zapper[0];
                float yy3 = coords_zapper[1];
                if (i == number_of_zappers)
                {
                    coords_zapper = glm::vec4(vertices_of_moving_zapper[24], vertices_of_moving_zapper[25], 0.0f, 1.0f);
                }
                else
                {

                    coords_zapper = glm::vec4(vertices_of_zappers[i * 32 + 24], vertices_of_zappers[i * 32 + 25], vertices_of_zappers[i * 32 + 26], 1.0f);
                }
                coords_zapper = transformation_of_zapper * coords_zapper;
                float xx4 = coords_zapper[0];
                float yy4 = coords_zapper[1];
                float distance_xx1_x1 = sqrt((x1 - xx1) * (x1 - xx1) + (y1 - yy1) * (y1 - yy1));
                float distance_xx2_x1 = sqrt((x1 - xx2) * (x1 - xx2) + (y1 - yy2) * (y1 - yy2));
                float distance_xx3_x1 = sqrt((x1 - xx3) * (x1 - xx3) + (y1 - yy3) * (y1 - yy3));
                float distance_xx4_x1 = sqrt((x1 - xx4) * (x1 - xx4) + (y1 - yy4) * (y1 - yy4));
                float distance_xx1_x2 = sqrt((x2 - xx1) * (x2 - xx1) + (y2 - yy1) * (y2 - yy1));
                float distance_xx2_x2 = sqrt((x2 - xx2) * (x2 - xx2) + (y2 - yy2) * (y2 - yy2));
                float distance_xx3_x2 = sqrt((x2 - xx3) * (x2 - xx3) + (y2 - yy3) * (y2 - yy3));
                float distance_xx4_x2 = sqrt((x2 - xx4) * (x2 - xx4) + (y2 - yy4) * (y2 - yy4));
                float distance_xx1_x3 = sqrt((x3 - xx1) * (x3 - xx1) + (y3 - yy1) * (y3 - yy1));
                float distance_xx2_x3 = sqrt((x3 - xx2) * (x3 - xx2) + (y3 - yy2) * (y3 - yy2));
                float distance_xx3_x3 = sqrt((x3 - xx3) * (x3 - xx3) + (y3 - yy3) * (y3 - yy3));
                float distance_xx4_x3 = sqrt((x3 - xx4) * (x3 - xx4) + (y3 - yy4) * (y3 - yy4));
                float distance_xx1_x4 = sqrt((x4 - xx1) * (x4 - xx1) + (y4 - yy1) * (y4 - yy1));
                float distance_xx2_x4 = sqrt((x4 - xx2) * (x4 - xx2) + (y4 - yy2) * (y4 - yy2));
                float distance_xx3_x4 = sqrt((x4 - xx3) * (x4 - xx3) + (y4 - yy3) * (y4 - yy3));
                float distance_xx4_x4 = sqrt((x4 - xx4) * (x4 - xx4) + (y4 - yy4) * (y4 - yy4));
                float distance_xx1_xx2 = sqrt((xx1 - xx2) * (xx1 - xx2) + (yy1 - yy2) * (yy1 - yy2));
                float distance_xx1_xx3 = sqrt((xx1 - xx3) * (xx1 - xx3) + (yy1 - yy3) * (yy1 - yy3));
                float distance_xx4_xx2 = sqrt((xx4 - xx2) * (xx4 - xx2) + (yy4 - yy2) * (yy4 - yy2));
                float distance_xx3_xx4 = sqrt((xx3 - xx4) * (xx3 - xx4) + (yy3 - yy4) * (yy3 - yy4));
                float distance_x1_x2 = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
                float distance_x1_x3 = sqrt((x1 - x3) * (x1 - x3) + (y1 - y3) * (y1 - y3));
                float distance_x4_x2 = sqrt((x4 - x2) * (x4 - x2) + (y4 - y2) * (y4 - y2));
                float distance_x3_x4 = sqrt((x3 - x4) * (x3 - x4) + (y3 - y4) * (y3 - y4));
                if (distance_xx1_x1 + distance_xx2_x1 - distance_xx1_xx2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x1 + distance_xx3_x1 - distance_xx1_xx3 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx4_x1 + distance_xx2_x1 - distance_xx4_xx2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx3_x1 + distance_xx4_x1 - distance_xx3_xx4 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x2 + distance_xx2_x2 - distance_xx1_xx2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x2 + distance_xx3_x2 - distance_xx1_xx3 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx4_x2 + distance_xx2_x2 - distance_xx4_xx2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx3_x2 + distance_xx4_x2 - distance_xx3_xx4 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x3 + distance_xx2_x3 - distance_xx1_xx2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x3 + distance_xx3_x3 - distance_xx1_xx3 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx4_x3 + distance_xx2_x3 - distance_xx4_xx2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx3_x3 + distance_xx4_x3 - distance_xx3_xx4 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x4 + distance_xx2_x4 - distance_xx1_xx2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x4 + distance_xx3_x4 - distance_xx1_xx3 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx4_x4 + distance_xx2_x4 - distance_xx4_xx2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx3_x4 + distance_xx4_x4 - distance_xx3_xx4 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x1 + distance_xx1_x2 - distance_x1_x2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x1 + distance_xx1_x3 - distance_x1_x3 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x4 + distance_xx1_x2 - distance_x4_x2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx1_x3 + distance_xx1_x4 - distance_x3_x4 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx2_x1 + distance_xx2_x2 - distance_x1_x2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx2_x1 + distance_xx2_x3 - distance_x1_x3 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx2_x4 + distance_xx2_x2 - distance_x4_x2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx2_x3 + distance_xx2_x4 - distance_x3_x4 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx3_x1 + distance_xx3_x2 - distance_x1_x2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx3_x1 + distance_xx3_x3 - distance_x1_x3 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx3_x4 + distance_xx3_x2 - distance_x4_x2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx3_x3 + distance_xx3_x4 - distance_x3_x4 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx4_x1 + distance_xx4_x2 - distance_x1_x2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx4_x1 + distance_xx4_x3 - distance_x1_x3 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx4_x4 + distance_xx4_x2 - distance_x4_x2 < collision_distance)
                {
                    flag = 1;
                    break;
                }
                if (distance_xx4_x3 + distance_xx4_x4 - distance_x3_x4 < collision_distance)
                {
                    flag = 1;
                    break;
                }
            }
        }
        else
        {
            glow = 1;
        }

        if ((speed_of_background * (glfwGetTime() - make_time_zero)) > distance_between_zappers * (number_of_zappers + 4))
        {
            if (flag != 1)
            {
                if (level == number_of_levels)
                {
                    flag = 2;
                }
                else
                {
                    flag = 3;
                }
            }
        }

        if (flag == 0)
        {
            glm::vec3 translation_of_background_vector = glm::vec3((speed_of_background * -1 * ((glfwGetTime() - make_time_zero)) + sub * 2), 0.0f, 0.0f);
            glm::mat4 transformation_of_background = glm::mat4(1.0f);
            transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
            int transLoc = glGetUniformLocation(shaderProgram, "trans");
            glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

            glBindTexture(GL_TEXTURE_2D, texture_background);
            glBindVertexArray(vao_background);
            glDrawElements(GL_TRIANGLES, sizeof(indices_of_background) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            int coin_number = 0;
            while (coin_number * size_of_coin < distance_between_zappers * (number_of_zappers + 3))
            {
                if (isalive[coin_number] == true)
                {
                    translation_of_background_vector = glm::vec3(coin_number * size_of_coin + (speed_of_background * -1 * (glfwGetTime() - make_time_zero)), ytrans[coin_number], 0.0f);
                    transformation_of_background = glm::mat4(1.0f);
                    transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                    transLoc = glGetUniformLocation(shaderProgram, "trans");
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

                    float x = init_player_x + size_of_player;
                    float xx = vertices_of_coins[0] + coin_number * size_of_coin + (speed_of_background * -1 * (glfwGetTime() - make_time_zero));
                    if (xx <= x)
                    {
                        float y1 = init_player_y + displacement;
                        float y2 = init_player_y + displacement + size_of_player;
                        float yy1 = ytrans[coin_number] + vertices_of_coins[1];
                        float yy2 = ytrans[coin_number] + vertices_of_coins[1] + size_of_coin;
                        if ((yy1 <= y2 && yy1 >= y1) || (yy2 <= y2 && yy2 >= y1))
                        {
                            isalive[coin_number] = false;
                            coin_number++;
                            score++;
                            continue;
                        }
                    }

                    if (coins_number == 1)
                    {
                        glBindTexture(GL_TEXTURE_2D, texture_coins_1);
                    }
                    if (coins_number == 2)
                    {
                        glBindTexture(GL_TEXTURE_2D, texture_coins_2);
                    }
                    if (coins_number == 3)
                    {
                        glBindTexture(GL_TEXTURE_2D, texture_coins_3);
                    }
                    if (coins_number == 4)
                    {
                        glBindTexture(GL_TEXTURE_2D, texture_coins_4);
                    }
                    if (coins_number == 5)
                    {
                        glBindTexture(GL_TEXTURE_2D, texture_coins_5);
                    }
                    if (coins_number == 6)
                    {
                        glBindTexture(GL_TEXTURE_2D, texture_coins_6);
                    }
                    glBindVertexArray(vao_coins);
                    glDrawElements(GL_TRIANGLES, sizeof(indices_of_coins) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }
                coin_number++;
            }

            translation_of_background_vector = glm::vec3((speed_of_background * -1 * (glfwGetTime() - make_time_zero)), 0.0f, 0.0f);
            transformation_of_background = glm::mat4(1.0f);
            transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
            transLoc = glGetUniformLocation(shaderProgram, "trans");
            glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

            glBindTexture(GL_TEXTURE_2D, texture_zapper);
            glBindVertexArray(vao_zappers);
            glDrawElements(GL_TRIANGLES, sizeof(indices_of_zappers) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            if (level > 2)
            {
                translation_of_background_vector = glm::vec3((speed_of_background * -1 * (glfwGetTime() - make_time_zero)), 0.0f, 0.0f);
                transformation_of_background = glm::mat4(1.0f);
                transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans, glm::vec3(-1 * (distance_between_zappers * number_of_zappers + distance_between_zappers * 2), 0.0f, 0.0f));
                glm::mat4 rotate = glm::mat4(1.0f);
                rotate = glm::rotate(rotate, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 trans2 = glm::mat4(1.0f);
                trans2 = glm::translate(trans2, glm::vec3((distance_between_zappers * number_of_zappers + distance_between_zappers * 2), 0.0f, 0.0f));
                transformation_of_background = transformation_of_background * trans2 * rotate * trans;
                transLoc = glGetUniformLocation(shaderProgram, "trans");
                glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

                glBindTexture(GL_TEXTURE_2D, texture_zapper);
                glBindVertexArray(vao_moving_zapper);
                glDrawElements(GL_TRIANGLES, sizeof(indices_of_moving_zapper) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }

            translation_of_background_vector = glm::vec3(0.0f, displacement, 0.0f);
            transformation_of_background = glm::mat4(1.0f);
            transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
            transLoc = glGetUniformLocation(shaderProgram, "trans");
            glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

            if (walking_number == 1 || walking_number == 3)
            {
                glBindTexture(GL_TEXTURE_2D, texture_player_1);
            }
            if (walking_number == 2 || walking_number == 4)
            {
                glBindTexture(GL_TEXTURE_2D, texture_player_2);
            }
            if (walking_number == 5)
            {
                glBindTexture(GL_TEXTURE_2D, texture_player_5);
            }
            if (glow == 1)
            {
                transLoc = glGetUniformLocation(shaderProgram, "glow");
                glUniform1f(transLoc, glow);
                glm::vec4 coords = transformation_of_background * glm::vec4(init_player_x + size_of_player / 2, init_player_y + size_of_player / 2, 0.0f, 1.0f);
                glm::vec2 coord = glm::vec2(coords[0], coords[1]);
                transLoc = glGetUniformLocation(shaderProgram, "centre");
                glm::mat4 send = glm::mat4(1.0f);
                send[0][0] = coord[0];
                send[1][1] = coord[1];
                glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(send));
            }
            glBindVertexArray(vao_player);
            glDrawElements(GL_TRIANGLES, sizeof(indices_of_player) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            transLoc = glGetUniformLocation(shaderProgram, "glow");
            glUniform1f(transLoc, 0);

            RenderText("Score:", 10.0f, 575.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
            RenderText(to_string(score), 70.0f, 575.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
            RenderText("Level:", 230.0f, 550.0f, 1.0f, glm::vec3(0.3, 0.7f, 0.9f));
            RenderText(to_string(level), 350.0f, 550.0f, 1.0f, glm::vec3(0.3, 0.7f, 0.9f));
            RenderText("Lives:", 520.0f, 575.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
            RenderText(to_string(lives), 580.0f, 575.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
            glUseProgram(shaderProgram);
        }
        else if (flag == 1)
        {
            lives--;
            if (lives > 0)
            {
                flag = 0;

                glm::vec3 translation_of_background_vector = glm::vec3((speed_of_background * -1 * ((glfwGetTime() - make_time_zero)) + sub * 2), 0.0f, 0.0f);
                glm::mat4 transformation_of_background = glm::mat4(1.0f);
                transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                int transLoc = glGetUniformLocation(shaderProgram, "trans");
                glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

                glBindTexture(GL_TEXTURE_2D, texture_background);
                glBindVertexArray(vao_background);
                glDrawElements(GL_TRIANGLES, sizeof(indices_of_background) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                int coin_number = 0;
                while (coin_number * size_of_coin < distance_between_zappers * (number_of_zappers + 3))
                {
                    if (isalive[coin_number] == true)
                    {
                        translation_of_background_vector = glm::vec3(coin_number * size_of_coin + (speed_of_background * -1 * (glfwGetTime() - make_time_zero)), ytrans[coin_number], 0.0f);
                        transformation_of_background = glm::mat4(1.0f);
                        transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                        transLoc = glGetUniformLocation(shaderProgram, "trans");
                        glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

                        float x = init_player_x + size_of_player;
                        float xx = vertices_of_coins[0] + coin_number * size_of_coin + (speed_of_background * -1 * (glfwGetTime() - make_time_zero));
                        if (xx <= x)
                        {
                            float y1 = init_player_y + displacement;
                            float y2 = init_player_y + displacement + size_of_player;
                            float yy1 = ytrans[coin_number] + vertices_of_coins[1];
                            float yy2 = ytrans[coin_number] + vertices_of_coins[1] + size_of_coin;
                            if ((yy1 <= y2 && yy1 >= y1) || (yy2 <= y2 && yy2 >= y1))
                            {
                                isalive[coin_number] = false;
                                coin_number++;
                                score++;
                                continue;
                            }
                        }

                        if (coins_number == 1)
                        {
                            glBindTexture(GL_TEXTURE_2D, texture_coins_1);
                        }
                        if (coins_number == 2)
                        {
                            glBindTexture(GL_TEXTURE_2D, texture_coins_2);
                        }
                        if (coins_number == 3)
                        {
                            glBindTexture(GL_TEXTURE_2D, texture_coins_3);
                        }
                        if (coins_number == 4)
                        {
                            glBindTexture(GL_TEXTURE_2D, texture_coins_4);
                        }
                        if (coins_number == 5)
                        {
                            glBindTexture(GL_TEXTURE_2D, texture_coins_5);
                        }
                        if (coins_number == 6)
                        {
                            glBindTexture(GL_TEXTURE_2D, texture_coins_6);
                        }
                        glBindVertexArray(vao_coins);
                        glDrawElements(GL_TRIANGLES, sizeof(indices_of_coins) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
                        glBindVertexArray(0);
                    }
                    coin_number++;
                }

                translation_of_background_vector = glm::vec3((speed_of_background * -1 * (glfwGetTime() - make_time_zero)), 0.0f, 0.0f);
                transformation_of_background = glm::mat4(1.0f);
                transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                transLoc = glGetUniformLocation(shaderProgram, "trans");
                glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

                glBindTexture(GL_TEXTURE_2D, texture_zapper);
                glBindVertexArray(vao_zappers);
                glDrawElements(GL_TRIANGLES, sizeof(indices_of_zappers) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                translation_of_background_vector = glm::vec3(0.0f, displacement, 0.0f);
                transformation_of_background = glm::mat4(1.0f);
                transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                transLoc = glGetUniformLocation(shaderProgram, "trans");
                glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

                if (walking_number == 1 || walking_number == 3)
                {
                    glBindTexture(GL_TEXTURE_2D, texture_player_1);
                }
                if (walking_number == 2 || walking_number == 4)
                {
                    glBindTexture(GL_TEXTURE_2D, texture_player_2);
                }
                if (walking_number == 5)
                {
                    glBindTexture(GL_TEXTURE_2D, texture_player_5);
                }
                glBindVertexArray(vao_player);
                glDrawElements(GL_TRIANGLES, sizeof(indices_of_player) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                RenderText("Score:", 10.0f, 575.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
                RenderText(to_string(score), 70.0f, 575.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
                RenderText("Level:", 230.0f, 550.0f, 1.0f, glm::vec3(0.3, 0.7f, 0.9f));
                RenderText(to_string(level), 350.0f, 550.0f, 1.0f, glm::vec3(0.3, 0.7f, 0.9f));
                RenderText("Lives:", 520.0f, 575.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
                RenderText(to_string(lives), 580.0f, 575.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
                glUseProgram(shaderProgram);
                invincibility_time_starts = glfwGetTime();
            }
            else
            {
                glm::vec3 translation_of_background_vector = glm::vec3((speed_of_background * -1 * ((glfwGetTime() - make_time_zero)) + sub * 2), 0.0f, 0.0f);
                glm::mat4 transformation_of_background = glm::mat4(1.0f);
                transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                int transLoc = glGetUniformLocation(shaderProgram, "trans");
                glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

                glBindTexture(GL_TEXTURE_2D, texture_background);
                glBindVertexArray(vao_background);
                glDrawElements(GL_TRIANGLES, sizeof(indices_of_background) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                translation_of_background_vector = glm::vec3(0.0f, 0.0f, 0.0f);
                transformation_of_background = glm::mat4(1.0f);
                transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
                transLoc = glGetUniformLocation(shaderProgram, "trans");
                glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

                glBindTexture(GL_TEXTURE_2D, texture_youloose);
                glBindVertexArray(vao_youloose);
                glDrawElements(GL_TRIANGLES, sizeof(indices_of_youloose) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

                RenderText("Score:", 220.0f, 200.0f, 1.0f, glm::vec3(0.3, 0.7f, 0.9f));
                RenderText(to_string(score), 340.0f, 200.0f, 1.0f, glm::vec3(0.3, 0.7f, 0.9f));
                glUseProgram(shaderProgram);
                glBindVertexArray(0);
            }
        }
        else if (flag == 2)
        {
            glm::vec3 translation_of_background_vector = glm::vec3((speed_of_background * -1 * ((glfwGetTime() - make_time_zero)) + sub * 2), 0.0f, 0.0f);
            glm::mat4 transformation_of_background = glm::mat4(1.0f);
            transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
            int transLoc = glGetUniformLocation(shaderProgram, "trans");
            glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

            glBindTexture(GL_TEXTURE_2D, texture_background);
            glBindVertexArray(vao_background);
            glDrawElements(GL_TRIANGLES, sizeof(indices_of_background) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            translation_of_background_vector = glm::vec3(0.0f, 0.0f, 0.0f);
            transformation_of_background = glm::mat4(1.0f);
            transformation_of_background = glm::translate(transformation_of_background, translation_of_background_vector);
            transLoc = glGetUniformLocation(shaderProgram, "trans");
            glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transformation_of_background));

            glBindTexture(GL_TEXTURE_2D, texture_youwin);
            glBindVertexArray(vao_youloose);
            glDrawElements(GL_TRIANGLES, sizeof(indices_of_youloose) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

            RenderText("Score:", 220.0f, 200.0f, 1.0f, glm::vec3(0.3, 0.7f, 0.9f));
            RenderText(to_string(score), 340.0f, 200.0f, 1.0f, glm::vec3(0.3, 0.7f, 0.9f));
            glUseProgram(shaderProgram);
            glBindVertexArray(0);
        }
        else
        {
            break;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (space_pressed == false)
            {
                time = (glfwGetTime() - make_time_zero);
                player_y = displacement;
                acceleration = jetpack_acceleration;
                space_pressed = true;
                walking_number = 5;
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        {
            if (space_pressed == true)
            {
                time = (glfwGetTime() - make_time_zero);
                player_y = displacement;
                acceleration = gravity_acceleration;
                space_pressed = false;
                walking_number = 1;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao_background);
    glDeleteBuffers(1, &ebo_background);

    glDeleteVertexArrays(1, &vao_zappers);
    glDeleteBuffers(1, &ebo_zappers);

    glDeleteVertexArrays(1, &vao_player);
    glDeleteBuffers(1, &ebo_player);

    glDeleteVertexArrays(1, &vao_coins);
    glDeleteBuffers(1, &ebo_coins);
    return;
}

int main()
{
    srand(time(0));
    int number_of_levels = 5;
    int level = 1;
    float speed_of_background = 1.0f;
    int number_of_zappers = 10;
    int lives = 4;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int vertexShader_text = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader_text, 1, &vertexShaderSource_text, NULL);
    glCompileShader(vertexShader_text);
    int success_text;
    char infoLog_text[512];
    glGetShaderiv(vertexShader_text, GL_COMPILE_STATUS, &success_text);
    if (!success_text)
    {
        glGetShaderInfoLog(vertexShader_text, 512, NULL, infoLog_text);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog_text << std::endl;
    }

    unsigned int fragmentShader_text = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader_text, 1, &fragmentShaderSource_text, NULL);
    glCompileShader(fragmentShader_text);
    glGetShaderiv(fragmentShader_text, GL_COMPILE_STATUS, &success_text);
    if (!success_text)
    {
        glGetShaderInfoLog(fragmentShader_text, 512, NULL, infoLog_text);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog_text << std::endl;
    }
    shaderProgram_text = glCreateProgram();
    glAttachShader(shaderProgram_text, vertexShader_text);
    glAttachShader(shaderProgram_text, fragmentShader_text);
    glLinkProgram(shaderProgram_text);
    glGetProgramiv(shaderProgram_text, GL_LINK_STATUS, &success_text);
    if (!success_text)
    {
        glGetProgramInfoLog(shaderProgram_text, 512, NULL, infoLog_text);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog_text << std::endl;
    }
    glDeleteShader(vertexShader_text);
    glDeleteShader(fragmentShader_text);

    for (int i = 0; i < number_of_levels; i++)
    {
        jetpack_joyrider(level, speed_of_background, number_of_zappers, window, shaderProgram, number_of_levels, lives);
        level++;
        speed_of_background += 0.2f;
        number_of_zappers += 10;
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
    glUseProgram(shaderProgram_text);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_text, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shaderProgram_text, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao_text);
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_text);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
// special effects