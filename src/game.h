#pragma once
#ifndef GAME_H
#define GAME_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <limits>

#include "model.h"
#include "shader.h"
#include "asteroida.h"
using namespace std;

struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};


void renderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, GLuint *VAO, GLuint *VBO, std::map<GLchar, Character> *characters)
{

	// Activate corresponding render state	
	shader.use();
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(*VAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = (*characters)[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos,     ypos,       0.0, 1.0 },
		{ xpos + w, ypos,       1.0, 1.0 },

		{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos + w, ypos,       1.0, 1.0 },
		{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

class Menu {
public:
	enum options { NEW_GAME, EXIT };
private:	
	options selectedOption;
	glm::vec3 selectedColor;
	glm::vec3 notSelectedColor;
public:
	
	Menu() {
		selectedOption = NEW_GAME;
		selectedColor = glm::vec3(0.0f, 1.0f, 0.0f);
		notSelectedColor = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	void draw(Shader *textShader, float windowHeight, float windowWidth, GLuint *VAO, GLuint *VBO, std::map<GLchar, Character> *characters) {
		if (selectedOption == NEW_GAME) {
			renderText(*textShader, "NEW GAME", windowWidth / 2 - 340.0f, windowHeight / 2 + 60.0f, 2.0f, selectedColor, VAO, VBO, characters);
			renderText(*textShader, "EXIT", windowWidth / 2 - 340.0f, windowHeight / 2 - 60.0f, 2.0f, notSelectedColor, VAO, VBO, characters);
		}
		else {
			renderText(*textShader, "NEW GAME", windowWidth / 2 - 340.0f, windowHeight / 2 + 60.0f, 2.0f, notSelectedColor, VAO, VBO, characters);
			renderText(*textShader, "EXIT", windowWidth / 2 - 340.0f, windowHeight / 2 - 60.0f, 2.0f, selectedColor, VAO, VBO, characters);
		}		
	}

	void changeOptionUp() {
		if (selectedOption == EXIT) {
			selectedOption = NEW_GAME;
		}
	}

	void changeOptionDown() {
		if (selectedOption == NEW_GAME) {
			selectedOption = EXIT;
		}
	}
	options getSelectedOption(){
		return selectedOption;
	}
	
};


class Game {
public:
	enum gameStates { RUNNING, MENU, MENU_RUNNING, ENDED, CLOSED, NEXT_LEVEL};

private:
	
	Scene *currentScene;
	Menu *menu;
	Shader *textShader;
	unsigned int defaultShaderID;
	unsigned int reflexShaderID;
	unsigned int refractShaderID;
	int windowHeight;
	int windowWidth;
	gameStates gameState;
	Model *bulletModel;
	unsigned int bulletShaderID;
	GLuint *VAO;
	GLuint *VBO;
	int level;
	std::map<GLchar, Character> *characters;
	vector<std::string> skyboxFaces;
	

	
	
public:

	Game(Shader *textShader, unsigned int defaultShaderID, unsigned int reflexShaderID, unsigned int refractShaderID, unsigned int bulletShaderID, int windowWidth, int windowHeight,
		GLuint *VAO, GLuint *VBO, std::map<GLchar, Character> *characters) {
		this->textShader = textShader;
		this->windowWidth = windowWidth;
		this->windowHeight = windowHeight;
		this->defaultShaderID = defaultShaderID;
		this->reflexShaderID = reflexShaderID;
		this->refractShaderID = refractShaderID;
		this->bulletModel = new Model("res/models/asteroid/asteroid.obj");
		this->bulletShaderID = bulletShaderID;
		this->VAO = VAO;
		this->VBO = VBO;
		this->characters = characters;
		menu = new Menu();
		gameState = MENU;
	}

	void initialize() {
		currentScene = new Scene(defaultShaderID, reflexShaderID, refractShaderID, 2.0f, 0.5f);
		currentScene->generateAsteroids(20, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 150.0f, 180.0f);
		level = 1;
		skyboxFaces = {
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox\\right.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox\\left.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox\\top.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox\\bottom.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox\\front.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox\\back.jpg"
		};
	}

	void loadNextLevel() {
		delete currentScene;
		currentScene = new Scene(defaultShaderID, reflexShaderID, refractShaderID, 2.0f, 0.5f);
		currentScene->generateAsteroids(20, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 150.0f, 180.0f);
		level = 1;
		skyboxFaces = {
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox2\\right.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox2\\left.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox2\\top.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox2\\bottom.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox2\\front.jpg",
			"C:\\Users\\mattibu\\Desktop\\studia\\5sem\\PAGi\\OpenGLPAG\\res\\textures\\skybox2\\back.jpg"
		};

		gameState = RUNNING;
	}

	vector<std::string> getSkyboxFaces() {
		return skyboxFaces;
	}


	void drawGui(){
		renderText(*textShader, "Points: " + to_string(currentScene->getPoints()), 20, windowHeight - 20,0.5f, glm::vec3(1.0f, 1.0f, 0.0f), VAO, VBO, characters);
		renderText(*textShader, "Lives: " + to_string(currentScene->getLives()), 20, windowHeight - 80, 0.5f, glm::vec3(1.0f, 1.0f, 0.0f), VAO, VBO, characters);
	}

	void drawDeathScreen() {
		renderText(*textShader, "GAME OVER", windowWidth / 2 -340.0f, windowHeight / 2, 2.0f, glm::vec3(1.0f, 1.0f, 0.0f), VAO, VBO, characters);
		renderText(*textShader, "Points: " + to_string(currentScene->getPoints()), windowWidth / 2 - 300.0f, windowHeight / 2 - 80.0f, 2.0f, glm::vec3(1.0f, 1.0f, 0.0f), VAO, VBO, characters);

	}

	void drawNextLevelScreen() {
		renderText(*textShader, "NEXT LEVEL", windowWidth / 2 - 340.0f, windowHeight / 2, 2.0f, glm::vec3(1.0f, 1.0f, 0.0f), VAO, VBO, characters);
		renderText(*textShader, "Points: " + to_string(currentScene->getPoints()), windowWidth / 2 - 300.0f, windowHeight / 2 - 80.0f, 2.0f, glm::vec3(1.0f, 1.0f, 0.0f), VAO, VBO, characters);

	}


	void shoot(glm::vec3 position, glm::vec3 direction) {
		currentScene->shoot(bulletModel, bulletShaderID, position, direction, 25000.0f);
	}

	void changeMenuOptionUp() {
		menu->changeOptionUp();
	}
	void changeMenuOptionDown() {
		menu->changeOptionDown();
	}

	void selectMenuOption() {
		if (menu->getSelectedOption() == menu->NEW_GAME) {
			initialize();
			gameState = RUNNING;
		}

		if (menu->getSelectedOption() == menu->EXIT) {
			gameState = CLOSED;
		}
	}

	gameStates getGameState() {
		return gameState;
	}

	void setGameState(gameStates gameState) {
		this->gameState = gameState;
	}
		

	void play(float deltaTime, glm::vec3 playerPosition) {
		if (gameState == MENU || gameState == MENU_RUNNING) {
			menu->draw(textShader, windowHeight, windowWidth, VAO, VBO, characters);
		}

		if (gameState == ENDED) {
			drawDeathScreen();
		}

		if (gameState == NEXT_LEVEL) {
			drawNextLevelScreen();
		}

		if (gameState == RUNNING) {
			if (!currentScene->update(deltaTime, playerPosition)) {
				gameState = ENDED;
			}

			if (currentScene->getAsteroidNumber() == 0) {
				gameState = NEXT_LEVEL;
			}
			
			drawGui();
		}
		
	}




};
#endif