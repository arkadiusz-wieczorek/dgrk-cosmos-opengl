

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <Windows.h> 
// Include GLEW
#include <GL/glew.h>
  
// Include GLFW
#include <GL/glfw.h>
  
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
 
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;
#include <common/shader.hpp> //ladowanie shaderow
#include <common/texture.hpp> 
#include <common/controls.hpp> //matematyczne
#include <common/objloader.hpp>
#include <common/quaternion_utils.cpp>  
#include <common/vboindexer.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <sstream>
#include <iostream>  


float zmienna = 1.0f;
  
class model {
	public :  
		//z modelu
	    std::vector<glm::vec3> vertices; //tablica wierzchołków
	    std::vector<glm::vec2> uvs; //tablica współrzędncyh uv
	    std::vector<glm::vec3> normals; // tablica normalnych
	    GLuint Texture; //tekstura
	    GLuint TextureID; // id tekstury
	    //bufor danych modelu
	    GLuint vertexbuffer; // bufor ( analogicznie )
	    GLuint uvbuffer; // 
		GLuint normalbuffer; //

	    GLuint VertexArrayID;
		int id;
	    glm::vec3 position,mscale; // pozycja, skala
		quat rotation; // rotacja
	  

	    glm::mat4 ModelMatrix;
		
    
  //ladowanie modelu
    model(const char* texture, const char* file, GLuint programID, int Id){
		id = Id; 
		// domyslne pozycje wierzcholkow x,y,z - zmienia sie je rotate i translate
		position.x = 0;
		position.y = 0;
		position.z = 0;

		// domyslna skala wierzcholkow x,y,z - zmienia sie je za pomoca scale
		mscale.x = 1;
		mscale.y = 1;
		mscale.z = 1;

		ModelMatrix = glm::mat4(1.0);
		// generowanie i bindowanie tablicy wierzcholkow
		glGenVertexArrays(1, &VertexArrayID); // generowanuie macierzy
		glBindVertexArray(VertexArrayID); // przypięcie maierzy jako aktywnej // dalej analogicznie

		// ladowanie tekstury
		Texture = loadBMP_custom(texture); 
		TextureID  = glGetUniformLocation(programID, "myTextureSampler");

		// ladowanie obiektu
		bool res = loadOBJ(file, vertices, uvs, normals);
		// generowanie i bind tablicy wierzcholkow
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
  		
  		// generowanie i bind tablicy uv's
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
      
      	// generowanie i bind tablicy normalnych
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    }
  
  	// rysowanie modelu
    int draw(GLuint ProgramID,glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix) {

		// bindowanie tekstury
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        
        // Ustawia "myTextureSampler" z wartoscia int
        glUniform1i(TextureID, 0);
  
        // 1 atrybut bufora - wierzcholki
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
  
        // 2 atrybut bufora - uv's
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            1,                                // attribute
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

		// 3 atrybut bufora - normalne
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
  		
  		// maciez modelu - translacja * skala * rotacja
		ModelMatrix = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), mscale) * toMat4(rotation);
		
		// wyswietlenie modelu tam gdzie powinien byc z punktu widzenia kamery,
		// transportuje wierzcholki przemnozone przez perspektywe i pozycje kamery
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
  		
  		// uniform MVP - Model View Projection
        GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		GLuint ViewMatrixID = glGetUniformLocation(ProgramID, "V");// view
		GLuint ModelMatrixID = glGetUniformLocation(ProgramID, "M"); // model
		
		// uniform Model i View
  		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glDrawArrays(GL_TRIANGLES, 0,vertices.size() );
  
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
  
        // Swap buffers
        return 0;
  
    }
  	// przeksztalcenia (+/- wektor)
    void transform(vec3 vect){
		position += vect;
    }
	
	void setpos (vec3 vect){
		position = vect;
	}
	// skalowanie modelu (* wektor)
    void scale(glm::vec3 vector){
		mscale = mscale * vector;
    }
  	// obracanie modelu (x,y,z)
    void rotate(glm::vec3 angle){
		angle.x = angle.x * 3.14 / 180;
		angle.y = angle.y * 3.14 / 180;
		angle.z = angle.z * 3.14 / 180;
		rotation = quat(angle);
    }
  	// usuwanie modelu
    ~model(){
    glDeleteBuffers(1, &vertexbuffer);// usuwa bufor wierzcholkow
    glDeleteBuffers(1, &uvbuffer); //bufor uv's
    glDeleteTextures(1, &TextureID); // usuwa teksture
    glDeleteVertexArrays(1, &VertexArrayID); //usuwa tablice wierzcholkow
    }
};

class light{
	// id oswietlenia - uniform
	GLuint LightID;

	public:
		glm::vec3 lightPos; // wektor 3-wym ustawiajacy pozycje oswietlenia
		light (glm::vec3 pos, GLuint programID){ // definiowanie oswietlenia
		// ustawienie pozycji oswietlenia
			lightPos = pos;
			LightID = glGetUniformLocation(programID, "LightPosition_worldspace"); 	// ustawienie id - uniform
		}
		// rysowanie oswietlenia
		void draw (){ 
				glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		}
	};

class planet{

	public:
		double rotation;

		planet() {
			rotation = 0;
		}

		void rotate (){
			rotation += 3;
			if (rotation > 360) rotation -= 360;
		}
	};

class control {
	public:
		vector <model*> models;
		vector <planet*> planets;
		vector <light*> lights; // oswietlenie
		GLuint MatrixID ;
		int lastTime;
		int nbFrames;
		GLuint programID;
		int frame ;
		int ktory;
		int map[10][50];
		glm::vec3 direction, right, up;
		double rotation, degree, degreey, coinrot;

	 glm::vec3 position;
	// horizontal angle : toward -Z
	float horizontalAngle;
	float goal_horizontalAngle;
	// vertical angle : 0, look at the horizon
	float verticalAngle ;
	float goal_verticalAngle;
	// Initial Field of View
 
	float speed;
	float mouseSpeed ;


    int init(){

		frame = 0;
		position = glm::vec3( 0, 0, 0 );
		horizontalAngle = 3.14f;
		goal_horizontalAngle = horizontalAngle;
		verticalAngle = 0.0f;
		goal_verticalAngle = verticalAngle;

		speed = 0.6f; 
		mouseSpeed = 0.005f;
		rotation = 0;
		degreey = 0;
		degree = 0;
		coinrot = 0;

	if( !glfwInit() ) {
        
		fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
	
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    // Open a window and create its OpenGL context
    if( !glfwOpenWindow( 1600, 1200, 0,0,0,0, 32,0, GLFW_WINDOW ) ) {

        fprintf( stderr, "Failed to open GLFW window..\n" );
        glfwTerminate();
        return -1;
    }
  
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
  
    glfwSetWindowTitle( "Projekt DGRK - Arkadiusz Wieczorek" );
  
    // Ensure we can capture the escape key being pressed below
    glfwEnable( GLFW_STICKY_KEYS );
  
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 
  
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
  
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "shader.vertexshader", "shader.fragmentshader" );
	

			for(int i = 0; i < 6; i++){

				planet *pl = new planet();
				planets.push_back(pl);
			}
				
				loadmodel("textures/mars.bmp","objects/Mars.obj", 1);
				models[models.size() - 1]->transform(glm::vec3 ( 0,3,0 ));
				models[models.size() - 1]->scale(glm::vec3 ( 0.2f,0.2f,0.2f ));


				loadmodel("textures/cassini.bmp","objects/cassini.obj", 1);
				models[models.size() - 1]->transform(glm::vec3 ( 2,0,2 ));
				models[models.size() - 1]->scale(glm::vec3 ( 0.5f,0.5f,0.5f ));

				loadmodel("textures/venus.bmp","objects/Venus.obj", 1);
				models[models.size() - 1]->transform(glm::vec3 ( 0,6,6 ));
				models[models.size() - 1]->scale(glm::vec3 ( 0.5f,0.5f,0.5f ));


				loadmodel("textures/neptune.bmp","objects/Neptune.obj", 1);
				models[models.size() - 1]->transform(glm::vec3 ( 0,1,-5 ));
				models[models.size() - 1]->scale(glm::vec3 ( 0.5f,0.5f,0.5f ));
				
				loadmodel("textures/sun.bmp","objects/Neptune.obj", 0);
				models[models.size()-1]->transform(glm::vec3 ( 0,0,0 ));
				models[models.size()-1]->scale(glm::vec3 (1.4f,1.4f,1.4f));
				models[models.size()-1]->rotate(glm::vec3 ( 0,90,0 ));

				loadmodel("textures/marine.bmp","objects/box.obj", 0);
				models[models.size()-1]->transform(glm::vec3 ( 0,-12,0 ));
				models[models.size()-1]->scale(glm::vec3 (1.3f,1.3f,1.3f));
				models[models.size()-1]->rotate(glm::vec3 ( 0,30,0 ));

				loadmodel("textures/tank.bmp","objects/tank.obj", 0);
				models[models.size()-1]->transform(glm::vec3 ( -10,-13,-20 ));
				models[models.size()-1]->scale(glm::vec3 (0.8f,0.8f,0.8f));
				models[models.size()-1]->rotate(glm::vec3 ( 0,-10,0 ));
				/*
				loadmodel("textures/city.bmp","objects/cobalt_city.obj", 0);
				models[models.size()-1]->transform(glm::vec3 ( 10,-14,30 ));
				models[models.size()-1]->scale(glm::vec3 (0.05f,0.05f,0.05f));
				models[models.size()-1]->rotate(glm::vec3 ( 0,-10,0 ));
				*/

				loadmodel("textures/top.bmp","objects/masterchief2.obj", 0);
				models[models.size()-1]->transform(glm::vec3 ( -5,-13.5,0 ));
				models[models.size()-1]->scale(glm::vec3 (1.0f,1.0f,1.0f));
				models[models.size()-1]->rotate(glm::vec3 ( 0,90,90 ));
				
				loadmodel("textures/teren3.bmp","objects/teren.obj", 0);
				models[models.size()-1]->transform(glm::vec3 ( 0,-20,0 ));
				models[models.size()-1]->scale(glm::vec3 (6.0f,6.0f,13.0f));
				models[models.size()-1]->rotate(glm::vec3 ( 0,90,0 ));
				
			

				//load skybox
				loadmodel("skybox/skybox15.bmp","objects/skybox.obj", 1);
				models[models.size() - 1]->scale(glm::vec3 ( 25,25,40 ));
			//pozycja statku
		    position = glm::vec3 (2,0,0);
		//statek
			loadmodel("textures/top.bmp","objects/ufo1.obj", 1);
			//models[models.size()-1]->transform(glm::vec3 ( 12,0,0 ));
			models[models.size()-1]->transform(glm::vec3 ( position ));
			models[models.size()-1]->scale(glm::vec3 (0.2f,0.2f,0.2f));
			models[models.size()-1]->rotate(glm::vec3 (0,0,0));
	//pozycja swiatla
	light *point = new light ( glm::vec3 ( 0, 0, 0 ), programID );
	lights.push_back(point);

	return 1;
    }
   
	//ladowanie calego stosu obiektów

    void loadmodel(const char* texuv,const char*  modelobj, int id)
    {
	    model *bot = new model(texuv,modelobj, programID, id);  
	    models.push_back( bot );
    }
  
	

    void update() {

		double currentTime = glfwGetTime();
		nbFrames++;
			if (1000.0/double(nbFrames) < 32) 
				Sleep(32 - 1000.0/double(nbFrames));
		float deltaTime = (float)(currentTime - lastTime);
	
		int xpos, ypos;
		//obsluga myszy
		glfwGetMousePos(&xpos, &ypos);

		/*
		float axesValues[2];
		glfwGetJoystickPos(GLFW_JOYSTICK_1,axesValues,2);
		for(int ax=0; ax<2; ax++){
			printf("%f", axesValues[ax]);
		}
		printf("\n");

		xpos = 800*axesValues[0] + 800;
		ypos = 600*axesValues[1] + 600;

		*/
		glfwSetMousePos(1600/2, 1200/2);

		float camera_angle_speed = 0.1;


			float horizontal_change = mouseSpeed * deltaTime * float(1600/2 - xpos );
			float vertical_change = mouseSpeed * deltaTime * float( 1200/2 - ypos );

			goal_horizontalAngle += horizontal_change;
			goal_verticalAngle   += vertical_change;

			//printf("goal_horizontalAngle: %f, horizontalAngle: %f\n", goal_horizontalAngle, horizontalAngle);

			if(goal_verticalAngle>verticalAngle){
				if(abs(goal_verticalAngle-verticalAngle)>camera_angle_speed){
					verticalAngle+=camera_angle_speed;
				}else{
					verticalAngle=goal_verticalAngle;
				}
			}else{
				if(abs(goal_verticalAngle-verticalAngle)>camera_angle_speed){
					verticalAngle-=camera_angle_speed;
				}else{
					verticalAngle=goal_verticalAngle;
				}
			}
			if(goal_horizontalAngle>horizontalAngle){
				if(abs(goal_horizontalAngle-horizontalAngle)>camera_angle_speed){
					horizontalAngle+=camera_angle_speed;
				}else{
					horizontalAngle=goal_horizontalAngle;
				}
			}else{
				if(abs(goal_horizontalAngle-horizontalAngle)>camera_angle_speed){
					horizontalAngle-=camera_angle_speed;
				}else{
					horizontalAngle=goal_horizontalAngle;
				}
			}



		double RotationAngle = 3.14f * 1 / 180;
		glm::vec3 RotationAxis = glm::vec3 (0,0,1); //predkosc Venusa

			quat orientation;
		glm::vec3 origin = glm::vec3(0,0,0);
			orientation.x = RotationAxis.x * sin(RotationAngle / 2);
			orientation.y = RotationAxis.y * sin(RotationAngle / 2);
			orientation.z = RotationAxis.z * sin(RotationAngle / 2);
			orientation.w = cos(RotationAngle / 2);
	
		models[0]->position = origin + (orientation * (models[0]->position - origin));
		RotationAxis = glm::vec3 (0,zmienna,0); //ruch satelity
		

		origin = glm::vec3(0,0,0);
			orientation.x = RotationAxis.x * sin(RotationAngle / 2);
			orientation.y = RotationAxis.y * sin(RotationAngle / 2);
			orientation.z = RotationAxis.z * sin(RotationAngle / 2);
			orientation.w = cos(RotationAngle / 2);
	
		models[1]->position = origin + (orientation * (models[1]->position - origin));
		RotationAxis = glm::vec3 (1,0,0); //predkosc turkusowej
		

			orientation.x = RotationAxis.x * sin(RotationAngle / 2);
			orientation.y = RotationAxis.y * sin(RotationAngle / 2);
			orientation.z = RotationAxis.z * sin(RotationAngle / 2);
			orientation.w = cos(RotationAngle / 2);
	
		models[2]->position = origin + (orientation * (models[2]->position - origin));
		RotationAxis = glm::vec3 (1,3,0); //predkosc niebieskiej


			orientation.x = RotationAxis.x * sin(RotationAngle / 2);
			orientation.y = RotationAxis.y * sin(RotationAngle / 2);
			orientation.z = RotationAxis.z * sin(RotationAngle / 2);
			orientation.w = cos(RotationAngle / 2);
	
		models[3]->position = origin + (orientation * (models[3]->position - origin));


			direction = glm::vec3(
				cos(verticalAngle) * sin(horizontalAngle),
				sin(verticalAngle),
				cos(verticalAngle) * cos(horizontalAngle)
			);

			right = glm::vec3(
				sin(horizontalAngle - 3.14f/2.0f),
				0,
				cos(horizontalAngle - 3.14f/2.0f)
			);

			up = glm::cross( right, direction );

		/*
		float axesValues[2];
		glfwGetJoystickPos(GLFW_JOYSTICK_1,axesValues,2);
		for(int ax=0; ax<2; ax++){
			printf("%f", axesValues[ax]);
		}
		printf("\n");
		*/
		
		unsigned char buttons[12];
		glfwGetJoystickButtons(GLFW_JOYSTICK_1,buttons,12);
		for(int j=0; j<12; j++){
			//printf("%i", buttons[j]);
		}
		printf("\n");
		
		if(buttons[1] != 0){position += direction * deltaTime * speed;} //przod
		if(buttons[2] != 0){position -= direction * deltaTime * speed;} //tyl
		if(buttons[3] != 0){position += right * deltaTime * speed;} //prawo
		if(buttons[0] != 0){position -= right * deltaTime * speed;} //lewo

		if(buttons[5] != 0){zmienna = zmienna + 0.1f;} //przyspieszanie satelity
		if(buttons[4] != 0){zmienna = zmienna - 0.1f;}

		if (glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS){position += direction * deltaTime * speed;	}
		if (glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS){position -= direction * deltaTime * speed;}
		if (glfwGetKey( GLFW_KEY_RIGHT ) == GLFW_PRESS){position += right * deltaTime * speed;}
		if (glfwGetKey( GLFW_KEY_LEFT ) == GLFW_PRESS){position -= right * deltaTime * speed;}
		/*
		if (glfwGetKey( GLFW_KEY_ENTER ) == GLFW_PRESS){
			zmienna = zmienna + 0.1f;
			printf("%f ",zmienna);
		}
		*/




		models[models.size()-1]->position = glm::vec3 ( position + direction * 7.0f  -up  );
		
		lastTime = currentTime;
	}


  
    void draw(){
		
		if(frame > 0) frame--;
		
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
        // Use our shader
        glUseProgram(programID);
          
		// computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

			glm::mat4 ViewMatrix       = glm::lookAt(
			    position,           // Camera is here
			    position+direction, // and looks here : at the same position, plus "direction"
			    up                  // Head is up (set to 0,-1,0 to look upside-down)
			);
			       	//rysowanie modeli
					for (int i = 0; i<models.size(); i++)models[i]->draw(programID,ProjectionMatrix,ViewMatrix);

					//rysowanie swiatla
					for (int i = 0; i<lights.size() ; i++) lights[i]->draw();
			        glfwSwapBuffers();
			    }
			  
			    ~control (){
					for (int i = 0; i<models.size() ; i++) delete &models[i];
					glfwTerminate();
			    }
			};
  
int main( void ){
     
    //tworzenie nowej instancji klasy control
    control *CONTROLLER = new control();
  
    if(CONTROLLER->init() != 1) return -1;
  
    CONTROLLER->nbFrames = 0;
    CONTROLLER->lastTime = glfwGetTime();
  
    do{
        CONTROLLER->update();
        CONTROLLER->draw();
  
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
           glfwGetWindowParam( GLFW_OPENED ) );
    return 0;
}


