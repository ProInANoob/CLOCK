import imgui
import imgui.core
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
    keys = None
    framecount = 0
    newframecount = 0

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
        self.io = imgui.get_io()
        self.new_font = self.io.fonts.add_font_from_file_ttf(
            "./serverApp/Helvetica.ttf", 25, 
        )
        self.impl.refresh_font_texture()
        

    def getKeys(self):
        #self.newframecount = self.io.frame_count
        #if( self.framecount != self.newframecount):
        #print("frame diff")

        self.framecount = self.newframecount
        keys = [0, 0, 0, 0, 0] # Otap, Btap, Start ,KO, Pause
        if glfw.get_key(self.window, glfw.KEY_SPACE) == glfw.PRESS and glfw.get_key(self.window, glfw.KEY_LEFT_CONTROL) == glfw.PRESS :
            print("CONTROLL SPACEEE!!!! ")
        elif glfw.get_key(self.window, glfw.KEY_SPACE) == glfw.PRESS:
                print("space press???")
        if glfw.get_key(self.window, glfw.KEY_F8) == glfw.PRESS and glfw.get_key(self.window, glfw.KEY_LEFT_CONTROL) == glfw.PRESS :
            print("this is orange tap")
            keys[0] = 1

        if glfw.get_key(self.window, glfw.KEY_F7) == glfw.PRESS and glfw.get_key(self.window, glfw.KEY_LEFT_CONTROL) == glfw.PRESS :
            print("this is blue tap")
            keys[1] = 1

        if glfw.get_key(self.window, glfw.KEY_F10) == glfw.PRESS and glfw.get_key(self.window, glfw.KEY_LEFT_CONTROL) == glfw.PRESS :
            print("this is match start")
            keys[2] = 1
            
        if glfw.get_key(self.window, glfw.KEY_F11) == glfw.PRESS and glfw.get_key(self.window, glfw.KEY_LEFT_CONTROL) == glfw.PRESS :
            print("this is KO. I have no clue ewitch one. this sucks. why would you do this too meeeee. ")
            keys[3] = 1

        if glfw.get_key(self.window, glfw.KEY_F12) == glfw.PRESS and glfw.get_key(self.window, glfw.KEY_LEFT_CONTROL) == glfw.PRESS :
            print("this is pause")
            keys[4] = 1

        return keys
        
                # or I could do callbacks butt uhhh yeah not foing that /// doooo yeahhh noooo.
        

    def loop(self,
                to_blue:messages.to_button_data,
                from_blue:messages.from_button_data,
                to_orange:messages.to_button_data, 
                from_orange:messages.from_button_data, 
                to_clock:messages.to_clock_data, 
                from_clock:messages.from_clock_data):
        
        glfw.poll_events()
        self.impl.process_inputs()
        keys = self.getKeys()
        if keys[0]:
            print("O tapin")
            to_clock.orangeTapin = 1
        if keys[1]:
            to_clock.blueTapin = 1
        if keys[2]:
            to_clock.startClock = 1
        if keys[3]:
            #KO. witch one JASONNNNNNNNN
            pass
        if keys[4]:
            to_clock.pause = 1


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
        
        #as ref.         
        #_, self.string = imgui.input_text("A String", self.string, 256)

        #_, self.f = imgui.slider_float("float", self.f, 0.25, 1.5)

        #imgui.show_test_window()
        imgui.core.pop_font()
        
        imgui.end()

        imgui.render()

        gl.glClearColor(*self.backgroundColor)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)
        self.impl.render(imgui.get_draw_data())
        glfw.swap_buffers(self.window)
    

        
        

        
        