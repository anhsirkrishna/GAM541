#include <Windows.h>
#include <SDL.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include "EditorUI.h"
#include "GameDefs.h"
#include "ShaderProgram.h"
#include "GameObjectManager.h"
#include "InputManager.h"
#include "FrameRateController.h"
#include "ResourceManager.h"
#include "AudioManager.h"
#include "GameManager.h"
#include "GameObjectFactory.h"
#include "Matrix3D.h"
#include "GLQuad.h"
#include "Transform.h"
#include "Movement.h"
#include "Camera.h"
#include "Controller.h"
#include "Collider.h"
#include "MemoryManager.h"
#include "GraphicsManager.h"
#include "LuaManager.h"

/*
* Few default global values. These extern variables are declared in GameDefs.h
* so they can be accessed anywhere in the project
*/
unsigned int WORLD_WIDTH = 720;
unsigned int WORLD_HEIGHT = 480;

unsigned int WINDOW_WIDTH = 720;
unsigned int WINDOW_HEIGHT = 480;

unsigned int DEFAULT_FRAMERATE = 60;

/*
* Global managers. These extern variables are decaled in their respective 
* header files so they can be accessed anywhere in the project.
*/
GameObjectManager* p_game_obj_manager;
GameManager* p_game_manager;
InputManager* p_input_manager;
Editor* p_editor;
FrameRateController* p_framerate_controller;
ResourceManager* p_resource_manager;
AudioManager* p_audio_manager;
Camera* p_camera;
GraphicsManager* p_graphics_manager;
MemoryManager g_memory_manager;
LuaManager* p_lua_manager;

bool RUN_WITH_EDITOR = true;

/*
* Macro used to check for OpenGL errors.
* Use this often to ensure all the OpenGL calls go through cleanly 
* without any errors
*/
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SDL_Log("OpenGL error (at line Main.cpp:%d): %s\n", __LINE__, glewGetErrorString(err));} }

/*
* Function to create all the global managers
*/
void CreateManagers() {
	p_game_obj_manager = new GameObjectManager();
	p_game_manager = new GameManager();
	p_input_manager = new InputManager();
	p_framerate_controller = new FrameRateController(DEFAULT_FRAMERATE);
	p_resource_manager = new ResourceManager();
	p_audio_manager = new AudioManager();
	p_editor = new Editor();
	p_camera = new Camera(glm::vec3(0.0f, 0.0f, -262.0f));
	p_graphics_manager = new GraphicsManager();
	p_lua_manager = new LuaManager();
}

/*
* Cleanup the global managers
*/
void DeleteManagers() {
	delete p_graphics_manager;
	p_game_obj_manager->Cleanup();
	delete p_game_obj_manager;
	delete p_game_manager;
	delete p_input_manager;
	delete p_framerate_controller;
	delete p_resource_manager;
	delete p_audio_manager;
	delete p_editor;
	delete p_camera;
}

/*
* Function to Close the application
* Destroys the SDL window and quits SDL
* Frees the Console if debug mode
* Returns : void 
*/
void CloseProgram() {

	// Quit SDL subsystems
	SDL_Quit();

	#ifdef DEBUG
		FreeConsole();
	#endif // DEBUG
}


int main(int argc, char* args[])
{
	#ifdef DEBUG
		//This call creates a console window that SDL can log to for debugging.
		AllocConsole();
	#endif // DEBUG

	//Create all the global managers
	CreateManagers();
	
	if (RUN_WITH_EDITOR)
		p_editor->Init();

	//Create the Shader Program
	ShaderProgram* p_shader_program = p_graphics_manager->GetActiveShader();

	GameObjectFactory go_factory;
	go_factory.CreateLevel(0);

	// loads behavior scripts after all game objects have been created 
	// (components have an assigned parent)
	p_lua_manager->LoadBehaviorScripts();

	std::vector<GameObject*> new_go_list;
  
	while (p_game_manager->Status())
	{
		p_framerate_controller->start_game_loop();
		p_game_obj_manager->Update();
		p_lua_manager->Update();
		p_input_manager->Update();

		if (p_input_manager->isQuit())
			p_game_manager->Quit();

		//Test code for game object state management
		if (p_input_manager->isKeyPressed(SDL_SCANCODE_X)) {
			for (auto game_object : p_game_obj_manager->game_object_list) {
				if (game_object->HasComponent("CONTROLLER"))
					game_object->state_manager.ChangeState("WALK");
			}
		}
		if (p_input_manager->isKeyPressed(SDL_SCANCODE_Z)) {
			for (auto game_object : p_game_obj_manager->game_object_list) {
				if (game_object->HasComponent("CONTROLLER"))
					game_object->state_manager.ChangeState("IDLE");
			}
		}

		//The following bit of code should be moved into a GameStateManager or and individual game State
		p_shader_program->Use();
		p_graphics_manager->ClearBuffer(glm::vec4(0.3f));

		if (RUN_WITH_EDITOR)
			p_editor->NewFrame();

		//Redraw the scene every frame
		p_game_obj_manager->Draw(p_shader_program);
		p_shader_program->Unuse();

		//ImGui Render
		if (RUN_WITH_EDITOR)
			p_editor->Render();

		p_graphics_manager->SwapBuffers();
		p_framerate_controller->end_game_loop();
	}

	if (RUN_WITH_EDITOR)
		p_editor->Cleanup();

	DeleteManagers();
	CloseProgram();
	return 0;
}