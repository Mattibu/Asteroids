#pragma once
#ifndef ASTEROIDA_H
#define ASTEROIDA_H

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
using namespace std;

class CollidBox {
private:
	glm::vec3 colliderDimension;
	glm::vec3 minusCorner;
	glm::vec3 plusCorner;

public:

	CollidBox(glm::vec3 colliderDimension) {
		this->colliderDimension = colliderDimension;
	}
	glm::vec3 getMinusCorner() {
		return this->minusCorner;
	}

	glm::vec3 getPlusCorner() {
		return this->plusCorner;
	}

	bool isColiding(glm::vec3 coordinates, glm::vec3 colliderPoint ) {

		minusCorner = coordinates - 0.5f * colliderDimension;
		plusCorner = coordinates + 0.5f * colliderDimension;
		if (colliderPoint.x > this->minusCorner.x && colliderPoint.y > this->minusCorner.y && colliderPoint.z > this->minusCorner.z &&
			colliderPoint.x < this->plusCorner.x && colliderPoint.y < this->plusCorner.y && colliderPoint.z < this->plusCorner.z) {
			return true;
		}

		return false;
	}

};

class Asteroida {
public:
	static enum types {DEFAULT, REFLEX, REFRACT};
private:
	CollidBox * collider;
	Model *model;
	glm::mat4 transformMatrix;
	GraphNode *graphNode;
	glm::vec3 movementDirection;
	float speed;
	types type;

	float distanceFromCenter(glm::vec3 point) {
		float distance = 0;
		distance += point.x * point.x + point.y*point.y + point.z*point.z;
		return sqrt(distance);
	}		

public:
	Asteroida(Model *model, unsigned int shaderID, float speed, glm::mat4 transformMatrix, glm::vec3 movementDirection, glm::vec3 colliderDimensions, types type)
	{
		this->model = model;
		this->transformMatrix = transformMatrix;
		this->graphNode = new GraphNode(transformMatrix, new DrawModel(model, shaderID), shaderID);
		this->speed = speed;
		this->movementDirection = movementDirection;
		this->collider = new CollidBox(colliderDimensions);
		this->type = type;
	}

	bool isColiding(glm::vec3 point) {
		return collider->isColiding(getPosition(), point);
	}

	void moveBack(float radius) {
		this->transformMatrix = glm::translate(transformMatrix, -1.0f * 1000.0f *radius * movementDirection);
		graphNode->setLocalTransform(this->transformMatrix);
	}

	bool isTooFarFromCenter(float radius) {
		glm::vec3 translationVector = glm::vec3(transformMatrix[3]);
		if (distanceFromCenter(translationVector) > radius) {
			//this->speed = 0;
			return true;
		}
		return false;
	}

	void move(float deltaTime) {
		this->transformMatrix = glm::translate(transformMatrix, this->speed * deltaTime * this->movementDirection);
		graphNode->setLocalTransform(this->transformMatrix);
	}

	void draw() {
		this->graphNode->draw();
	}

	glm::vec3 getPosition() {
		return glm::vec3(transformMatrix[3]);
	}

	types getType() {
		return type;
	}

	void setShader(unsigned int shaderID) {
		this->graphNode->setShader(shaderID);
	}


	
};

class Bullet {
private:
	Model * model;
	glm::mat4 transformMatrix;
	GraphNode *graphNode;
	glm::vec3 movementDirection;
	float speed;

	float distanceFromCenter(glm::vec3 point) {
		float distance = 0;
		distance += point.x * point.x + point.y*point.y + point.z*point.z;
		return sqrt(distance);
	}

public:
	Bullet(Model *model, unsigned int shaderID, float speed, glm::mat4 transformMatrix, glm::vec3 movementDirection)
	{
		this->model = model;
		this->transformMatrix = transformMatrix;
		this->graphNode = new GraphNode(transformMatrix, new DrawModel(model, shaderID), shaderID);
		this->speed = speed;
		this->movementDirection = movementDirection;
	}		

	bool isTooFarFromCenter(float radius) {
		glm::vec3 translationVector = glm::vec3(transformMatrix[3]);
		if (distanceFromCenter(translationVector) > radius) {
			//this->speed = 0;
			return true;
		}
		return false;
	}

	void move(float deltaTime) {
		this->transformMatrix = glm::translate(transformMatrix, this->speed * deltaTime * this->movementDirection);
		graphNode->setLocalTransform(this->transformMatrix);
	}

	void draw() {
		this->graphNode->draw();
	}

	glm::vec3 getPosition() {
		return glm::vec3(transformMatrix[3]);
	}



};

class Scene {
private:
	vector<Asteroida*> asteroids;
	vector<Bullet*> bullets;
	unsigned int defaultShaderID;
	unsigned int reflexShaderID;
	unsigned int refractShaderID;
	float maxAsteroidDistance;//promieñ, po jakiego przebyciu asteroida jest cofana na drug¹ stronê
	float bulletCooldown; //ms
	float currentBulletCooldown;
	glm::vec3 collidBoxDimensions;
	int points;
	int lives;
	Model *drawModel;
	glm::vec3 minPosition;
	glm::vec3 maxPosition;
	float minSpeed;
	float maxSpeed;

	float randomFloat(float a, float b) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	}

	int randomInt(float a, float b) {		
		int diff = b - a + 1;
		int random = rand()%diff + a;
		return random;
	}

	glm::vec3 randomVec3(glm::vec3 a, glm::vec3 b) {
		glm::vec3 random = glm::vec3(randomFloat(a.x, b.x), randomFloat(a.y, b.y), randomFloat(a.y, b.y));
		return random;
	}

	


public:

	Scene(unsigned int defaultShaderID,	unsigned int reflexShaderID,unsigned int refractShaderID, float maxAsteroidDistance, float bulletCooldown) {
		this->defaultShaderID = defaultShaderID;
		this->reflexShaderID = reflexShaderID;
		this->refractShaderID = refractShaderID;

		this->maxAsteroidDistance = maxAsteroidDistance;
		this->currentBulletCooldown = 0;
		this->bulletCooldown = bulletCooldown;
		this->points = 0;
		this->lives = 3;
		drawModel = new Model("res/models/asteroid/asteroid.obj");
	}
	
	void generateAsteroids(int number, glm::vec3 minPosition, glm::vec3 maxPosition, float minSpeed, float maxSpeed) {
		this->minPosition = minPosition;
		this->maxPosition = maxPosition;
		this->minSpeed = minSpeed;
		this->maxSpeed = maxSpeed;
		this->collidBoxDimensions = 0.001f * calculateColidBoxDimensions(drawModel);
		cout << "Collid box dimentions: " << collidBoxDimensions.x << " " << collidBoxDimensions.y << " " << collidBoxDimensions.z << " " << endl;
		for (int i = 0; i < number; i++) {
			glm::mat4 localTransform(1);
			localTransform = glm::scale(localTransform, glm::vec3(0.001f, 0.001f, 0.001f));
			localTransform = glm::translate(localTransform, 1000.0f * randomVec3(minPosition, maxPosition));
			int rand = randomInt(1, 8);
			Asteroida *asteroid;
			if (rand == 1) {
				asteroid = new Asteroida(drawModel, reflexShaderID, this->randomFloat(minSpeed, maxSpeed), localTransform,
					randomVec3(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f)), collidBoxDimensions, asteroid->REFLEX);
			}
			else if (rand == 2) {
				asteroid = new Asteroida(drawModel, refractShaderID, this->randomFloat(minSpeed, maxSpeed), localTransform,
					randomVec3(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f)), collidBoxDimensions, asteroid->REFRACT);
			}
			else {
				asteroid = new Asteroida(drawModel, defaultShaderID, this->randomFloat(minSpeed, maxSpeed), localTransform,
					randomVec3(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f)), collidBoxDimensions, asteroid->DEFAULT);
			}

			asteroids.push_back(asteroid);
		}
	}

	void generateAsteroid() {
		
		glm::mat4 localTransform(1);
		localTransform = glm::scale(localTransform, glm::vec3(0.001f, 0.001f, 0.001f));
		localTransform = glm::translate(localTransform, 1000.0f * randomVec3(minPosition, maxPosition));

		int rand = randomInt(1, 8);
		Asteroida *asteroid;
		if (rand == 1) {
			asteroid = new Asteroida(drawModel, reflexShaderID, this->randomFloat(minSpeed, maxSpeed), localTransform,
				randomVec3(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f)), collidBoxDimensions, asteroid->REFLEX);
		}
		else if (rand == 2) {
			asteroid = new Asteroida(drawModel, refractShaderID, this->randomFloat(minSpeed, maxSpeed), localTransform,
				randomVec3(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f)), collidBoxDimensions, asteroid->REFRACT);
		}
		else {
			asteroid = new Asteroida(drawModel, defaultShaderID, this->randomFloat(minSpeed, maxSpeed), localTransform,
				randomVec3(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f)), collidBoxDimensions, asteroid->DEFAULT);
		}

		asteroids.push_back(asteroid);
		
	}

	glm::vec3 calculateColidBoxDimensions(Model *model) {
		glm::vec3 dimensionsMax = glm::vec3(numeric_limits<float>::min(), numeric_limits<float>::min(), numeric_limits<float>::min());
		glm::vec3 dimensionsMin = glm::vec3(numeric_limits<float>::max(), numeric_limits<float>::max(), numeric_limits<float>::max());
		for (int i = 0; i < model->meshes.size(); i++) {
			for (int j = 0; j < model->meshes[i].vertices.size();j++) {
				if (model->meshes[i].vertices[j].Position.x > dimensionsMax.x) {
					dimensionsMax.x = model->meshes[i].vertices[j].Position.x;
				}
				if (model->meshes[i].vertices[j].Position.y > dimensionsMax.y) {
					dimensionsMax.y = model->meshes[i].vertices[j].Position.y;
				}
				if (model->meshes[i].vertices[j].Position.z > dimensionsMax.z) {
					dimensionsMax.z = model->meshes[i].vertices[j].Position.z;
				}

				if (model->meshes[i].vertices[j].Position.x < dimensionsMin.x) {
					dimensionsMin.x = model->meshes[i].vertices[j].Position.x;
				}
				if (model->meshes[i].vertices[j].Position.y < dimensionsMin.y) {
					dimensionsMin.y = model->meshes[i].vertices[j].Position.y;
				}
				if (model->meshes[i].vertices[j].Position.z < dimensionsMin.z) {
					dimensionsMin.z = model->meshes[i].vertices[j].Position.z;
				}
			}
		}

		return dimensionsMax - dimensionsMin;

	}

	int getPoints() {
		return points;
	}

	int getLives(){
		return lives;
	}

	float shoot(Model * model, unsigned int shaderID, glm::vec3 position, glm::vec3 direction, float speed) {
		if (this->currentBulletCooldown <= 0) {
			this->currentBulletCooldown = this->bulletCooldown;
			this->addBullet(model, shaderID, position, direction, speed);
		}
		return this->currentBulletCooldown;
	}

	void addBullet(Model * model, unsigned int shaderID, glm::vec3 position, glm::vec3 direction, float speed) {
		glm::mat4 localTransform(1);
		localTransform = glm::scale(localTransform, glm::vec3(0.0001f, 0.0001f, 0.0001f));
		localTransform = glm::translate(localTransform, 10000.0f * position);
		bullets.push_back(new Bullet(model, shaderID, speed, localTransform, direction));

	}

	void move(float deltaTime) {
		for (int i = 0; i < asteroids.size(); i++) {
			if (asteroids[i]->isTooFarFromCenter(maxAsteroidDistance)) {
				Asteroida *asteroid = asteroids[i];
				asteroids.erase(asteroids.begin() + i);
				delete asteroid;
				generateAsteroid();

			}
			else {
				asteroids[i]->move(deltaTime);
			}
			
		}

		for (int i = 0; i < bullets.size(); i++) {
			if (bullets[i]->isTooFarFromCenter(maxAsteroidDistance)) {//remove bullets
				Bullet *bullet = bullets[i];
				bullets.erase(bullets.begin() + i);
				delete bullet;
			}
			else {
				bullets[i]->move(deltaTime);
			}

		}
	}

	void draw() {
		for (int i = 0; i < bullets.size(); i++) {
			bullets[i]->draw();
		}
		for (int i = 0; i < asteroids.size(); i++) {
			asteroids[i]->draw();
		}
		
	}

	int getAsteroidNumber() {
		return asteroids.size();
	}

	void checkBulletsColisions() {
		for (int i = 0; i < bullets.size(); i++) {
			for (int j = 0; j < asteroids.size(); j++) {
				if (asteroids[j]->isColiding(bullets[i]->getPosition())) {
					Asteroida *asteroid = asteroids[j];
					Bullet *bullet = bullets[i];
					if (asteroid->getType() == asteroid->REFLEX) {
						points += 200;
					}
					else if(asteroid->getType() == asteroid->REFRACT){
						points += 250;
					}
					else {
						points += 100;
					}
					asteroids.erase(asteroids.begin() + j);
					bullets.erase(bullets.begin() + i);
					delete asteroid;
					delete bullet;
					cout << "Trafiony! Zostalo "<< asteroids.size()<<"asteroid" << endl;
					
					
					break;

				}
			}
		}
	}

	bool isPlayerColliding(glm::vec3 playerPosition) {
		for (int i = 0; i < asteroids.size(); i++) {
			if (asteroids[i]->isColiding(playerPosition)) {
				return true;
			}
		}
		return false;
	}

	bool update(float deltaTime, glm::vec3 playerPosition) {
		if (this->currentBulletCooldown >= 0) {
			this->currentBulletCooldown -= deltaTime;
		}
		this->move(deltaTime);
		this->checkBulletsColisions();
		if (this->isPlayerColliding(playerPosition)) {
			if (lives > 0) {
				lives--;
			}
			else {
				return false; //koniec gry
			}
		}
		this->draw();
		return true;
	}


};
#endif