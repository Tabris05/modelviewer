#include "dbg.h"
#ifndef NDEBUG
    void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
        // ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    
        std::cout << "---------------" << std::endl;
        std::cout << "Debug message (" << id << "): " << message << std::endl;
    
        switch (source) {
        case GL_DEBUG_SOURCE_API:
            print("Source: API");
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            print("Source: Window System");
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            print("Source: Shader Compiler");
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            print("Source: Third Party");
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            print("Source: Application");
            break;
        case GL_DEBUG_SOURCE_OTHER:
            print("Source: Other");
            break;
        }
    
        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:
            print("Type: Error");
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            print("Type: Deprecated Behaviour");
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            print("Type: Undefined Behaviour");
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            print("Type: Portability");
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            print("Type: Performance");
            break;
        case GL_DEBUG_TYPE_MARKER:
            print("Type: Marker");
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            print("Type: Push Group");
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            print("Type: Pop Group");
            break;
        case GL_DEBUG_TYPE_OTHER:
            print("Type: Other");
            break;
        }
    
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            print("Severity: high");
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            print("Severity: medium");
            break;
        case GL_DEBUG_SEVERITY_LOW:
            print("Severity: low");
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            print("Severity: notification");
            break;
        }
    }
#endif