#pragma once

#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>

class Shader {
public:
    Shader();
    ~Shader();
    
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromStrings(const std::string& vertexSource, const std::string& fragmentSource);
    void use();
    void cleanup();
    
    // Uniform setters
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat4(const std::string& name, const glm::mat4& value);
    
    bool isValid() const { return programID != 0; }
    GLuint getProgramID() const { return programID; }

private:
    bool compileShader(const std::string& source, GLenum type, GLuint& shaderID);
    bool linkProgram();
    void checkCompileErrors(GLuint shader, const std::string& type);
    
    GLuint programID;
    GLuint vertexShader, fragmentShader;
};
