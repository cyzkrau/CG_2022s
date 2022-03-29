//   Copyright © 2021, Renjie Chen @ USTC

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#define FREEGLUT_STATIC
#include "gl_core_3_3.h"
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

#define TW_STATIC
#include <AntTweakBar.h>

#include <vector>
#include <string>

#include "glprogram.h"
#include "MyImage.h"
#include "VAOImage.h"
#include "VAOMesh.h"


GLProgram MyMesh::prog;

MyMesh M;
int viewport[4] = { 0, 0, 1280, 960 };

bool showATB = true;

std::string imagefile = "boy.png";
std::string energyfile = "boy_saliency.png";
std::string savedir = "result.png";

MyImage img;
MyImage after;
int resize_width, resize_height;

int mousePressButton;
int mouseButtonDown;
int mousePos[2];

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, viewport[2], viewport[3]);
    M.draw(viewport);

    if (showATB) TwDraw();
    glutSwapBuffers();
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
        }
    }

    mousePos[0] = x; mousePos[1] = y;

    glutPostRedisplay();
}


void onMouseWheel(int wheel_number, int direction, int x, int y)
{
    if (glutGetModifiers() & GLUT_ACTIVE_CTRL) {
    }
    else
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

void uploadImage(const MyImage& img)
{
    int w = img.width();
    int h = img.height();
    float x[] = { 0, 0, 0, w, 0, 0, w, h, 0, 0, h, 0 };
    after = img;
    M.upload(x, 4, nullptr, 0, nullptr);

    M.tex.setImage(img);
    M.tex.setClamping(GL_CLAMP_TO_EDGE);
}


void readImage(const std::string& file)
{
    int w0 = img.width(), h0 = img.height();
    img = MyImage(file);
    uploadImage(img);
    resize_width = img.width();
    resize_height = img.height();

    if (w0 != img.width() || h0 != img.height()) M.updateBBox();
}
void saveImage() {
    after.write(savedir);
}

int themin(double x1, double x2, double x3) {
    if (x2>=0 && (x1 >= x2 || x1 <= 0)) {
        if (x2 >= x3 && x3 >= 0)return 1;
        else return 0;
    }
    if (x1 >= x3 && x3 >= 0)return 1;
    return -1;
}
std::vector<int> change_width_line(std::vector<std::vector<double>>& eng, int k=1, double punish=50) {
    int hei = eng.size(), wid = eng[0].size();
    std::vector<int>finalpath;
    for (int _ = 0; _ < k; _++) {
        std::vector<std::vector<int>>eachpath;
        std::vector<std::vector<double>>energy(eng);
        for (int i = 0; i < hei; i++)eachpath.push_back(std::vector<int>(wid));
        for (int i = hei - 2; i >= 0; i--) {
            eachpath[i][0] = themin(-1, energy[i + 1][0], energy[i + 1][1]);
            energy[i][0] += energy[i + 1][eachpath[i][0]];
            for (int j = 1; j < wid - 1; j++) {
                eachpath[i][j] = themin(energy[i + 1][j - 1], energy[i + 1][j], energy[i + 1][j + 1]);
                energy[i][j] += energy[i + 1][j + eachpath[i][j]];
            }
            eachpath[i][wid - 1] = themin(energy[i + 1][wid - 2], energy[i + 1][wid - 1], -1);
            energy[i][wid - 1] += energy[i + 1][wid - 1 + eachpath[i][wid - 1]];
        }
        int mini = 0;
        for (int i = 1; i < wid; i++)if (energy[0][i] < energy[0][mini])mini = i;
        for (int i = 0; i < hei - 1; i++) { eng[i][mini] += punish; finalpath.push_back(mini); mini = eachpath[i][mini] + mini; }
        eng[hei - 1][mini] += punish;
        finalpath.push_back(mini);
    }
    return finalpath;
}
std::vector<BYTE>tr(std::vector<std::vector<double>>& eng) {
    std::vector<double>ans;
    for (int i = 0; i < size(eng); i++)for (int j = 0; j < size(eng[i]); j++)ans.push_back(eng[i][j]);
    double m = 0;
    for (int i = 0; i < size(ans); i++)if (ans[i] > m)m = ans[i];
    std::vector<BYTE>real_ans;
    for (int i = 0; i < size(ans); i++)for (int j = 0; j < 3; j++)real_ans.push_back(BYTE(255 * ans[i] / m));
    return real_ans;
}
MyImage seamCarving(const MyImage& img, int w, int h)
{
    // TODO
    int com = img.dim(), wid = img.width(), hei = img.height();
    MyImage new_img = img;
    MyImage energy(energyfile);
    for (int i = wid; i > w; i--) {
        std::vector<int>path = change_width_line(energy.to_energy());
        new_img.decrease_width(path);
        energy.decrease_width(path);
    }
    if (w > wid)new_img.increase_width(change_width_line(energy.to_energy(), w - wid));
    if (hei != h) {
        new_img.transpose();
        energy.transpose();
        for (int i = hei; i > h; i--) {
            std::vector<int>path = change_width_line(energy.to_energy());
            new_img.decrease_width(path);
            energy.decrease_width(path);
        }
        if (h > hei)new_img.increase_width(change_width_line(energy.to_energy(), h - hei));
        new_img.transpose();
        energy.transpose();
    }
    return new_img;
}

void createTweakbar()
{
    //Create a tweak bar
    TwBar *bar = TwNewBar("Image Viewer");
    TwDefine(" 'Image Viewer' size='220 190' color='100 0 255' text=dark alpha=128 position='5 5'"); // change default tweak bar size and color

    TwAddVarRW(bar, "Scale", TW_TYPE_FLOAT, &M.mMeshScale, " min=0 step=0.1");

    TwAddVarRW(bar, "Image filename", TW_TYPE_STDSTRING, &imagefile, " ");
    TwAddButton(bar, "Read Image", [](void*) { readImage(imagefile); }, nullptr, "");

    TwAddVarRW(bar, "Resize Width", TW_TYPE_INT32, &resize_width, "group='Seam Carving' min=1 ");
    TwAddVarRW(bar, "Resize Height", TW_TYPE_INT32, &resize_height, "group='Seam Carving' min=1 ");
    TwAddButton(bar, "Run Seam Carving", [](void* img) {
        MyImage newimg = seamCarving(*(const MyImage*)img, resize_width, resize_height);
        uploadImage(newimg);
        }, 
        &img, "");

    TwAddVarRW(bar, "Savedir", TW_TYPE_STDSTRING, &savedir, "group='Save Image'");
    TwAddButton(bar, "Save Now", [](void*) { saveImage(); }, nullptr, "group='Save Image'");
}


int main(int argc, char *argv[])
{
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), { 100, 5000 });

    if (initGL(argc, argv)) {
        fprintf(stderr, "!Failed to initialize OpenGL!Exit...");
        exit(-1);
    }

    MyMesh::buildShaders();


    float x[] = { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0 };
    float uv[] = { 0, 0, 1, 0, 1, 1, 0, 1 };
    int t[] = { 0, 1, 2, 2, 3, 0 };

    M.upload(x, 4, t, 2, uv);

    //////////////////////////////////////////////////////////////////////////
    TwInit(TW_OPENGL_CORE, NULL);
    //Send 'glutGetModifers' function pointer to AntTweakBar;
    //required because the GLUT key event functions do not report key modifiers states.
    TwGLUTModifiersFunc(glutGetModifiers);
    glutSpecialFunc([](int key, int x, int y) { TwEventSpecialGLUT(key, x, y); glutPostRedisplay(); }); // important for special keys like UP/DOWN/LEFT/RIGHT ...
    TwCopyStdStringToClientFunc([](std::string& dst, const std::string& src) {dst = src; });


    createTweakbar();

    //////////////////////////////////////////////////////////////////////////
    atexit([] { TwDeleteAllBars();  TwTerminate(); });  // Called after glutMainLoop ends

    glutTimerFunc(1, [](int) { readImage(imagefile); },  0);


    //////////////////////////////////////////////////////////////////////////
    glutMainLoop();

    return 0;
}

