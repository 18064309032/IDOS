#include <QApplication>
#include <QVTKOpenGLWindow.h>

#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    vtkNew<vtkCubeSource> cube;
    cube->SetXLength(2.0);
    cube->SetYLength(2.0);
    cube->SetZLength(2.0);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(cube->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.2, 0.4);

    QVTKOpenGLWindow window;
    window.renderWindow()->AddRenderer(renderer);
    window.resize(800, 600);
    window.show();

    return app.exec();
}
