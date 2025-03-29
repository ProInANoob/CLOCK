import imgui
from imgui import core
import glfw
import OpenGL.GL as gl
from imgui.integrations.glfw import GlfwRenderer
import messages


def impl_glfw_init(window_name="minimal ImGui/GLFW3 example", width=1280, height=720): # Ithink ill need to put thss in main, and do while !done
    if not glfw.init():
        print("Could not initialize OpenGL context")
        exit(1)

    # OS X supports only forward-compatible core profiles from 3.2
    glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
    glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
    glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

    glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, gl.GL_TRUE)

    # Create a windowed mode window and its OpenGL context
    window = glfw.create_window(int(width), int(height), window_name, None, None)
    glfw.make_context_current(window)

    if not window:
        glfw.terminate()
        print("Could not initialize Window")
        exit(1)

    return window

class GUI(object):
    window = None

    def __init__(self):
        print("Run innit")
        super().__init__()
        self.backgroundColor = (0, 0, 0, 1)
        self.window = impl_glfw_init()
        gl.glClearColor(*self.backgroundColor)
        imgui.create_context()
        self.impl = GlfwRenderer(self.window)

        self.string = ""
        self.f = 0.5


        # setup font plsssss
        io = imgui.get_io()
        self.new_font = io.fonts.add_font_from_file_ttf(
            "./serverApp/Helvetica.ttf", 25, 
        )
        self.impl.refresh_font_texture()
        

    def loop(self,
                to_blue:messages.to_button_data,
                from_blue:messages.from_button_data,
                to_orange:messages.to_button_data, 
                from_orange:messages.from_button_data, 
                to_clock:messages.to_clock_data, 
                from_clock:messages.from_clock_data):
        
        glfw.poll_events()
        self.impl.process_inputs()
        imgui.new_frame()
        imgui.begin("Custom window", True)
        imgui.core.push_font(self.new_font)
        ##Okay so... 
        # start button
        # reset button
        # pause button remove when not running?
        # win buttons
        # tapin buttons ???? is that A thing??? IDK??? jasonnnnnnnnn???/
        #
        # keybinds.... yayyyyyyayyayay 
        #
 

        imgui.text("Hello, world!")

        if imgui.button("START"):
                print("STAERT")
                to_clock.startClock = 1
        
        if imgui.button("RESET"):
                print("RESERT")
                to_clock.reset = 1
                # also buttons shoud go to idle probably. 
        if to_clock.startClock or from_clock.running:

            if imgui.button("PAUSE"):
                    print("PAUSES")
                    to_clock.pause = 1
        
        
        _, self.string = imgui.input_text("A String", self.string, 256)

        _, self.f = imgui.slider_float("float", self.f, 0.25, 1.5)

        #imgui.show_test_window()
        imgui.core.pop_font()
        
        imgui.end()

        imgui.render()

        gl.glClearColor(*self.backgroundColor)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)
        self.impl.render(imgui.get_draw_data())
        glfw.swap_buffers(self.window)
    

        
        

        
        