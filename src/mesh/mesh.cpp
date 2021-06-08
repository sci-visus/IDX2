#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

int
main()
{
  /* create window and GL context via GLFW */
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* w = glfwCreateWindow(640, 480, "Sokol Triangle GLFW", 0, 0);
  glfwMakeContextCurrent(w);
  glfwSwapInterval(1);

  /* setup sokol_gfx */
  sg_desc SgDesc{};
  sg_setup(&SgDesc);

  /* a vertex buffer */
  const float vertices[] = {
    // positions            // colors
     0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
  };
  sg_buffer_desc SgBufferDesc{.data = SG_RANGE(vertices)};
  sg_buffer vbuf = sg_make_buffer(&SgBufferDesc);

  /* a shader */
  sg_shader_desc SgShaderDesc{
    .vs{
      .source =
        "#version 330\n"
        "layout(location=0) in vec4 position;\n"
        "layout(location=1) in vec4 color0;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  gl_Position = position;\n"
        "  color = color0;\n"
        "}\n"
    },
    .fs{
      .source =
        "#version 330\n"
        "in vec4 color;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "  frag_color = color;\n"
        "}\n"
    }
  };
  sg_shader shd = sg_make_shader(&SgShaderDesc);

  /* a pipeline state object (default render states are fine for triangle) */
  sg_pipeline_desc SgPipelineDesc{
    .shader = shd,
    .layout = {
      .attrs = {
        sg_vertex_attr_desc{.format=SG_VERTEXFORMAT_FLOAT3},
        sg_vertex_attr_desc{.format=SG_VERTEXFORMAT_FLOAT4}
      }
    }
  };
  sg_pipeline pip = sg_make_pipeline(&SgPipelineDesc);

  /* resource bindings */
  sg_bindings bind = {
    .vertex_buffers = { vbuf }
  };

  /* default pass action (clear to grey) */
  sg_pass_action pass_action = {0};

  /* draw loop */
  while (!glfwWindowShouldClose(w)) {
    int cur_width, cur_height;
    glfwGetFramebufferSize(w, &cur_width, &cur_height);
    sg_begin_default_pass(&pass_action, cur_width, cur_height);
    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);
    sg_draw(0, 3, 1);
    sg_end_pass();
    sg_commit();
    glfwSwapBuffers(w);
    glfwPollEvents();
  }

  /* cleanup */
  sg_shutdown();
  glfwTerminate();
  return 0;
}
