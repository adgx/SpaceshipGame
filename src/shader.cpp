#include "shader.h"
#include "log.h"
#include "utils/utils.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <sys/stat.h>
#include <vector>
#include <map>

using std::ifstream;
using std::ios;
using std::string;

namespace SpaceEngine
{

    namespace Info 
    {
    	std::map<std::string, Type> extensions = {
    		{".vs",   Type::VERTEX},
    		{".vert", Type::VERTEX},
    		{"_vert.glsl", Type::VERTEX},
    		{".vert.glsl", Type::VERTEX },
    		{".gs",   Type::GEOMETRY},
    		{".geom", Type::GEOMETRY},
    		{ ".geom.glsl", Type::GEOMETRY },
    		{".tcs",  Type::TESS_CONTROL},
    		{ ".tcs.glsl",  Type::TESS_CONTROL },
    		{ ".tes",  Type::TESS_EVALUATION },
    		{".tes.glsl",  Type::TESS_EVALUATION},
    		{".fs",   Type::FRAGMENT},
    		{".frag", Type::FRAGMENT},
    		{"_frag.glsl", Type::FRAGMENT},
    		{".frag.glsl", Type::FRAGMENT},
    		{".cs",   Type::COMPUTE},
    		{ ".cs.glsl",   Type::COMPUTE }
    	};

        inline const char* typeToString(Type type)
        {
            switch(type)
            {
                case Type::VERTEX: return "VERTEX";
                case Type::GEOMETRY: return "GEOMETRY";
                case Type::TESS_CONTROL: return "TESS_CONTROL";
                case Type::TESS_EVALUATION: return "TESS_EVALUATION";
                case Type::FRAGMENT: return "FRAGMENT";
                case Type::COMPUTE: return "COMPUTE";
                default: return "Unknown";
            }
        }
    }


    ShaderProgram::ShaderProgram() : handle(0), linked(false) {}

    ShaderProgram::~ShaderProgram() {
        if (handle == 0) return;
    	detachAndDeleteShaderObjects();
        // Delete the program
        glDeleteProgram(handle);
    }

    void ShaderProgram::detachAndDeleteShaderObjects() {
    	// Detach and delete the shader objects (if they are not already removed)
    	GLint numShaders = 0;
    	glGetProgramiv(handle, GL_ATTACHED_SHADERS, &numShaders);
    	std::vector<GLuint> shaderNames(numShaders);
    	glGetAttachedShaders(handle, numShaders, NULL, shaderNames.data());
    	for (GLuint shader : shaderNames) {
    		glDetachShader(handle, shader);
    		glDeleteShader(shader);
    	}
    }

    int ShaderProgram::compileShader(const char *fileName) 
    {
        // Check the file name's extension to determine the shader type
        string ext = getExtension(fileName);
        Type type = Type::VERTEX;
    	auto it = Info::extensions.find(ext);
    	if (it != Info::extensions.end()) {
    		type = it->second;
    	} else {
    		SPACE_ENGINE_ERROR("Unrecognized extension: {}", ext);
    		return 0;
    	}

        // Pass the discovered shader type along
        int res = compileShader(fileName, type);
        SPACE_ENGINE_ASSERT(res, "Shader compiling error");
        return res;
    }

    string ShaderProgram::getExtension(const char *name) {
        string nameStr(name);

        size_t dotLoc = nameStr.find_last_of('.');
        if (dotLoc != string::npos) {
            std::string ext = nameStr.substr(dotLoc);
            if( ext == ".glsl" ) {

                size_t loc = nameStr.find_last_of('.', dotLoc - 1);
                if( loc == string::npos ) {
                    loc = nameStr.find_last_of('_', dotLoc - 1);
                }
                if( loc != string::npos ) {
                    return nameStr.substr(loc);
                }
            } else {
                return ext;
            }
        }
        return "";
    }

    //TODO: track the VERTEX and the FRAMENT shader.
    int ShaderProgram::compileShader(const char *fileName, Type type) {
        if (!Utils::fileExists(fileName)) {
            SPACE_ENGINE_ERROR("Shader: {} not found", fileName);
            return 0;
        }

        if (handle <= 0) {
            handle = glCreateProgram();
            if (handle == 0) {
                SPACE_ENGINE_ERROR("Unable to create shader program.");
                return 0;
            }
        }

        ifstream inFile(fileName, ios::in);
        if (!inFile) {
            SPACE_ENGINE_ERROR("Unable to open: {}", fileName);
            return 0;
        }

        SPACE_ENGINE_DEBUG("Compiling shader Name: {} Type: {}", fileName, Info::typeToString(type));
        // Get file contents
        std::stringstream code;
        code << inFile.rdbuf();
        inFile.close();

        int res = compileShader(code.str(), type, fileName);
        SPACE_ENGINE_ASSERT(res, "Shader compiling error");
        return res;
    }

    int ShaderProgram::compileShader(const string &source,
                                    Type type,
                                    const char *fileName) 
    {
        if (handle <= 0) 
        {
            handle = glCreateProgram();
            if (handle == 0) 
            {
                SPACE_ENGINE_ERROR("Unable to create shader program.");
                return 0;
            }
        }

        GLuint shaderHandle = glCreateShader(type);

        const char *c_code = source.c_str();
        glShaderSource(shaderHandle, 1, &c_code, NULL);

        // Compile the shader
        glCompileShader(shaderHandle);

        // Check for errors
        int result;
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
        if (GL_FALSE == result) {
            // Compile failed, get log
    		std::string msg;
    		if (fileName) {
    			msg = string(fileName) + ": shader compliation failed\n";
    		}
    		else {
    			msg = "Shader compilation failed.\n";
    		}

            int length = 0;
            glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
            if (length > 0) {
                std::string log(length, ' ');
                int written = 0;
                glGetShaderInfoLog(shaderHandle, length, &written, &log[0]);
    			msg += log;
            }
            SPACE_ENGINE_ERROR("{}", msg);
            return 0;
        } 
        else 
        {
            // Compile succeeded, attach shader
            glAttachShader(handle, shaderHandle);
        }

        if(type == Type::VERTEX)
        {
            isVSComp = true;    
        }

        if(type == Type::FRAGMENT)
        {
            isFSComp = true;
        }

        return 1;
    }

    int ShaderProgram::link() {
        if (linked) return 1;
        if (handle <= 0)
        {
            SPACE_ENGINE_ERROR("Program has not been compiled.");
            return 0;
        }

        if(!isVSComp)
        {
            SPACE_ENGINE_WARN("Warning: no Vertex shader were compiled");
        }
        else if(!isFSComp)
        {
            SPACE_ENGINE_WARN("Warning: no Fragment shader were compiled");
        }

        glLinkProgram(handle);
    	int status = 0;
    	std::string errString;
        SPACE_ENGINE_DEBUG("Linking shader");
    	glGetProgramiv(handle, GL_LINK_STATUS, &status);
    	if (GL_FALSE == status) {
    		// Store log and return false
    		int length = 0;
    		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
    		errString += "Program link failed:\n";
    		if (length > 0) {
    			std::string log(length,' ');
    			int written = 0;
    			glGetProgramInfoLog(handle, length, &written, &log[0]);
    			errString += log;
    		}
    	}
    	else {
    		reflectUniforms();
    		linked = true;
    	}
    
    	detachAndDeleteShaderObjects();

    	if( GL_FALSE == status ) 
        {
            SPACE_ENGINE_ERROR(errString);
            return 0;
        }
        return 1;
    }

    int ShaderProgram::use() {
        if (handle <= 0 || (!linked)){
            SPACE_ENGINE_DEBUG("Shader has not been linked");
            return 0;
        }
        glUseProgram(handle);
        return 1;
    }
    
    int ShaderProgram::getHandle() {
        return handle;
    }
    
    bool ShaderProgram::isLinked() {
        return linked;
    }
    
    void ShaderProgram::bindAttribLocation(GLuint location, const char *name) {
        glBindAttribLocation(handle, location, name);
    }
    
    void ShaderProgram::bindFragDataLocation(GLuint location, const char *name) {
        glBindFragDataLocation(handle, location, name);
    }
    
    void ShaderProgram::setUniform(const char *name, float x, float y, float z) {
        GLint loc = getUniformLocation(name);
        glUniform3f(loc, x, y, z);
    }
    
    void ShaderProgram::setUniform(const char *name, const glm::vec3 &v) {
        this->setUniform(name, v.x, v.y, v.z);
    }
    
    void ShaderProgram::setUniform(const char *name, const glm::vec4 &v) {
        GLint loc = getUniformLocation(name);
        glUniform4f(loc, v.x, v.y, v.z, v.w);
    }
    
    void ShaderProgram::setUniform(const char *name, const glm::vec2 &v) {
        GLint loc = getUniformLocation(name);
        glUniform2f(loc, v.x, v.y);
    }
    
    void ShaderProgram::setUniform(const char *name, const glm::mat4 &m) {
        GLint loc = getUniformLocation(name);
        glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
    }
    
    void ShaderProgram::setUniform(const char *name, const glm::mat3 &m) {
        GLint loc = getUniformLocation(name);
        glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
    }
    
    void ShaderProgram::setUniform(const char *name, float val) {
        GLint loc = getUniformLocation(name);
        glUniform1f(loc, val);
    }
    
    void ShaderProgram::setUniform(const char *name, int val) {
        GLint loc = getUniformLocation(name);
        glUniform1i(loc, val);
    }
    
    void ShaderProgram::setUniform(const char *name, GLuint val) {
        GLint loc = getUniformLocation(name);
        glUniform1ui(loc, val);
    }
    
    void ShaderProgram::setUniform(const char *name, bool val) {
        int loc = getUniformLocation(name);
        glUniform1i(loc, val);
    }
    
    void ShaderProgram::printActiveUniforms() {
        GLint nUniforms, size, location, maxLen;
        GLchar *name;
        GLsizei written;
        GLenum type;
    
        glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
        glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &nUniforms);
    
        name = new GLchar[maxLen];
    
        SPACE_ENGINE_DEBUG("Active uniforms:");
        SPACE_ENGINE_DEBUG("------------------------------------------------");
        for (GLint i = 0; i < nUniforms; ++i) {
            glGetActiveUniform(handle, i, maxLen, &written, &size, &type, name);
            location = glGetUniformLocation(handle, name);
            SPACE_ENGINE_DEBUG("{} {} ({})", location, name, getTypeString(type));
        }
    
        delete[] name;
    }

    void ShaderProgram::reflectUniforms() {
        GLint nUniforms, size, location, maxLen;
        GLchar *name;
        GLsizei written;
        GLenum type;
    
        glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
        glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &nUniforms);
    
        name = new GLchar[maxLen];
        if(nUniforms > 0) 
        {
            SPACE_ENGINE_DEBUG("Fetch Uniform information for the shader:");
        } 
        else
        {
            SPACE_ENGINE_DEBUG("No Uniform was found");
        }
            
        for (GLint i = 0; i < nUniforms; ++i) 
        {
            glGetActiveUniform(handle, i, maxLen, &written, &size, &type, name);
            location = glGetUniformLocation(handle, name);
            uniformsInfo[name] = UniformInfo{location, type, size};
            SPACE_ENGINE_DEBUG("Uniform information: name: {}, location:{}, type{}", name, location, type);
        }

        //subroutines
        //now only for vertex and fragment shader
        if(isVSComp)
            reflectionSubrroutines(Type::VERTEX);
        if(isFSComp)
            reflectionSubrroutines(Type::FRAGMENT);
    
        delete[] name;
    }

    void ShaderProgram::reflectionSubrroutines(Type shType)
    {
        //num of subroutines functions
        GLint numSubRoutines;
        glGetProgramStageiv(handle, shType,
                    GL_ACTIVE_SUBROUTINES,
                    &numSubRoutines);
        if(numSubRoutines == 0)
        {
            SPACE_ENGINE_DEBUG("Shader type: {} No subroutines found", static_cast<int>(shType));
            return;
        }

        
        SPACE_ENGINE_DEBUG("Subroutines are found");
        //nameSubroutineFunc - location
        std::unordered_map<std::string, GLuint> subroutinesInfo;
        //get the max n chars
        GLint len;
            glGetProgramStageiv(handle, shType,
                                   GL_ACTIVE_SUBROUTINE_MAX_LENGTH,
                                   &len);
        char* name = new char[len];
        auto flag = glGetError();
        
        //fill the map 
        for (int i = 0; i < numSubRoutines; ++i) 
        {
            
            GLsizei written;

            glGetActiveSubroutineName(handle,
  	                                   shType,
  	                                   i,
                                       len,
                                       &written,
                                       name);
            flag = glGetError();
            GLuint loc = glGetSubroutineIndex( handle,
                                               shType,
                                               name);
            SPACE_ENGINE_DEBUG("ActiveSubroutine: {} Loc: {}", name, i);
            SPACE_ENGINE_DEBUG("True Location: {}", loc);
            subroutinesInfo[name] = i;
        }
        
        delete[] name;
        
        //get the max n chars to identify the subroutine uniform
        glGetProgramStageiv(handle, shType,
                            GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH,
                            &len);
        name = new char[len];
        flag = glGetError();

        //get the number of subroutines uniform
        if(shType == Type::VERTEX)
        {
            //make this a function
            GLint numLocs;
            glGetProgramStageiv(handle, Type::VERTEX,
                    GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
                    &numLocs);
            auto flag = glGetError();
            
            for(int i = 0; i < numLocs; i++)
            {
                GLsizei written;
                glGetActiveSubroutineUniformName(handle,
  	                                            shType,
  	                                            i,
  	                                            len,
  	                                            &written,
  	                                            name);
            SPACE_ENGINE_DEBUG("ActiveSubroutineUniform: {} Loc: {}", name, i);
            subroutineUniformsInfo[shType][name] = i;
            
            GLuint loc = glGetSubroutineIndex( handle,
                                               shType,
                                               name);
            SPACE_ENGINE_DEBUG("True Location: {}", loc);
            }
            vsIdxSubRoutUniform.resize(numLocs);
            vsSubroutinesInfo = subroutinesInfo;
        }
        else if(shType == Type::FRAGMENT)
        {
            GLint numLocs;
            glGetProgramStageiv(handle, Type::FRAGMENT,
                    GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
                    &numLocs);
            auto flag = glGetError();
            for(int i = 0; i < numLocs; i++)
            {
                GLsizei written;
                glGetActiveSubroutineUniformName(handle,
  	                                            shType,
  	                                            i,
  	                                            len,
  	                                            &written,
  	                                            name);
                SPACE_ENGINE_DEBUG("ActiveSubroutineUniform: {} Loc: {}", name, i);
                GLint loc = glGetSubroutineUniformLocation(handle,
                                                    shType,
                                                    name);
                SPACE_ENGINE_DEBUG("True Location: {}", loc);
                subroutineUniformsInfo[shType][name] = loc;
            }
            fsIdxSubRoutUniform.resize(numLocs);
            fsSubroutinesInfo = subroutinesInfo;
        }
        delete[] name;
    }

    void ShaderProgram::setSubroutinesUniform(const char *name, const std::string& type)
    {
        //search on vs
        auto pos = vsSubroutinesInfo.find(name);

	    if (pos != vsSubroutinesInfo.end()) 
        {
            auto uniform = subroutineUniformsInfo[Type::VERTEX].find(type);
            if(uniform != subroutineUniformsInfo[Type::VERTEX].end())
	    	    vsIdxSubRoutUniform[subroutineUniformsInfo[Type::VERTEX][type]] = pos->second;
            else 
            {
                SPACE_ENGINE_FATAL("Not found the active subroutine uniform associated to the active subroutine");
                exit(-1);
            }
            return;
	    }
        
        //search on fs
        pos = fsSubroutinesInfo.find(name);

	    if (pos != fsSubroutinesInfo.end()) 
        {
	    	auto uniform = subroutineUniformsInfo[Type::FRAGMENT].find(type);
            if(uniform != subroutineUniformsInfo[Type::FRAGMENT].end())
	    	    fsIdxSubRoutUniform[subroutineUniformsInfo[Type::FRAGMENT][type]] = pos->second;
            else 
            {
                SPACE_ENGINE_FATAL("Not found the active subroutine uniform associated to the active subroutine");
                exit(-1);
            }
            return;
	    }
    }

    void ShaderProgram::bindSubroutines()
    {
        GL_CHECK_ERRORS();
        if(vsIdxSubRoutUniform.size() > 0){
            glUniformSubroutinesuiv(GL_VERTEX_SHADER, static_cast<GLsizei>(vsIdxSubRoutUniform.size()), vsIdxSubRoutUniform.data());
            GL_CHECK_ERRORS();
        }
        if(fsIdxSubRoutUniform.size() > 0)
        {
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, static_cast<GLsizei>(fsIdxSubRoutUniform.size()), fsIdxSubRoutUniform.data());
            GL_CHECK_ERRORS();
        }
    }

    
    void ShaderProgram::printActiveUniformBlocks() {
        GLint written, maxLength, maxUniLen, nBlocks, binding;
        GLchar *name;
    
        glGetProgramiv(handle, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxLength);
        glGetProgramiv(handle, GL_ACTIVE_UNIFORM_BLOCKS, &nBlocks);
        glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLen);
        GLchar *uniName = new GLchar[maxUniLen];
        name = new GLchar[maxLength];
    
        SPACE_ENGINE_DEBUG("Active Uniform blocks:");
        SPACE_ENGINE_DEBUG("------------------------------------------------\n");
        for (GLint i = 0; i < nBlocks; i++) {
            glGetActiveUniformBlockName(handle, i, maxLength, &written, name);
            glGetActiveUniformBlockiv(handle, i, GL_UNIFORM_BLOCK_BINDING, &binding);
            SPACE_ENGINE_DEBUG("Uniform block \"{}\" ({}):", name, binding);
        
            GLint nUnis;
            glGetActiveUniformBlockiv(handle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &nUnis);
            GLint *unifIndexes = new GLint[nUnis];
            glGetActiveUniformBlockiv(handle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, unifIndexes);
        
            for (int unif = 0; unif < nUnis; ++unif) {
                GLuint uniIndex = unifIndexes[unif];
                GLint size;
                GLenum type;
            
                glGetActiveUniform(handle, uniIndex, maxUniLen, &written, &size, &type, uniName);
                SPACE_ENGINE_DEBUG("    {} ({})", name, getTypeString(type));
            }
        
            delete[] unifIndexes;
        }
        delete[] name;
        delete[] uniName;
    }

    int ShaderProgram::isPresentUniform(const char *name)
    {
	    if (uniformsInfo.find(name) == uniformsInfo.end()) 
        {
	    	return 0;
	    }

        return 1;
    }
    
    void ShaderProgram::printActiveAttribs() {
        GLint written, size, location, maxLength, nAttribs;
        GLenum type;
        GLchar *name;
    
        glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
        glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &nAttribs);
    
        name = new GLchar[maxLength];
        SPACE_ENGINE_DEBUG("Active Attributes:");
        SPACE_ENGINE_DEBUG("------------------------------------------------");
        for (int i = 0; i < nAttribs; i++) {
            glGetActiveAttrib(handle, i, maxLength, &written, &size, &type, name);
            location = glGetAttribLocation(handle, name);
            SPACE_ENGINE_DEBUG(" {} {} ({})", location, name, getTypeString(type));
        }
        delete[] name;
    }
    
    const char *ShaderProgram::getTypeString(GLenum type) {
        // There are many more types than are covered here, but
        // these are the most common in these examples.
        switch (type) {
            case GL_FLOAT:
                return "float";
            case GL_FLOAT_VEC2:
                return "vec2";
            case GL_FLOAT_VEC3:
                return "vec3";
            case GL_FLOAT_VEC4:
                return "vec4";
            case GL_DOUBLE:
                return "double";
            case GL_INT:
                return "int";
            case GL_UNSIGNED_INT:
                return "unsigned int";
            case GL_BOOL:
                return "bool";
            case GL_FLOAT_MAT2:
                return "mat2";
            case GL_FLOAT_MAT3:
                return "mat3";
            case GL_FLOAT_MAT4:
                return "mat4";
            case GL_SAMPLER_2D:
                return "sampler2D";
            default:
                return "?";
        }
    }
    
    int ShaderProgram::validate() {
        if (!isLinked())
        {
            SPACE_ENGINE_ERROR("Program is not linked");
            return 0;
        }
            
    
        GLint status;
        glValidateProgram(handle);
        glGetProgramiv(handle, GL_VALIDATE_STATUS, &status);
    
        if (GL_FALSE == status) {
            // Store log and return false
            int length = 0;
            string logString;
        
            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
        
            if (length > 0) {
                char *c_log = new char[length];
                int written = 0;
                glGetProgramInfoLog(handle, length, &written, c_log);
                logString = c_log;
                delete[] c_log;
            }
        
            SPACE_ENGINE_ERROR("Program failed to validate\n{}", logString);
            return 0;
        }
        return 1;
    }
    //ShaderManager
    std::unordered_map<std::string, ShaderProgram*> ShaderManager::shadersMap;

    ShaderProgram* ShaderManager::createShaderProgram(const std::string nameFile)
    {
        ShaderProgram* pSP = new ShaderProgram();
        std::vector<std::filesystem::path> shaderFiles;

        if (std::filesystem::exists(SHADERS_PATH)) {
            for (const auto& entry : std::filesystem::directory_iterator(SHADERS_PATH)) 
            {
                if (!entry.is_regular_file())
                    continue;

                const auto& p = entry.path();
                if (p.stem() == nameFile) {
                    shaderFiles.push_back(p);
                }
            }
        }
        else
        {
             SPACE_ENGINE_ERROR("CRITICAL: Shader folder not found at {}", SHADERS_PATH);
        }

        if(shaderFiles.empty())
        {
            SPACE_ENGINE_ERROR("Shader file not found, name: {}", nameFile);
            delete pSP;
            return nullptr;
        }

        
        for(const std::filesystem::path& path : shaderFiles)
        {
            // FIX: path.string() per evitare problemi
            pSP->compileShader(path.string().c_str());
        }
        pSP->link();

        shadersMap[nameFile] = pSP;
        
        return pSP;
    }
    
    ShaderProgram* ShaderManager::findShaderProgram(const std::string nameShader)
    {
        auto pos = shadersMap.find(nameShader);

        if(pos == shadersMap.end())
        {
            SPACE_ENGINE_ERROR("ShaderProgram not found, name: {}", nameShader);
            return nullptr;
        }

        return pos->second;

    }

    void ShaderManager::Initialize()
    {
        GL_CHECK_ERRORS();
        createShaderProgram("simple");
        GL_CHECK_ERRORS();
        createShaderProgram("ui");
        GL_CHECK_ERRORS();
        createShaderProgram("uiButton");
        GL_CHECK_ERRORS();
        createShaderProgram("simpleTex");
        GL_CHECK_ERRORS();
        createShaderProgram("pbr");
        GL_CHECK_ERRORS();
        createShaderProgram("skybox");
        GL_CHECK_ERRORS();        
        createShaderProgram("glyphs");
        GL_CHECK_ERRORS();
    }

    void ShaderManager::Shutdown()
    {
        for (auto& [name, pTex] : shadersMap)
            delete pTex;
        shadersMap.clear();
    }

    std::vector<std::tuple<const std::string, GLenum>> ShaderProgram::getPairUniformNameLocation()
    {
        std::vector<std::tuple<const std::string, GLenum>> listUniform;

        for( const auto& [key, value] : uniformsInfo)
        {
            listUniform.push_back(std::tuple<const std::string, GLenum>{key, value.type});
        }

        return listUniform;
    }
}