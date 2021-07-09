#include "simulationhandler.h"

// Coordinate system
//           | y+
//           |
//  x+       |
//  ---------+
//          /
//         /
//        / z+
//

// Constructor
SimulationHandler::SimulationHandler(LoggerHandler *loggerRef,
                                     QSettings *settingsRef)
{
    logger = loggerRef;
    settings = settingsRef;
    loadedMeshesCount = 0;
    expectedLoadedMeshes = 0;
    root = new Qt3DCore::QEntity();
    view = new Qt3DExtras::Qt3DWindow();    

    simulationWidget = QWidget::createWindowContainer(view);
    simulationWidget->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);
    setup3DView();



    // Materials
    Qt3DExtras::QDiffuseSpecularMaterial *gridMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    gridMaterial->setAmbient(QColor(255,255,255));
    gridMaterial->setAlphaBlendingEnabled(true);
    gridMaterial->setDiffuse(QColor(255,255,255,128));
    gridMaterial->setShininess(0.0f);

    Qt3DExtras::QDiffuseSpecularMaterial *innerBaseMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    innerBaseMaterial->setAmbient(QColor(226,35,255));
    innerBaseMaterial->setAlphaBlendingEnabled(true);
    innerBaseMaterial->setDiffuse(QColor(226,35,255,128));
    innerBaseMaterial->setShininess(0.0f);

    Qt3DExtras::QDiffuseSpecularMaterial *frameMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    frameMaterial->setAmbient(QColor(255,255,255));
    frameMaterial->setDiffuse(QColor(255,255,255));
    frameMaterial->setShininess(0.0f);

    Qt3DExtras::QDiffuseSpecularMaterial *arrowMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    arrowMaterial->setAmbient(QColor(226,35,255));
    arrowMaterial->setDiffuse(QColor(226,35,255));
    arrowMaterial->setShininess(0.0f);

    Qt3DExtras::QDiffuseSpecularMaterial *FRBLMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    FRBLMaterial->setAmbient(QColor(232,77,209));
    FRBLMaterial->setDiffuse(QColor(232,77,209));
    FRBLMaterial->setShininess(0.0f);

    Qt3DExtras::QDiffuseSpecularMaterial *FLBRMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    FLBRMaterial->setAmbient(QColor(79,70,250));
    FLBRMaterial->setDiffuse(QColor(79,70,250));
    FLBRMaterial->setShininess(0.0f);

    FRWheel = generateWheel(9,
                            SimulationConstants::WHEEL_WIDTH,
                            SimulationConstants::WHEEL_DIAMETER,
                            SimulationConstants::FRAME_THICKNESS,
                            SimulationConstants::WHEEL_RIGHT,
                            frameMaterial);

    BLWheel = generateWheel(9,
                            SimulationConstants::WHEEL_WIDTH,
                            SimulationConstants::WHEEL_DIAMETER,
                            SimulationConstants::FRAME_THICKNESS,
                            SimulationConstants::WHEEL_RIGHT,
                            frameMaterial);

    FLWheel = generateWheel(9,
                            SimulationConstants::WHEEL_WIDTH,
                            SimulationConstants::WHEEL_DIAMETER,
                            SimulationConstants::FRAME_THICKNESS,
                            SimulationConstants::WHEEL_LEFT,
                            frameMaterial);

    BRWheel = generateWheel(9,
                            SimulationConstants::WHEEL_WIDTH,
                            SimulationConstants::WHEEL_DIAMETER,
                            SimulationConstants::FRAME_THICKNESS,
                            SimulationConstants::WHEEL_LEFT,
                            frameMaterial);

    baseFrame = generateFrame(SimulationConstants::INBASE_LENGTH,
                              SimulationConstants::INBASE_WIDTH,
                              SimulationConstants::FRAME_THICKNESS,
                              frameMaterial,
                              innerBaseMaterial);

    generateGrid(SimulationConstants::GRID_WIDTH,
                 gridMaterial);

    arrowR = generateArrow(true, false, arrowMaterial);
    arrowL = generateArrow(true, true, arrowMaterial);
    arrow = generateArrow(false, false, arrowMaterial);

    Qt3DCore::QTransform *baseTransform = new Qt3DCore::QTransform();
    baseTransform->setTranslation(QVector3D(0.0f,
                                            SimulationConstants::WHEEL_DIAMETER/2 + SimulationConstants::FRAME_THICKNESS,
                                            0.0f));
    baseFrame->addComponent(baseTransform);

    Qt3DCore::QTransform *FRTransform = new Qt3DCore::QTransform();
    FRTransform->setTranslation(QVector3D(-SimulationConstants::INBASE_WIDTH/2 - SimulationConstants::WHEEL_WIDTH/2,
                                          SimulationConstants::WHEEL_DIAMETER/2 + SimulationConstants::FRAME_THICKNESS,
                                          SimulationConstants::INBASE_LENGTH/2));
    FRWheel->addComponent(FRTransform);

    Qt3DCore::QTransform *BLTransform = new Qt3DCore::QTransform();
    BLTransform->setTranslation(QVector3D(SimulationConstants::INBASE_WIDTH/2 + SimulationConstants::WHEEL_WIDTH/2,
                                          SimulationConstants::WHEEL_DIAMETER/2 + SimulationConstants::FRAME_THICKNESS,
                                          -SimulationConstants::INBASE_LENGTH/2));
    BLWheel->addComponent(BLTransform);

    Qt3DCore::QTransform *FLTransform = new Qt3DCore::QTransform();
    FLTransform->setTranslation(QVector3D(SimulationConstants::INBASE_WIDTH/2 + SimulationConstants::WHEEL_WIDTH/2,
                                          SimulationConstants::WHEEL_DIAMETER/2 + SimulationConstants::FRAME_THICKNESS,
                                          SimulationConstants::INBASE_LENGTH/2));
    FLWheel->addComponent(FLTransform);

    Qt3DCore::QTransform *BRTransform = new Qt3DCore::QTransform();
    BRTransform->setTranslation(QVector3D(-SimulationConstants::INBASE_WIDTH/2 - SimulationConstants::WHEEL_WIDTH/2,
                                          SimulationConstants::WHEEL_DIAMETER/2 + SimulationConstants::FRAME_THICKNESS,
                                          -SimulationConstants::INBASE_LENGTH/2));
    BRWheel->addComponent(BRTransform);

    arrowTransform = new Qt3DCore::QTransform();
    arrowTransform->setTranslation(QVector3D(0.0f,
                                              SimulationConstants::WHEEL_DIAMETER/2 + SimulationConstants::FRAME_THICKNESS,
                                              0.0f));
    arrow->addComponent(arrowTransform);


    arrowRTransform = new Qt3DCore::QTransform();
    arrowR->addComponent(arrowRTransform);

    arrowLTransform = new Qt3DCore::QTransform();
    arrowL->addComponent(arrowLTransform);

    view->setRootEntity(root);
}

//Grab keyboard and send it over to the input handler
void SimulationHandler::setup3DView() {
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x05050f)));
    Qt3DRender::QFrameGraphNode *framegraph = view->activeFrameGraph();
    Qt3DRender::QSortPolicy *sortPolicy = new Qt3DRender::QSortPolicy(root);
    framegraph->setParent(sortPolicy);
    QVector<Qt3DRender::QSortPolicy::SortType> sortTypes =
      QVector<Qt3DRender::QSortPolicy::SortType>() << Qt3DRender::QSortPolicy::BackToFront;
    sortPolicy->setSortTypes(sortTypes);
    view->setActiveFrameGraph(framegraph);

    Qt3DRender::QCamera *camera = view->camera();
    camera->setPosition(QVector3D(-10.0f, 9.0f, 10.0f));
    camera->setUpVector(QVector3D(0, 1.0f, 0));
    camera->setViewCenter(QVector3D(0, 0, 0));
    camera->setFieldOfView(45);

    // Disable light
    Qt3DRender::QDirectionalLight *light = new Qt3DRender::QDirectionalLight();
    light->setColor("white");
    light->setIntensity(0.0);

    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(root);
    lightEntity->addComponent(light);

    // For camera controls
    Qt3DExtras::QOrbitCameraController *camController = new Qt3DExtras::QOrbitCameraController(root);
    camController->setCamera(camera);

}

Qt3DCore::QEntity* SimulationHandler::generateArrow(bool curved,
                                                    bool mirrorCurve,
                                                    Qt3DExtras::QDiffuseSpecularMaterial *arrowMaterial) {
    Qt3DCore::QEntity *arrowEntity = new Qt3DCore::QEntity(root);
    Qt3DRender::QMesh *arrowMesh = new Qt3DRender::QMesh();
    connect(arrowMesh,
            &Qt3DRender::QMesh::statusChanged,
            this,
            &SimulationHandler::checkLoaded);
    if (curved && !mirrorCurve) {
        arrowMesh->setSource(QUrl("qrc:/obj/resources/Arrow-Right-Curved.obj"));
    } else if (curved && mirrorCurve) {
        arrowMesh->setSource(QUrl("qrc:/obj/resources/Arrow-Right-Curved-M.obj"));
    } else {
        arrowMesh->setSource(QUrl("qrc:/obj/resources/Arrow-Right.obj"));
    }
    expectedLoadedMeshes++;

    arrowEntity->addComponent(arrowMesh);
    arrowEntity->addComponent(arrowMaterial);

    return arrowEntity;
}


void SimulationHandler::generateGrid(double size,
                                     Qt3DExtras::QDiffuseSpecularMaterial *gridMaterial) {

    Qt3DCore::QEntity *baseEntity = new Qt3DCore::QEntity(root);
    generateGridLabels(size, gridMaterial);

    // Meshs
    Qt3DExtras::QPlaneMesh **gridLines = new Qt3DExtras::QPlaneMesh*[10];
    Qt3DCore::QTransform **lineTransform = new Qt3DCore::QTransform*[10];
    Qt3DCore::QEntity **lineEntity = new Qt3DCore::QEntity*[10];

    for (int i=0; i < 10; i++) {
        gridLines[i] = new Qt3DExtras::QPlaneMesh();
        gridLines[i]->setHeight(SimulationConstants::GRID_THICKNESS);
        gridLines[i]->setWidth(size+SimulationConstants::GRID_PAD);
    }

    for (int i=0; i < 10; i++) {
        lineTransform[i] = new Qt3DCore::QTransform();
    }

    // Transforms
    //Vertical
    lineTransform[0]->setRotation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 90.0f));
    lineTransform[1]->setRotation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 90.0f));
    lineTransform[1]->setTranslation(QVector3D(size/2, 0.0f, 0.0f));
    lineTransform[2]->setRotation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 90.0f));
    lineTransform[2]->setTranslation(QVector3D(-size/2, 0.0f, 0.0f));
    lineTransform[3]->setRotation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 90.0f));
    lineTransform[3]->setTranslation(QVector3D(-size/2+0.2, 0.0f, 0.0f));
    lineTransform[4]->setRotation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 90.0f));
    lineTransform[4]->setTranslation(QVector3D(size/2-0.2, 0.0f, 0.0f));

    //Horizontal
    lineTransform[5]->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
    lineTransform[6]->setTranslation(QVector3D(0.0f, 0.0f, size/2));
    lineTransform[7]->setTranslation(QVector3D(0.0f, 0.0f, -size/2));
    lineTransform[8]->setTranslation(QVector3D(0.0f, 0.0f, size/2-0.2));
    lineTransform[9]->setTranslation(QVector3D(0.0f, 0.0f, -size/2+0.2));

    //Entities
    for (int i=0; i < 10; i++) {
        lineEntity[i] = new Qt3DCore::QEntity(baseEntity);
        lineEntity[i]->addComponent(gridLines[i]);
        lineEntity[i]->addComponent(lineTransform[i]);
        lineEntity[i]->addComponent(gridMaterial);
    }
}


void SimulationHandler::generateGridLabels(double size,
                                           Qt3DExtras::QDiffuseSpecularMaterial *gridMaterial) {

    Qt3DCore::QEntity *baseEntity = new Qt3DCore::QEntity(root);

    // Meshs
    Qt3DExtras::QExtrudedTextMesh **textMeshs = new Qt3DExtras::QExtrudedTextMesh*[4];
    Qt3DCore::QTransform **textTransform = new Qt3DCore::QTransform*[4];
    Qt3DCore::QEntity **textEntity = new Qt3DCore::QEntity*[4];

    for (int i=0; i < 4; i++) {
        textMeshs[i] = new Qt3DExtras::QExtrudedTextMesh();
        textMeshs[i]->setDepth(0.0);
        textMeshs[i]->setFont(QFont("Open Sans", 50));
        switch(i) {
            case 0:
                textMeshs[i]->setText("Front");
                break;
            case 1:
                textMeshs[i]->setText("Back");
                break;
            case 2:
                textMeshs[i]->setText("Right");
                break;
            case 3:
                textMeshs[i]->setText("Left");
                break;
        }
    }

    // Transforms
    for (int i=0; i < 4; i++) {
        textTransform[i] = new Qt3DCore::QTransform();
        textTransform[i]->setScale(0.5);
        textTransform[i]->setRotationX(-90);
        textTransform[i]->setRotationZ(0);
    }

    //TODO Get rid of the magic number offsets
    //Front
    textTransform[0]->setTranslation(QVector3D(-0.77f,
                                               0.0f,
                                               (size/2+0.2+0.5)));
    //Back
    textTransform[1]->setTranslation(QVector3D(-0.73f,
                                               0.0f,
                                               (-size/2-0.2)));
    //Right
    textTransform[2]->setTranslation(QVector3D((-size/2-0.2-1.55),
                                               0.0f,
                                               0.25f));
    //Left
    textTransform[3]->setTranslation(QVector3D(((size/2+0.2)),
                                               0.0f,
                                               0.25f));

    //Entities
    for (int i=0; i < 4; i++) {
        textEntity[i] = new Qt3DCore::QEntity(baseEntity);
        textEntity[i]->addComponent(textMeshs[i]);
        textEntity[i]->addComponent(textTransform[i]);
        textEntity[i]->addComponent(gridMaterial);
    }
}


Qt3DCore::QEntity* SimulationHandler::generateFrame(double baseLength,
                                                    double baseWidth,
                                                    double frameThickness,
                                                    Qt3DExtras::QDiffuseSpecularMaterial *frameMaterial,
                                                    Qt3DExtras::QDiffuseSpecularMaterial *inBaseMaterial) {
    Qt3DCore::QEntity *frameEntity = new Qt3DCore::QEntity(root);

    Qt3DExtras::QCylinderMesh **cylMeshs = new Qt3DExtras::QCylinderMesh*[4];
    Qt3DCore::QTransform **cylTransform = new Qt3DCore::QTransform*[4];
    Qt3DCore::QEntity **cylEntity = new Qt3DCore::QEntity*[4];

    Qt3DExtras::QSphereMesh **sphMeshs = new Qt3DExtras::QSphereMesh*[4];
    Qt3DCore::QTransform **sphTransform = new Qt3DCore::QTransform*[4];
    Qt3DCore::QEntity **sphEntity = new Qt3DCore::QEntity*[4];

    // Frame
    // Meshs
    for (int i=0; i < 4; i++) {
        cylMeshs[i] = new Qt3DExtras::QCylinderMesh();
        if (i >= 2) {
            cylMeshs[i]->setLength(baseLength);
        } else {
            cylMeshs[i]->setLength(baseWidth);
        }
        cylMeshs[i]->setRadius(frameThickness);
        cylMeshs[i]->setSlices(20);
        cylMeshs[i]->setRings(2);
    }

    for (int i=0; i < 4; i++) {
        sphMeshs[i] = new Qt3DExtras::QSphereMesh();
        sphMeshs[i]->setRadius(frameThickness*2);
        sphMeshs[i]->generateTangents();
    }

    for (int i=0; i < 4; i++) {
        cylTransform[i] = new Qt3DCore::QTransform();
        sphTransform[i] = new Qt3DCore::QTransform();
    }

    // Transforms
    cylTransform[0]->setRotation(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 90.0f));
    cylTransform[0]->setTranslation(QVector3D(0.0f, 0.0f, baseLength/2));
    cylTransform[1]->setRotation(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 90.0f));
    cylTransform[1]->setTranslation(QVector3D(0.0f, 0.0f, -baseLength/2));
    cylTransform[2]->setRotation(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 90.0f));
    cylTransform[2]->setTranslation(QVector3D(baseWidth/2, 0.0f, 0.0f));
    cylTransform[3]->setRotation(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 90.0f));
    cylTransform[3]->setTranslation(QVector3D(-baseWidth/2, 0.0f, 0.0f));

    sphTransform[0]->setTranslation(QVector3D(baseWidth/2, 0.0f, baseLength/2));
    sphTransform[1]->setTranslation(QVector3D(-baseWidth/2, 0.0f, baseLength/2));
    sphTransform[2]->setTranslation(QVector3D(baseWidth/2, 0.0f, -baseLength/2));
    sphTransform[3]->setTranslation(QVector3D(-baseWidth/2, 0.0f, -baseLength/2));

    // Entities
    for (int i=0; i < 4; i++) {
        cylEntity[i] = new Qt3DCore::QEntity(frameEntity);
        cylEntity[i]->addComponent(cylMeshs[i]);
        cylEntity[i]->addComponent(cylTransform[i]);
        cylEntity[i]->addComponent(frameMaterial);
    }

    for (int i=0; i < 4; i++) {
        sphEntity[i] = new Qt3DCore::QEntity(frameEntity);
        sphEntity[i]->addComponent(sphMeshs[i]);
        sphEntity[i]->addComponent(sphTransform[i]);
        sphEntity[i]->addComponent(frameMaterial);
    }

    // Inner Base
    // Meshs
    Qt3DExtras::QPlaneMesh *innerBasePlane = new Qt3DExtras::QPlaneMesh();
    innerBasePlane->setHeight(baseLength);
    innerBasePlane->setWidth(baseWidth);

    // Entities
    Qt3DCore::QEntity *innerBaseEntity = new Qt3DCore::QEntity(frameEntity);
    innerBaseEntity->addComponent(innerBasePlane);
    innerBaseEntity->addComponent(inBaseMaterial);
    return frameEntity;
}

Qt3DCore::QEntity* SimulationHandler::generateWheel(int partCount,
                                                    double wheelWidth,
                                                    double wheelDiameter,
                                                    double frameThickness,
                                                    bool invert,
                                                    Qt3DExtras::QDiffuseSpecularMaterial *wheelMaterial) {
    Qt3DCore::QEntity *wEntity = new Qt3DCore::QEntity(root);

    //Double pointer setup
    Qt3DCore::QEntity **parts = new Qt3DCore::QEntity*[partCount];

    double angleBetween = 360.0/(double)partCount;
    double currentAngle = 0.0;

    //For each part
    for (int i=0; i<partCount; i++) {
        //LW 1
        //lw1Entity
        parts[i] = new Qt3DCore::QEntity(wEntity);
        currentAngle += angleBetween;

        // Meshs
        Qt3DExtras::QCylinderMesh *wCyl = new Qt3DExtras::QCylinderMesh();
        wCyl->setLength(wheelWidth);
        wCyl->setRadius(frameThickness);
        wCyl->setSlices(20);
        wCyl->setRings(2);

        Qt3DExtras::QSphereMesh *wSph1 = new Qt3DExtras::QSphereMesh();
        wSph1->setRadius(frameThickness*2);
        wSph1->setSlices(10);
        wSph1->setRings(10);

        Qt3DExtras::QSphereMesh *wSph2 = new Qt3DExtras::QSphereMesh();
        wSph2->setRadius(frameThickness*2);
        wSph2->setSlices(10);
        wSph2->setRings(10);

        // Transforms
        Qt3DCore::QTransform *wCylTransform = new Qt3DCore::QTransform();
        wCylTransform->setRotation(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 90.0f));

        Qt3DCore::QTransform *wSph1Transform = new Qt3DCore::QTransform();
        wSph1Transform->setTranslation(QVector3D(wheelWidth/2, 0.0f, 0.0f));

        Qt3DCore::QTransform *wSph2Transform = new Qt3DCore::QTransform();
        wSph2Transform->setTranslation(QVector3D(-wheelWidth/2, 0.0f, 0.0f));

        // Entities
        Qt3DCore::QEntity *wCylEntity = new Qt3DCore::QEntity(parts[i]);
        wCylEntity->addComponent(wCyl);
        wCylEntity->addComponent(wCylTransform);
        wCylEntity->addComponent(wheelMaterial);

        Qt3DCore::QEntity *wSph1Entity = new Qt3DCore::QEntity(parts[i]);
        wSph1Entity->addComponent(wSph1);
        wSph1Entity->addComponent(wSph1Transform);
        wSph1Entity->addComponent(wheelMaterial);

        Qt3DCore::QEntity *wSph2Entity = new Qt3DCore::QEntity(parts[i]);
        wSph2Entity->addComponent(wSph2);
        wSph2Entity->addComponent(wSph2Transform);
        wSph2Entity->addComponent(wheelMaterial);

        // Rotating segments to 45 degrees around the axis of a vector pointing
        // to the center of the wheel.
        Qt3DCore::QTransform *partsTransform = new Qt3DCore::QTransform();
        double height = sin(currentAngle*MathConstants::PI/180.0);
        double width = cos(currentAngle*MathConstants::PI/180.0);
        if(invert) {
            partsTransform->setRotation(QQuaternion::fromAxisAndAngle(0.0,height,width,-45.0));
        } else {
            partsTransform->setRotation(QQuaternion::fromAxisAndAngle(0.0,height,width,45.0));
        }
        partsTransform->setTranslation(QVector3D(0, height*wheelDiameter/2, width*wheelDiameter/2));
        parts[i]->addComponent(partsTransform);
    }
    return wEntity;

}

void SimulationHandler::updateWithSettings()
{
    qDebug() << "simulation handler update";
    view->defaultFrameGraph()->setShowDebugOverlay(true);
//    view->defaultFrameGraph()->
//        setShowDebugOverlay(
//            settings->value(SettingsConstants::RENDER_VIEW_DEBUG_EN,
//                            SettingsConstants::D_RENDER_VIEW_DEBUG_EN).toBool());
}

void SimulationHandler::updateArrow(double dir, double mag, double z)
{

    if (mag > 0) {
        arrow->setEnabled(true);
        dir = dir*180.0/MathConstants::PI;
        if (dir < 0.0) {
            dir = dir + 360;
        }
        arrowTransform->setRotationY(dir);
        arrowTransform->setScale(linearMap(mag, 0.0, 1.0, 0.13, 0.4));
    } else {
        arrow->setEnabled(false);
    }

    double offset = linearMap(abs(z), 0.0, 1.0, 0.4, 1.5);
    double scale = linearMap(abs(z), 0.0, 1.0, 0.13, 0.3);
    if (z < 0) { // Left
        arrowL->setEnabled(true);
        arrowR->setEnabled(false);
        arrowLTransform->setScale(scale);
        arrowLTransform->setTranslation(QVector3D(0.6f,
                                                  SimulationConstants::WHEEL_DIAMETER/2 + SimulationConstants::FRAME_THICKNESS,
                                                  SimulationConstants::INBASE_LENGTH/2+offset));
    } else if (z > 0) { // Right
        arrowL->setEnabled(false);
        arrowR->setEnabled(true);
        arrowRTransform->setScale(scale);
        arrowRTransform->setTranslation(QVector3D(-0.6f,
                                                  SimulationConstants::WHEEL_DIAMETER/2 + SimulationConstants::FRAME_THICKNESS,
                                                  SimulationConstants::INBASE_LENGTH/2+offset));
    } else {
        arrowL->setEnabled(false);
        arrowR->setEnabled(false);
    }
}

void SimulationHandler::updateWheels(double FR, double BL, double FL, double BR)
{
    //qDebug() << FR << BL << FL << BR;
}

void SimulationHandler::checkLoaded(Qt3DRender::QMesh::Status status) {
    if (status == Qt3DRender::QMesh::Status::Ready) {
        loadedMeshesCount++;
    }
    qDebug() << loadedMeshesCount;
    if (loadedMeshesCount == expectedLoadedMeshes) {
        updateArrow(0.0, 0.0, 0.0);
    }
}

// Getters
QWidget* SimulationHandler::getWidget()
{
    return simulationWidget;
}

