#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#define FREEGLUT_STATIC
#include "gl_core_3_3.h"
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

#define TW_STATIC
#include <AntTweakBar.h>


#include <ctime>
#include <memory>
#include <vector>
#include <string>
#include <cstdlib>
#include<iostream>

#include "objloader.h"
#include "glprogram.h"
#include "MyImage.h"
#include "VAOImage.h"
#include "VAOMesh.h"
#include "trackball.h"

#include "laplacian.h"

bool _cot = false, nowcot = false;
GLProgram MyMesh::prog, MyMesh::pickProg, MyMesh::pointSetProg;
GLTexture MyMesh::colormapTex;

MyMesh M;
int viewport[4] = { 0, 0, 1280, 960 };
int actPrimType = MyMesh::PE_VERTEX;

bool showATB = true;

using MatX3f = Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>;
using MatX3i = Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>;
MatX3f meshX, nmeshX;
Eigen::Matrix<float, 3, 3, Eigen::RowMajor>revolve;
MatX3f delta,oridelta;
MatX3i meshF;
Eigen::SparseMatrix<float, Eigen::ColMajor> L;
float myRotation[4] = { 1, 0, 0, 0 }; //BONUS: interactively specify the rotation for the Laplacian coordinates at the handles

float* updatedelta() {
    float w = myRotation[0], x = myRotation[1], y = myRotation[2], z = myRotation[3];
    revolve(0, 0) = 1 - 2 * y * y - 2 * z * z;
    revolve(0, 1) = 2 * x * y - 2 * w * z;
    revolve(0, 2) = 2 * x * z + 2 * w * y;
    revolve(1, 0) = 2 * x * y + 2 * w * z;
    revolve(1, 1) = 1 - 2 * x * x - 2 * z * z;
    revolve(1, 2) = 2 * y * z - 2 * w * x;
    revolve(2, 0) = 2 * x * z - 2 * w * y;
    revolve(2, 1) = 2 * y * z - 2 * w * x;
    revolve(2, 2) = 1 - 2 * y * y - 2 * x * x;
    delta = (revolve * oridelta.transpose()).transpose();
    return myRotation;
}
void deform_preprocess()
{
    if(_cot)
        L = Laplacian_cot(meshX, meshF);
    else
        L = Laplacian_uniform(meshX, meshF);
    oridelta = L * meshX;
    updatedelta();
}
char* get_mode() {
    if (_cot)return "Cot";
    return "Uniform";
}



void meshDeform()
{
    using namespace Eigen;
    if (nowcot != _cot) {
        deform_preprocess();
        nowcot = _cot;
    }


    std::vector<int> P2PVtxIds = M.getConstrainVertexIds();
    std::vector<float> p2pDsts = M.getConstrainVertexCoords();


    // TODO 2: compute deformation result y using Laplacian coordinates


    SparseMatrix<float, ColMajor> L1(L);
    MatX3f b(delta);
    for (int i = 0; i < size(P2PVtxIds); i++) {
        float lambda = 1e5;
        L1.coeffRef(P2PVtxIds[i], P2PVtxIds[i]) += lambda;
        b(P2PVtxIds[i], 0) += p2pDsts[3 * i + 0] * lambda;
        b(P2PVtxIds[i], 1) += p2pDsts[3 * i + 1] * lambda;
        b(P2PVtxIds[i], 2) += p2pDsts[3 * i + 2] * lambda;
    }
    SimplicialLDLT<SparseMatrix<float, ColMajor>> solver;
    solver.compute(L1);
    Matrix<float, Dynamic, Dynamic, RowMajor> y = solver.solve(b);
    if (y.cols() > 3)  y = y.leftCols(3);
    if (y.rows() == 0 || y.cols() != 3) return;

    M.upload(y.data(), y.rows(), nullptr, 0, nullptr);
}

int mousePressButton;
int mouseButtonDown;
int mousePos[2];

bool msaa = true;


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (msaa) glEnable(GL_MULTISAMPLE);
    else glDisable(GL_MULTISAMPLE);

    glViewport(0, 0, viewport[2], viewport[3]);
    M.draw(viewport);

    if (showATB) TwDraw();
    glutSwapBuffers();

    //glFinish();
}

void onKeyboard(unsigned char code, int x, int y)
{
    if (!TwEventKeyboardGLUT(code, x, y)) {
        switch (code) {
        case 17:
            exit(0);
        case 'f':
            glutFullScreenToggle();
            break;
        case ' ':
            showATB = !showATB;
            break;
        }
    }

    glutPostRedisplay();
}

void onMouseButton(int button, int updown, int x, int y)
{
    if (!showATB || !TwEventMouseButtonGLUT(button, updown, x, y)) {
        mousePressButton = button;
        mouseButtonDown = updown;

        if (updown == GLUT_DOWN) {
            if (button == GLUT_LEFT_BUTTON) {
                if (glutGetModifiers()&GLUT_ACTIVE_CTRL) {
                }
                else {
                    int r = M.pick(x, y, viewport, M.PE_VERTEX, M.PO_ADD);
                }
            }
            else if (button == GLUT_RIGHT_BUTTON) {
                M.pick(x, y, viewport, M.PE_VERTEX, M.PO_REMOVE);
            }
        }
        else { // updown == GLUT_UP
            if (button == GLUT_LEFT_BUTTON);
        }

        mousePos[0] = x;
        mousePos[1] = y;
    }

    glutPostRedisplay();
}


void onMouseMove(int x, int y)
{
    if (!showATB || !TwEventMouseMotionGLUT(x, y)) {
        if (mouseButtonDown == GLUT_DOWN) {
            if (mousePressButton == GLUT_MIDDLE_BUTTON) {
                M.moveInScreen(mousePos[0], mousePos[1], x, y, viewport);
            }
            else if (mousePressButton == GLUT_LEFT_BUTTON) {
                if (!M.moveCurrentVertex(x, y, viewport)) {
                    meshDeform();
                }
                else {
                    const float s[2] = { 2.f / viewport[2], 2.f / viewport[3] };
                    auto r = Quat<float>(M.mQRotate)*Quat<float>::trackball(x*s[0] - 1, 1 - y*s[1], s[0]*mousePos[0] - 1, 1 - s[1]*mousePos[1]);
                    std::copy_n(r.q, 4, M.mQRotate);
                }
            }
        }
    }

    mousePos[0] = x; mousePos[1] = y;

    glutPostRedisplay();
}


void onMouseWheel(int wheel_number, int direction, int x, int y)
{
    M.mMeshScale *= direction > 0 ? 1.1f : 0.9f;
    glutPostRedisplay();
}

int initGL(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowSize(960, 960);
    glutInitWindowPosition(200, 50);
    glutCreateWindow(argv[0]);

    // !Load the OpenGL functions. after the opengl context has been created
    if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
        return -1;

    glClearColor(1.f, 1.f, 1.f, 0.f);

    glutReshapeFunc([](int w, int h) { viewport[2] = w; viewport[3] = h; TwWindowSize(w, h); });
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyboard);
    glutMouseFunc(onMouseButton);
    glutMotionFunc(onMouseMove);
    glutMouseWheelFunc(onMouseWheel);
    glutCloseFunc([]() {exit(0); });
    return 0;
}


void createTweakbar()
{
    TwBar *bar = TwGetBarByName("MeshViewer");
    // TwBar* bonusbar = TwGetBarByName("Bonus");
    if (bar)    TwDeleteBar(bar);

    //Create a tweak bar
    bar = TwNewBar("MeshViewer");
    // bonusbar = TwNewBar("Bonus");
    TwDefine(" MeshViewer size='220 200' color='0 128 255' text=dark alpha=128 position='5 5'"); // change default tweak bar size and color
    // TwDefine(" Bonus size='220 150' color='128 0 255' text=dark alpha=128 position='5 250'");
    TwAddButton(bar, "Change mode", [](void* a) {
        _cot = !_cot;
        deform_preprocess();
        meshDeform();
        },
        &_cot, "");

    TwAddVarRO(bar, "#Vertex", TW_TYPE_INT32, &M.nVertex, " group='Mesh View'");
    TwAddVarRO(bar, "#Face", TW_TYPE_INT32, &M.nFace, " group='Mesh View'");

    TwAddVarRW(bar, "Point Size", TW_TYPE_FLOAT, &M.pointSize, " group='Mesh View' ");
    TwAddVarRW(bar, "Vertex Color", TW_TYPE_COLOR4F, M.vertexColor.data(), " group='Mesh View' help='mesh vertex color' ");


    TwAddVarRW(bar, "Edge Width", TW_TYPE_FLOAT, &M.edgeWidth, " group='Mesh View' ");
    TwAddVarRW(bar, "Edge Color", TW_TYPE_COLOR4F, M.edgeColor.data(), " group='Mesh View' help='mesh edge color' ");

    TwAddVarRW(bar, "Face Color", TW_TYPE_COLOR4F, M.faceColor.data(), " group='Mesh View' help='mesh face color' ");

    TwDefine(" MeshViewer/'Mesh View' opened=true ");

    //  TwAddVarRW(bonusbar, "Rotation", TW_TYPE_QUAT4F, myRotation, " group='Modeling' open help='can be used to specify the rotation for current handle' ");
    /* TwAddButton(bonusbar, "Update", [](void* a) {
        updatedelta();
        meshDeform();
        },
        &_cot, "");*/
}

int main(int argc, char *argv[])
{
    if (initGL(argc, argv)) {
        fprintf(stderr, "!Failed to initialize OpenGL!Exit...");
        exit(-1);
    }

    MyMesh::buildShaders();

    std::vector<float> x, nx;
    std::vector<int> f, nf;

    const char* meshpath = argc > 1 ? argv[1] : "balls.obj";
    readObj(meshpath, x, f);
    readObj("newball.obj", nx, nf);

    meshX = Eigen::Map<MatX3f>(x.data(), x.size() / 3, 3);
    nmeshX = Eigen::Map<MatX3f>(nx.data(), nx.size() / 3, 3);
    meshF = Eigen::Map<MatX3i>(f.data(), f.size() / 3, 3);


    // M.upload(x.data(), x.size() / 3, f.data(), f.size() / 3, nullptr);
    M.upload(x.data(), x.size() / 3, f.data(), f.size() / 3, (nmeshX.leftCols(2)*4).eval().data());
    M.tex.setImage(std::string("bricks2.png"));
    M.showTexture = true;

    //////////////////////////////////////////////////////////////////////////
    TwInit(TW_OPENGL_CORE, NULL);
    //Send 'glutGetModifers' function pointer to AntTweakBar;
    //required because the GLUT key event functions do not report key modifiers states.
    TwGLUTModifiersFunc(glutGetModifiers);
    glutSpecialFunc([](int key, int x, int y) { TwEventSpecialGLUT(key, x, y); glutPostRedisplay(); }); // important for special keys like UP/DOWN/LEFT/RIGHT ...
    TwCopyStdStringToClientFunc([](std::string& dst, const std::string& src) {dst = src; });

    createTweakbar();

    //////////////////////////////////////////////////////////////////////////
    atexit([] { TwDeleteAllBars();  TwTerminate(); }); 

    glutTimerFunc(1, [](int) {
        deform_preprocess();
    }, 
        0);


    //////////////////////////////////////////////////////////////////////////
    glutMainLoop();

    return 0;
}
