#define GL_SILENCE_DEPRECATION

#include "ShaderProgram.h"

void ShaderProgram::load(const char *vertex_shader_file, const char *fragment_shader_file) {
    
    
    m_vertex_shader = load_shader_from_file(vertex_shader_file, GL_VERTEX_SHADER);

    m_fragment_shader = load_shader_from_file(fragment_shader_file, GL_FRAGMENT_SHADER);
    
   
    m_program_id = glCreateProgram();
    glAttachShader(m_program_id, m_vertex_shader);
    glAttachShader(m_program_id, m_fragment_shader);
    glLinkProgram(m_program_id);
    
    GLint link_success;
    glGetProgramiv(m_program_id, GL_LINK_STATUS, &link_success);
    
    if(link_success == GL_FALSE)
    {
        printf("Error linking shader program!\n");
    }
    
    m_model_matrix_uniform      = glGetUniformLocation(m_program_id, "modelMatrix");
    m_projection_matrix_uniform = glGetUniformLocation(m_program_id, "projectionMatrix");
    m_view_matrix_uniform       = glGetUniformLocation(m_program_id, "viewMatrix");
    m_colour_uniform            = glGetUniformLocation(m_program_id, "color");
    
    m_position_attribute  = glGetAttribLocation(m_program_id, "position");
    m_tex_coord_attribute = glGetAttribLocation(m_program_id, "texCoord");
    
    set_colour(1.0f, 1.0f, 1.0f, 1.0f);
    
}

void ShaderProgram::cleanup()
{
    glDeleteProgram(m_program_id);
    glDeleteShader(m_vertex_shader);
    glDeleteShader(m_fragment_shader);
}

GLuint ShaderProgram::load_shader_from_file(const std::string &shaderFile, GLenum type)
{
   
    std::ifstream infile(shaderFile);
    
    if(infile.fail()) {
        std::cout << "Error opening shader file:" << shaderFile << std::endl;
    }
    
    
    std::stringstream buffer;
    buffer << infile.rdbuf();
    
    
    return load_shader_from_string(buffer.str(), type);
}

GLuint ShaderProgram::load_shader_from_string(const std::string &shaderContents, GLenum type)
{
    
    GLuint shaderID = glCreateShader(type);
    
    
    const char *shader_string  = shaderContents.c_str();
    GLint shader_string_length = (GLint) shaderContents.size();
    

    glShaderSource(shaderID, 1, &shader_string, &shader_string_length);
    glCompileShader(shaderID);
    
    
    GLint compile_success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compile_success);
    
  
    if (compile_success == GL_FALSE)
    {
        GLchar messages[512];
        glGetShaderInfoLog(shaderID, sizeof(messages), 0, &messages[0]);
        std::cout << messages << std::endl;
    }
    
   
    return shaderID;
}

void ShaderProgram::set_colour(float red, float green, float blue, float alpha)
{
    glUseProgram(m_program_id);
    glUniform4f(m_colour_uniform, red, green, blue, alpha);
}

void ShaderProgram::set_view_matrix(const glm::mat4 &matrix)
{
    glUseProgram(m_program_id);
    glUniformMatrix4fv(m_view_matrix_uniform, 1, GL_FALSE, &matrix[0][0]);
}

void ShaderProgram::set_model_matrix(const glm::mat4 &matrix)
{
    glUseProgram(m_program_id);
    glUniformMatrix4fv(m_model_matrix_uniform, 1, GL_FALSE, &matrix[0][0]);
}

void ShaderProgram::set_projection_matrix(const glm::mat4 &matrix)
{
    glUseProgram(m_program_id);
    glUniformMatrix4fv(m_projection_matrix_uniform, 1, GL_FALSE, &matrix[0][0]);
}
