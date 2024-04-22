#define _USE_MATH_DEFINES

#include <iostream>
#include <sstream>    
#include <string> 
#include <fstream> 
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h> 

using namespace std;

// Globals.
vector<float> verticesVector; // Vector to read in vertex x, y and z values fromt the OBJ file.
vector<int> facesVector; // Vector to read in face vertex indices from the OBJ file.
static float* vertices = NULL;  // Vertex array of the object x, y, z values.
static int* faces = NULL; // Face (triangle) vertex indices.
static int numIndices; // Number of face vertex indices.
vector<float*> allvertices;
vector<int> allnumIndices;
vector<int*> allfaces;

using namespace std;

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

// Initial position of eye
static float R = 3.0;
static float theta = M_PI * 0;
static float phi = M_PI * 0;

// Function to set the eye position for viewing
void eye(float R, float theta, float phi)
{
	// Polar to Axial Coods
	float x = R * cos(theta) * cos(phi);
	float y = R * sin(theta);
	float z = R * cos(theta) * sin(phi);
	gluLookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void loadOBJ(string fileName)
{
	string line;
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
				if ((previousCharacter == '#') || (currentCharacter == '#')) break;

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
				else previousCharacter = currentCharacter;
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

void appendOBJ(string filename)
{
	// Read the external OBJ file into the internal vertex and face vectors.
	loadOBJ(filename);

	// Size the vertex array and copy into it x, y, z values from the vertex vector.
	vertices = new float[verticesVector.size()];
	for (int i = 0; i < verticesVector.size(); i++) vertices[i] = verticesVector[i];

	// Size the faces array and copy into it face index values from the face vector.
	faces = new int[facesVector.size()];
	for (int i = 0; i < facesVector.size(); i++) faces[i] = facesVector[i];
	numIndices = facesVector.size();
	cout << numIndices;

	facesVector.clear();
	verticesVector.clear();

	//glVertexPointer(3, GL_FLOAT, 0, vertices);

	allvertices.push_back(vertices);
	allfaces.push_back(faces);
	allnumIndices.push_back(numIndices);
}
// Initialization routine.
void setup(void)
{
	glEnableClientState(GL_VERTEX_ARRAY);

	glClearColor(1.0, 1.0, 1.0, 0.0);

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
	appendOBJ("Counter.obj");
	appendOBJ("cupsontables.obj");

	appendOBJ("Diningtable.obj");
	appendOBJ("seatrest.obj");
	appendOBJ("Teaincup.obj");
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

	eye(R, theta, phi);
	//gluPerspective(cameraFoV, 800.0f / 600.0f, 0.1f, 100.0f);
	//gluLookAt(cameraPosition[0], cameraPosition[1], cameraPosition[2], cameraPosition[0] + cameraFront[0], cameraPosition[1] + cameraFront[1], cameraPosition[2] + cameraFront[2], cameraUp[0], cameraUp[1], cameraUp[2]);
	//gluLookAt(0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	// Draw the object mesh.
	drawOBJ(0, 0.982, 0.591, 1);		//Walls
	drawOBJ(1, 0.8, 0.638, 0.423);		//WindowTable
	drawOBJ(2, 0.178, 0.034, 0.0);		//Door
	drawOBJ(3, 0.8, 0.483, 0.165);		//DoorKnob
	drawOBJ(4, 0.791, 0.305, 0.098);	//Floor
	drawOBJ(5, 0.411, 0.117, 0.065);	//Logo

	drawOBJ(6, 0.799, 0.352, 0.12);     //Barchairs
	drawOBJ(7, 1.0, 1.0, 1.0);          //chairjoints
	drawOBJ(8, 0.163, 0.076, 0.029);    //chairlegs
	drawOBJ(9, 0.799, 0.352, 0.12);     //chairseats
	drawOBJ(10, 0.799, 0.647, 0.315);   //Counter
	drawOBJ(11, 0.5, 0.5, 0.5);         //cupsontables

	drawOBJ(12, 0.799, 0.647, 0.315);   //Dining Table
	drawOBJ(13, 0.799, 0.647, 0.315);   //Seat Rest
	drawOBJ(14, 0.163, 0.063, 0.017);   //Tea in cup

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
	gluPerspective(60.0, (float)w / (float)h, 1.0, 50.0);
	glMatrixMode(GL_MODELVIEW);
}

// Function to handle keyboard input
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 48:
		R += 0.05;
		glutPostRedisplay();
		break;
	case 50:
		theta -= 0.04;
		glutPostRedisplay();
		break;
	case 52:
		phi += 0.04;
		glutPostRedisplay();
		break;
	case 53:
		R -= 0.05;
		glutPostRedisplay();
		break;
	case 54:
		phi -= 0.04;
		glutPostRedisplay();
		break;
	case 56:
		theta += 0.04;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}


// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	cout << "How to Use: " << endl;
	cout << "4 and 6 will move camera left and right respectively" << endl;
	cout << "8 and 2 will move camera up and down respectively" << endl;
	cout << "5 and 0 will move zoom in and zoom out respectively" << endl;
	cout << "Camera always points to the origin" << endl;
}

// Main routine.
int main(int argc, char** argv)
{
	printInteraction();
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA);

	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("OBJmodelViewer.cpp");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);


	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}

