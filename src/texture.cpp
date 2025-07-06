// 独立实现
#include "texture.hpp"
#include <unordered_map>

std::unordered_map<std::string, std::shared_ptr<Texture2D>> Texture2D::texture2d_map;

Texture::~Texture() {}