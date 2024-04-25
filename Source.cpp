#define _USE_MATH_DEFINES
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
using namespace std;
#include <GL/glew.h>
#include <GL/freeglut.h>

// Camera parameters
float cameraSpeed = 0.05f;
float cameraYaw = -90.0f;
float cameraPitch = 0.0f;
float cameraFoV = 45.0f;
GLfloat cameraPosition[3] = {0.0f, 0.0f, 3.0f};
GLfloat cameraFront[3] = {0.0f, 0.0f, -1.0f};
GLfloat cameraUp[3] = {0.0f, 1.0f, 0.0f};

// Mouse movement variables
int lastX = 400, lastY = 300;
bool firstMouse = true;

// Initial position of eye
static float radial_coordinate = 3;
static float latitude_angle = 0;
static float longitude_angle = 0;
static float zoomFactor = 1.0;

vector<float *> allvertices;
vector<int> allnumIndices;
vector<int *> allfaces;

// Globals.
static std::vector<float> verticesVector;			   // Vector to read in vertex x, y and z values from the OBJ file.
static std::vector<int> facesVector;				   // Vector to read in face vertex indices from the OBJ file.
static float *vertices = NULL;						   // Vertex array of the object x, y, z values.
static int *faces = NULL;							   // Face (triangle) vertex indices.
static int numIndices;								   // Number of face vertex indices.
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate the object.

// Routine to read a Wavefront OBJ file.
// Only vertex and face lines are processed. All other lines,including texture,
// normal, material, etc., are ignored.
// Vertex lines are assumed to have only x, y, z coordinate values and no w value.
// If a w value is present it is ignored.
// Within a face line only vertex indices are read. Texture and normal indices are
// allowed but ignored. Face lines can have more than three vertices.
//
// OUTPUT: The vertex coordinate values are written to a vector "verticesVector"
// (this name is hardcoded in the routine so the calling program should take note).
// The face vertex indices are written to a vector "facesVector" (hardcoded name).
// Faces with more than three vertices are fan triangulated about the first vertex
// and the triangle indices written. In other words, output faces are all triangles.
// All vertex indices are decremented by 1 to make the index range start from 0.
void loadOBJ(std::string fileName)
{
	std::string line;
	int count, vertexIndex1, vertexIndex2, vertexIndex3;
	float coordinateValue;
	char currentCharacter, previousCharacter;

	// Open the OBJ file.
	std::ifstream inFile(fileName.c_str(), std::ifstream::in);

	// Read successive lines.
	while (getline(inFile, line))
	{
		// Line has vertex data.
		if (line.substr(0, 2) == "v ")
		{
			// Initialize a string from the character after "v " to the end.
			std::istringstream currentString(line.substr(2));

			// Read x, y and z values. The (optional) w value is not read.
			for (count = 1; count <= 3; count++)
			{
				currentString >> coordinateValue;
				verticesVector.push_back(coordinateValue);
			}
		}

		// Line has face data.
		else if (line.substr(0, 2) == "f ")
		{
			// Initialize a string from the character after "f " to the end.
			std::istringstream currentString(line.substr(2));

			// Strategy in the following to detect a vertex index within a face line is based on the
			// fact that vertex indices are exactly those that follow a white space. Texture and
			// normal indices are ignored.
			// Moreover, from the third vertex of a face on output one triangle per vertex, that
			// being the next triangle in a fan triangulation of the face about the first vertex.
			previousCharacter = ' ';
			count = 0;
			while (currentString.get(currentCharacter))
			{
				// Stop processing line at comment.
				if ((previousCharacter == '#') || (currentCharacter == '#'))
					break;

				// Current character is the start of a vertex index.
				if ((previousCharacter == ' ') && (currentCharacter != ' '))
				{
					// Move the string cursor back to just before the vertex index.
					currentString.unget();

					// Read the first vertex index, decrement it so that the index range is from 0, increment vertex counter.
					if (count == 0)
					{
						currentString >> vertexIndex1;
						vertexIndex1--;
						count++;
					}

					// Read the second vertex index, decrement it, increment vertex counter.
					else if (count == 1)
					{
						currentString >> vertexIndex2;
						vertexIndex2--;
						count++;
					}

					// Read the third vertex index, decrement it, increment vertex counter AND output the first triangle.
					else if (count == 2)
					{
						currentString >> vertexIndex3;
						vertexIndex3--;
						count++;
						facesVector.push_back(vertexIndex1);
						facesVector.push_back(vertexIndex2);
						facesVector.push_back(vertexIndex3);
					}

					// From the fourth vertex and on output the next triangle of the fan.
					else
					{
						vertexIndex2 = vertexIndex3;
						currentString >> vertexIndex3;
						vertexIndex3--;
						facesVector.push_back(vertexIndex1);
						facesVector.push_back(vertexIndex2);
						facesVector.push_back(vertexIndex3);
					}

					// Begin the process of detecting the next vertex index just after the vertex index just read.
					currentString.get(previousCharacter);
				}

				// Current character is not the start of a vertex index. Move ahead one character.
				else
					previousCharacter = currentCharacter;
			}
		}

		// Nothing other than vertex and face data is processed.
		else
		{
		}
	}

	// Close the OBJ file.
	inFile.close();
}

void eye_position(float radial_coordinate, float latitude_angle, float longitude_angle)
{
	float x_position = radial_coordinate * cos(latitude_angle) * cos(longitude_angle);
	float y_position = radial_coordinate * sin(latitude_angle);
	float z_position = radial_coordinate * cos(latitude_angle) * sin(longitude_angle);
	gluLookAt(x_position, y_position, z_position, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void appendOBJ(string filename)
{
	// Read the external OBJ file into the internal vertex and face vectors.
	loadOBJ(filename);

	// Size the vertex array and copy into it x, y, z values from the vertex vector.
	vertices = new float[verticesVector.size()];
	for (int i = 0; i < verticesVector.size(); i++)
		vertices[i] = verticesVector[i];

	// Size the faces array and copy into it face index values from the face vector.
	faces = new int[facesVector.size()];
	for (int i = 0; i < facesVector.size(); i++)
		faces[i] = facesVector[i];
	numIndices = facesVector.size();

	facesVector.clear();
	verticesVector.clear();

	allvertices.push_back(vertices);
	allfaces.push_back(faces);
	allnumIndices.push_back(numIndices);
}

// Initialization routine.
void setup(void)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glClearColor(1.0, 1.0, 1.0, 0.0);
	// glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	appendOBJ("walls.obj");
	appendOBJ("Windowtable.obj");
	appendOBJ("Door.obj");
	appendOBJ("Doorknob.obj");
	appendOBJ("floor.obj");
	appendOBJ("Logo.obj");

	appendOBJ("Barchairs.obj");
	appendOBJ("chairjoints.obj");
	appendOBJ("chairlegs.obj");
	appendOBJ("chairseats.obj");
	appendOBJ("cupsontables.obj");

	appendOBJ("Diningtable.obj");
	appendOBJ("seatrest.obj");
	appendOBJ("Teaincup.obj");

	appendOBJ("Pancakes.obj");
	appendOBJ("Croissantsinplates.obj");
	appendOBJ("butteronpancakes.obj");
	appendOBJ("baguettepart1.obj");
	appendOBJ("baguettepart2.obj");
	appendOBJ("Drippings.obj");
	appendOBJ("Donutsonrack.obj");
	appendOBJ("Counterbreadrack.obj");
	appendOBJ("Counterdonutrack.obj");
}

void drawOBJ(int i, float r, float g, float b)
{
	glPushMatrix();
	glColor3f(r, g, b);
	glVertexPointer(3, GL_FLOAT, 0, allvertices[i]);
	glDrawElements(GL_TRIANGLES, allnumIndices[i], GL_UNSIGNED_INT, allfaces[i]);
	glPopMatrix();
}

// Drawing routine.
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Set up camera position.
	eye_position(radial_coordinate * zoomFactor, latitude_angle, longitude_angle);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	// Rotate scene.
	glRotatef(Zangle, 0.0, 0.0, 1.0);
	glRotatef(Yangle, 0.0, 1.0, 0.0);
	glRotatef(Xangle, 1.0, 0.0, 0.0);

	glScalef(5.0, 5.0, 5.0);
	// Draw the object mesh.
	drawOBJ(0, 0.709, 0.756, 0.556); // Walls
	// drawOBJ(0, 0.0, 0.0, 0.0);
	drawOBJ(1, 0.8, 0.638, 0.423);		// WindowTable
	drawOBJ(2, 0.178, 0.034, 0.0);		// Door
	drawOBJ(3, 0.8, 0.483, 0.165);		// DoorKnob
	drawOBJ(4, 0.6274, 0.3215, 0.3647); // Floor
	drawOBJ(5, 0.411, 0.117, 0.065);	// Logo

	drawOBJ(6, 0.0, 0.0, 0.0);		 // Barchairs
	drawOBJ(7, 1.0, 1.0, 1.0);		 // chairjoints
	drawOBJ(8, 0.163, 0.076, 0.029); // chairlegs
	drawOBJ(9, 0.0, 0.0, 0.0);		 // chairseats
	// drawOBJ(10, 0.799, 0.647, 0.315);   //Counter
	drawOBJ(10, 0.5, 0.5, 0.5); // cupsontables

	drawOBJ(11, 0.799, 0.647, 0.315); // Dining Table
	drawOBJ(12, 0.799, 0.647, 0.315); // Seat Rest
	drawOBJ(13, 0.163, 0.063, 0.017); // Tea in cup

	drawOBJ(14, 0.42, 0.14, 0.054);	  // Pancakes
	drawOBJ(15, 0.82, 0.65, 0.472);	  // CroissantinPlates
	drawOBJ(16, 1.0, 1.0, 0.51);	  // ButteronPancakes
	drawOBJ(17, 0.607, 0.49, 0.32);	  // BaguetterPart1
	drawOBJ(18, 0.74, 0.60, 0.4196);  // BaguettePart2
	drawOBJ(19, 1.0, 0.74, 0.0);	  // Drippings
	drawOBJ(20, 0.972, 0.513, 0.474); // Donut
	drawOBJ(21, 0.949, 0.823, 0.741); // Counterbread
	drawOBJ(22, 0.1, 0.1, 0.1);		  // CounterDonut

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glutPostRedisplay();
	glFlush();

	glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)w / (float)h, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

// Mouse callback function for handling rotation and zooming.
void mouse(int button, int state, int x, int y)
{
	static int lastX = 0, lastY = 0;
	static float rotationFactor = 0.01;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		lastX = x;
		lastY = y;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		lastX = x;
		lastY = y;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		lastY = y;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		lastY = y;
	}
}

void mouse_callback(int xpos, int ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	cameraYaw += xoffset;
	cameraPitch += yoffset;

	// Prevent camera from flipping over
	if (cameraPitch > 89.0f)
		cameraPitch = 89.0f;
	if (cameraPitch < -89.0f)
		cameraPitch = -89.0f;

	GLfloat front[3];
	front[0] = cos(cameraYaw * M_PI / 180) * cos(cameraPitch * M_PI / 180);
	front[1] = sin(cameraPitch * M_PI / 180);
	front[2] = sin(cameraYaw * M_PI / 180) * cos(cameraPitch * M_PI / 180);
	for (int i = 0; i < 3; ++i)
		cameraFront[i] = front[i];
}

// Mouse wheel callback function for zooming.
void mouseWheel(int wheel, int direction, int x, int y)
{
	if (direction > 0)
		zoomFactor *= 1.1;
	else
		zoomFactor *= 0.9;

	glutPostRedisplay();
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 48:
		radial_coordinate += 0.03;
		glutPostRedisplay();
		break;
	case 50:
		latitude_angle -= 0.02;
		glutPostRedisplay();
		break;
	case 52:
		longitude_angle += 0.02;
		glutPostRedisplay();
		break;
	case 53:
		radial_coordinate -= 0.03;
		glutPostRedisplay();
		break;
	case 54:
		longitude_angle -= 0.02;
		glutPostRedisplay();
		break;
	case 56:
		latitude_angle += 0.02;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	cout << "Interaction:" << std::endl;
	cout << "Use mouse to rotate the object." << std::endl;
	cout << "Use mouse wheel to zoom in/out." << std::endl;
}

// Main routine.
int main(int argc, char **argv)
{
	printInteraction();
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("OBJmodelViewer.cpp");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutPassiveMotionFunc(mouse_callback);
	glutKeyboardFunc(keyInput);
	glutMouseFunc(mouse);
	glutMouseWheelFunc(mouseWheel);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
	return 0;
}
