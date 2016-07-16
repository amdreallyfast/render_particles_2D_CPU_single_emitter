#pragma once

#include <map>
#include <vector>
#include <string>

// usually I avoid including this because it is so large, but the interested party must have 
// access to GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.
#include "glload/include/glload/gl_4_4.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Handles the assembly, storage, and retrieval of different shader programs.  The term 
    "storage" does not imply assembly, but the main functionality of this class after startup 
    will be storage and retrieval of shader program ID, so "storage" seemed to be an appropriate 
    description.
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
class ShaderStorage
{
public:
    ShaderStorage();
    ~ShaderStorage();
    void NewShader(const std::string &programKey);
    void DeleteProgram(const std::string &programKey);

    void AddShaderFile(const std::string &programKey, const std::string &filePath,
        const GLenum shaderType);
    GLuint LinkShader(const std::string &programKey);
    GLuint GetShaderProgram(const std::string &programKey) const;
    GLuint GetUniformLocation(const std::string &programKey,
        const std::string &uniformName) const;
    GLuint GetAttributeLocation(const std::string &programKey,
        const std::string &attributeName) const;

private:
    typedef std::map<std::string, GLuint> _PROGRAM_MAP;
    _PROGRAM_MAP _compiledPrograms;

    // before a shader program is compiled, it is a collection of binaries, so each shader 
    // program can have multiple binaries
    // Note: The typedefs make typing easier when checking for iterator 
    typedef std::map<std::string, std::vector<GLuint>> _BINARY_MAP;
    _BINARY_MAP _shaderBinaries;
};