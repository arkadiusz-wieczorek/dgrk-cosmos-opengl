

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
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/quaternion_utils.cpp>  
#include <common/vboindexer.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <sstream>
#include <iostream>  



  
  
class model 
{
public :  
    std::vector<glm::vec3> vertices; //tablica wierzchołków
    std::vector<glm::vec2> uvs; //tablica współrzędncyh uv
    std::vector<glm::vec3> normals; // tablica normalnych
    GLuint Texture; //tekstura
    GLuint TextureID; // id tekstury
    GLuint vertexbuffer; // bufor ( analogicznie )
    GLuint uvbuffer; // 
	GLuint normalbuffer; //

    GLuint VertexArrayID;
	int id;
    glm::vec3 position,mscale; // pozycja, skala
	quat rotation; // rotacja
  
    glm::mat4 ModelMatrix;
	
    
  
    model(const char* texture, const char* file, GLuint programID, int Id)
    {
    id = Id; 
    position.x = 0;
    position.y = 0;
    position.z = 0;

	mscale.x = 1;
	mscale.y = 1;
	mscale.z = 1;

    ModelMatrix = glm::mat4(1.0);
	// ładowanie modelu z pliku obj, przypinanie macierzy, tekstur itp itd
    glGenVertexArrays(1, &VertexArrayID); // generowanuie macierzy
    glBindVertexArray(VertexArrayID); // przypięcie amcierzy jako aktywnej // dalej analogicznie
    Texture = loadBMP_custom(texture); 
    TextureID  = glGetUniformLocation(programID, "myTextureSampler"); // 
    bool res = loadOBJ(file, vertices, uvs, normals);
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
  
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
      
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
  
  
    }
  
    int draw(GLuint ProgramID,glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix)
    {
  
  
  
        // Bind our texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler"
        glUniform1i(TextureID, 0);
  
        // 1rst attribute buffer : vertices
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
  
        // 2nd attribute buffer : UVs
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

		// 3rd attribute buffer : normals
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
  
		ModelMatrix = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), mscale) * toMat4(rotation);

       
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
  
        GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		GLuint ViewMatrixID = glGetUniformLocation(ProgramID, "V");
		GLuint ModelMatrixID = glGetUniformLocation(ProgramID, "M");

        
  		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);



        glDrawArrays(GL_TRIANGLES, 0,vertices.size() );
  
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
  
        // Swap buffers
          
        return 0;
  
    }
  
    void transform(vec3 vect)
    {
    position += vect;
    }
	
	void setpos (vec3 vect)
	{
		position = vect;
	}

    void scale(glm::vec3 vector)
    {
	mscale = mscale * vector;
    }
  
    void rotate(glm::vec3 angle)
    {
	angle.x = angle.x * 3.14 / 180;
	angle.y = angle.y * 3.14 / 180;
	angle.z = angle.z * 3.14 / 180;
	rotation = quat(angle);
    }
  
    ~model() 
    {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteTextures(1, &TextureID);
    glDeleteVertexArrays(1, &VertexArrayID);
    }
};

class light
{
	GLuint LightID;

public:
	glm::vec3 lightPos;
	light (glm::vec3 pos, GLuint programID)
	{
		lightPos = pos;
		LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	}

	void draw ()
	{
			glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	}
};

class planet 
{
public:
	double rotation;

	planet() 
	{
		rotation = 0;
	}
	void rotate ()
	{
		rotation += 3;
		if (rotation > 360) rotation -= 360;
	}
};

class control 
{
public:
    vector <model*> models;
	vector <planet*> planets;
	vector <light*> lights;
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
// vertical angle : 0, look at the horizon
float verticalAngle ;
// Initial Field of View
 
float speed;
float mouseSpeed ;


    int init()
    {

	frame = 0;
 position = glm::vec3( 0, 0, 5 );

 horizontalAngle = 3.14f;

 verticalAngle = 0.0f;


 
 speed = 0.2f; 
 mouseSpeed = 0.005f;
	rotation = 0;
	degreey = 0;
	degree = 0;
	coinrot = 0;
	if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
	
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    // Open a window and create its OpenGL context
    if( !glfwOpenWindow( 1024, 768, 0,0,0,0, 32,0, GLFW_WINDOW ) )
    {
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
  
    glfwSetWindowTitle( "Projekt Grafika" );
  
    // Ensure we can capture the escape key being pressed below
    glfwEnable( GLFW_STICKY_KEYS );
  
  
    // Dark blue background
    glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
  
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 
  
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
  
  
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "shader.vertexshader", "shader.fragmentshader" );
	
	


			for(int i = 0; i < 5; i++)
			{
			planet *pl = new planet();
			planets.push_back(pl);
			}

			loadmodel("Mars.bmp","Mars.obj", 1);
			models[models.size() - 1]->transform(glm::vec3 ( 5,5,0 ));
			models[models.size() - 1]->scale(glm::vec3 ( 0.5f,0.5f,0.5f ));


			loadmodel("Neptune.bmp","Neptune.obj", 1);
			models[models.size() - 1]->transform(glm::vec3 ( 4,0,4 ));

			loadmodel("moon.bmp","moon.obj", 1);
			models[models.size() - 1]->transform(glm::vec3 ( 0,2,2 ));

			loadmodel("Venus.bmp","Venus.obj", 1);
			models[models.size() - 1]->transform(glm::vec3 ( 0,-5,-5 ));

			loadmodel("zolty.bmp","ball.obj", 0);
			models[models.size()-1]->transform(glm::vec3 ( 0,0,0 ));
			models[models.size()-1]->rotate(glm::vec3 ( 0,90,0 ));


    position = glm::vec3 (5,5,5);
	
			loadmodel("floor.bmp","ball.obj", 0);
			models[models.size()-1]->transform(glm::vec3 ( position ));
			
			models[models.size()-1]->rotate(glm::vec3 (0,0,0));

	light *point = new light ( glm::vec3 ( 0, 0, 0 ), programID );
	lights.push_back(point);

    return 1;
    }
   
	

    void loadmodel(const char* texuv,const char*  modelobj, int id)
    {
    model *bot = new model(texuv,modelobj, programID, id);  
    models.push_back( bot );
    }
  
	

    void update()
    {

	double currentTime = glfwGetTime();
    nbFrames++;
	if (1000.0/double(nbFrames) < 32) 
		Sleep(32 - 1000.0/double(nbFrames));
	float deltaTime = (float)(currentTime - lastTime);
	
	
	int xpos, ypos;
glfwGetMousePos(&xpos, &ypos);

glfwSetMousePos(1024/2, 768/2);

horizontalAngle += mouseSpeed * deltaTime * float(1024/2 - xpos );
verticalAngle   += mouseSpeed * deltaTime * float( 768/2 - ypos );


		double RotationAngle = 3.14f * 3 / 180;
		glm::vec3 RotationAxis = glm::vec3 (0,0,1);

quat orientation;
glm::vec3 origin = glm::vec3(0,0,0);

orientation.x = RotationAxis.x * sin(RotationAngle / 2);
orientation.y = RotationAxis.y * sin(RotationAngle / 2);
orientation.z = RotationAxis.z * sin(RotationAngle / 2);
orientation.w = cos(RotationAngle / 2);
	
models[0]->position = origin + (orientation * (models[0]->position - origin));


RotationAxis = glm::vec3 (0,1,0);


origin = glm::vec3(0,0,0);

orientation.x = RotationAxis.x * sin(RotationAngle / 2);
orientation.y = RotationAxis.y * sin(RotationAngle / 2);
orientation.z = RotationAxis.z * sin(RotationAngle / 2);
orientation.w = cos(RotationAngle / 2);
	
models[1]->position = origin + (orientation * (models[1]->position - origin));


RotationAxis = glm::vec3 (1,0,0);



orientation.x = RotationAxis.x * sin(RotationAngle / 2);
orientation.y = RotationAxis.y * sin(RotationAngle / 2);
orientation.z = RotationAxis.z * sin(RotationAngle / 2);
orientation.w = cos(RotationAngle / 2);
	
models[2]->position = origin + (orientation * (models[2]->position - origin));


RotationAxis = glm::vec3 (1,1,0);



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


if (glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS){
    position += direction * deltaTime * speed;
}
// Move backward
if (glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS){
    position -= direction * deltaTime * speed;
}
// Strafe right
if (glfwGetKey( GLFW_KEY_RIGHT ) == GLFW_PRESS){
    position += right * deltaTime * speed;
}
// Strafe left
if (glfwGetKey( GLFW_KEY_LEFT ) == GLFW_PRESS){
    position -= right * deltaTime * speed;
}




models[models.size()-1]->position = glm::vec3 ( position + direction * 7.0f  -up  );


lastTime = currentTime;
	
	
	
	}


  
    void draw()
    {
		



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
       


		for (int i = 0; i<models.size(); i++)models[i]->draw(programID,ProjectionMatrix,ViewMatrix);
		for (int i = 0; i<lights.size() ; i++) lights[i]->draw();
        glfwSwapBuffers();
    }
  
    ~control () 
    {
for (int i = 0; i<models.size() ; i++) delete &models[i];
  
glfwTerminate();
    }
};
  
int main( void )
{
      
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


