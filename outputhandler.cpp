#include "outputhandler.h"

OutputHandler::OutputHandler(QSlider *FR_topSliderRef,
                             QSlider *FR_botSliderRef,
                             QSlider *BL_topSliderRef,
                             QSlider *BL_botSliderRef,
                             QSlider *FL_topSliderRef,
                             QSlider *FL_botSliderRef,
                             QSlider *BR_topSliderRef,
                             QSlider *BR_botSliderRef,
                             QtCharts::QChartView *chartViewRef,
                             LoggerHandler *loggerRef)
{
    chartView = chartViewRef;
    logger = loggerRef;
    FR_topSlider = FR_topSliderRef;
    FR_botSlider = FR_botSliderRef;
    BL_topSlider = BL_topSliderRef;
    BL_botSlider = BL_botSliderRef;

    FL_topSlider = FL_topSliderRef;
    FL_botSlider = FL_botSliderRef;
    BR_topSlider = BR_topSliderRef;
    BR_botSlider = BR_botSliderRef;
    detailLevel = SettingsConstants::DETAILED_INFO;
    axisX = new QtCharts::QCategoryAxis();
    axisY = new QtCharts::QCategoryAxis();
    FRBLSeries = new QtCharts::QLineSeries();
    FLBRSeries = new QtCharts::QLineSeries();
    dirSeries = new QtCharts::QLineSeries();
    chart = new QtCharts::QChart();
    configurePenBrushFont();
    configureAxis();
    configureSeries();
    configureChart();
    updateChart(0,0,0,0);
}


/**
 * @brief Configures all brushes used to style the chart.
 */
void OutputHandler::configurePenBrushFont()
{
    axisYPen = new QPen(QBrush(QRgb(0x5E5E6F)), 1,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    axisXPen = new QPen(QBrush(QRgb(0x303046)), 1,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    axisLabelFont = new QFont("Open Sans", 12);
    axisLabelPenBrush = new QBrush(QRgb(0xA3A3AD));

    // TODO setup a way to grab gradient from somwhere without having to
    // generate a new one every time
    QLinearGradient lightPinkBruise_Gradient(QPointF(0, 0), QPointF(1, 0));
    lightPinkBruise_Gradient.setColorAt(0.0, QRgb(0xDD3CFD));
    lightPinkBruise_Gradient.setColorAt(1.0, QRgb(0xFF6F7A));
    lightPinkBruise_Gradient.setCoordinateMode(QGradient::StretchToDeviceMode);

    QLinearGradient darkUltramarine_Gradient(QPointF(0, 0),QPointF(1, 0));
    darkUltramarine_Gradient.setColorAt(0.0, QRgb(0x7517F8));
    darkUltramarine_Gradient.setColorAt(1.0, QRgb(0x02A4FF));
    darkUltramarine_Gradient.setCoordinateMode(QGradient::StretchToDeviceMode);

    FRBLPen = new QPen(lightPinkBruise_Gradient, 5,
                       Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    FLBRPen = new QPen(darkUltramarine_Gradient, 5,
                       Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QBrush dirPenBrush = QBrush(QRgb(0xFFFFFF));
    dirPen = new QPen(dirPenBrush, 3,
                      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

}


/**
 * @brief Configures all needed axis with all the needed styling
 * and settings.
 */
void OutputHandler::configureAxis()
{
    axisY->setLabelsFont(*axisLabelFont);
    axisY->setLabelsBrush(*axisLabelPenBrush);
    axisY->setLinePen(*axisYPen);
    axisY->setGridLinePen(*axisXPen);
    axisX->setGridLineVisible(false);

    //Axis data
    axisY->append("-1.00 ", IOConstants::MIN);
    axisY->append("0 ", 0);
    axisY->append("1.00 ", IOConstants::MAX);
    axisY->setLabelsPosition(QtCharts::
                                 QCategoryAxis::AxisLabelsPositionOnValue);
    // + and - are padding around max numbers shown
    axisY->setRange(IOConstants::MIN-0.1, IOConstants::MAX+0.1);
    axisX->setRange(IOConstants::MIN_XCHART-0.3, IOConstants::MAX_XCHART+0.3);
}


/**
 * @brief Configures all needed series with all the needed styling
 * and settings.
 */
void OutputHandler::configureSeries()
{
    FRBLSeries->setPen(*FRBLPen);
    FLBRSeries->setPen(*FLBRPen);
    dirSeries->setPen(*dirPen);
}


/**
 * @brief Configures the specified chart with all the needed styling
 * and settings.
 */
void OutputHandler::configureChart()
{
    configureAxis();
    // Adding details
    chart->legend()->hide();
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->setBackgroundVisible(false);
    // Removing unneeded space around chart
    chart->setMargins(QMargins(0, 0, 0, -20)); //-20 removes unnessary x-axis
    chart->setBackgroundRoundness(0);
    chartView->setChart(chart);
    chartView->setStyleSheet(NULL);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->chart()->addSeries(FRBLSeries);
    chartView->chart()->addSeries(FLBRSeries);
    chartView->chart()->addSeries(dirSeries);
    FRBLSeries->attachAxis(axisX);
    FRBLSeries->attachAxis(axisY);
    FLBRSeries->attachAxis(axisX);
    FLBRSeries->attachAxis(axisY);
    dirSeries->attachAxis(axisX);
    dirSeries->attachAxis(axisY);
}


/**
 * @brief Generates a set number of data points of a modified sine function. The
 * modified sine function is the basis of the kinematics for a mechanum drive
 * system.
 * @param int number of data points in array generated.
 * @param double cycles from start to finsh.
 * @param double amplitude of sine.
 * @param double up and down offset.
 * @param double left and right offset.
 * @param double magnitude of force (how fast).
 * @param double z coordinate of input.
 * @param double value that is used for normalization.
 * @return double array of pointers pointing to data points.
 */
double** generateSinePointsKinematics(int numberOfPoints, double cycles, double amp, double yOffset, double xOffset, double mag, double z, double scale) {
    double y = 0.0;
    double f = cycles/double(numberOfPoints-1);
    double** arr = new double*[numberOfPoints];
    for (int i = 0; i < numberOfPoints; i++) {
        arr[i] = new double[2];
    }

    for (int t=0; t<(numberOfPoints); t++) {
        //y = (roundf(((((amp * sin (2 * 3.14159 * f * t + xOffset) + yOffset) * mag) + z)/scale) * 100000) / 100000.0);
        y = std::clamp((roundf(((((amp * sin (2 * 3.14159 * f * t + xOffset) + yOffset) * mag) + z)/scale) * 100000) / 100000.0), IOConstants::MIN, IOConstants::MAX);
        arr[(t)][0] = t+1;
        arr[(t)][1] = y;
    }
    return arr;
}


/**
 * @brief Updates sliders on GUI to repersent FR/BL and FL/BR values. Function
 * is called any time a kinematics value is updated or changed.
 */
void OutputHandler::updateSliders(double FRSpeed, double BLSpeed, double FLSpeed, double BRSpeed)
{
    setFRSlider(FRSpeed);
    setBLSlider(BLSpeed);
    setFLSlider(FLSpeed);
    setBRSlider(BRSpeed);
}


//TODO this function needs heavy performance fixes
//TODO add option in settings to adjust detail level and ammount of data points
/**
 * @brief Main update function that calls all he needs functions for updating
 * the graph to new data that has been sent. Performance is directly connected
 * connected to how many points per update need to be generated.
 * @param double direction of force.
 * @param double magnitude of force (how fast).
 * @param double z coordinate of input.
 * @param double value that is used for normalization.
 */
void OutputHandler::updateChart(double dir,
                                double mag,
                                double z,
                                double scaleFactor)
{
    logger->write(LoggerConstants::DEBUG, "Updating kinematics chart ...");
    // Generate series showing vertical line where speeds are currently getting
    // fetched from.
    if (dir < 0.0) { dir = dir + (2 * MathConstants::PI); }
    dir = linearMap(dir,
                    0,
                    (2 * MathConstants::PI),
                    IOConstants::MIN_XCHART,
                    IOConstants::MAX_XCHART);
    dirSeries->clear();
    dirSeries->append(dir, IOConstants::MAX+.02);
    dirSeries->append(dir, IOConstants::MIN-.02);

    if (scaleFactor == 0) { scaleFactor = 1.0; }

    double x, y = -9.9;
    // TODO Potentially switch over to using vectors? Statically allocated
    // arrays seems fine in this case as the array size does not change after
    // compile timer.
    double** FRBLarrPtr;
    double** FLBRarrPtr;

    // Show line at dir
    switch (getCurrentDetailLevel()) {
        case SettingsConstants::BASIC_INFO:
            // Generate data of wave with mag, and scale for FRBL
            FRBLarrPtr = generateSinePointsKinematics(IOConstants::MAX_XCHART,
                                                      1.0,
                                                      1.0,
                                                      0.0,
                                                      (-(MathConstants::PI/4)),
                                                      mag,
                                                      0.0,
                                                      scaleFactor);
            FRBLSeries->clear();
            for (int i=0; i<IOConstants::MAX_XCHART; i++) {
                for (int j=0; j<2; j++) {
                    if (j == 0) {
                        x = FRBLarrPtr[i][j];
                    } else {
                        y = FRBLarrPtr[i][j];
                    }
                }
                //qDebug() << "FRBL" << x << y;
                // Add new generated data
                FRBLSeries->append(x, y);
            }
            // Clean up memory used by array
            for(int i=0; i<IOConstants::MAX_XCHART; i++) {
                delete[] FRBLarrPtr[i];
            }
            delete[] FRBLarrPtr;

            // Generate data of wave with mag, and scale for FLBR
            FLBRarrPtr = generateSinePointsKinematics(IOConstants::MAX_XCHART,
                                                      1.0,
                                                      1.0,
                                                      0.0,
                                                      ((MathConstants::PI/4)),
                                                      mag,
                                                      0.0,
                                                      scaleFactor);
            FLBRSeries->clear();
            for (int i=0; i<IOConstants::MAX_XCHART; i++) {
                for (int j=0; j<2; j++) {
                    if (j == 0) {
                        x = FLBRarrPtr[i][j];
                    } else {
                        y = FLBRarrPtr[i][j];
                    }
                }
                //qDebug() << "FRBL" << x << y;
                FLBRSeries->append(x, y);
            }
            // Clean up memory used by array
            for(int i=0; i<IOConstants::MAX_XCHART; i++) {
                delete[] FLBRarrPtr[i];
            }
            delete[] FLBRarrPtr;
            break;
        case SettingsConstants::DETAILED_INFO:
            // Generate data of wave with mag, z, and scale for FRBL
            FRBLarrPtr = generateSinePointsKinematics(IOConstants::MAX_XCHART,
                                                      1.0,
                                                      1.0,
                                                      0.0,
                                                      (-(MathConstants::PI/4)),
                                                      mag,
                                                      z,
                                                      scaleFactor);
            FRBLSeries->clear();
            for (int i=0; i<IOConstants::MAX_XCHART; i++) {
                for (int j=0; j<2; j++) {
                    if (j == 0) {
                        x = FRBLarrPtr[i][j];
                    } else {
                        y = FRBLarrPtr[i][j];
                    }
                }
                //qDebug() << "FRBL" << x << y;
                FRBLSeries->append(x, y);
            }
            // Clean up memory used by array
            for(int i=0; i<IOConstants::MAX_XCHART; i++) {
                delete[] FRBLarrPtr[i];
            }
            delete[] FRBLarrPtr;
            // Generate data of wave with mag, z, and scale for FLBR
            FLBRarrPtr = generateSinePointsKinematics(IOConstants::MAX_XCHART,
                                                      1.0,
                                                      1.0,
                                                      0.0,
                                                      ((MathConstants::PI/4)),
                                                      mag,
                                                      z,
                                                      scaleFactor);
            FLBRSeries->clear();
            for (int i=0; i<IOConstants::MAX_XCHART; i++) {
                for (int j=0; j<2; j++) {
                    if (j == 0) {
                        x = FLBRarrPtr[i][j];
                    } else {
                        y = FLBRarrPtr[i][j];
                    }
                }
                //qDebug() << "FRBL" << x << y;
                FLBRSeries->append(x, y);
            }
            // Clean up memory used by array
            for(int i=0; i<IOConstants::MAX_XCHART; i++) {
                delete[] FLBRarrPtr[i];
            }
            delete[] FLBRarrPtr;
            break;
    }
}


// Getters

/**
 * @brief Gets current detail level that the graphing is set at.
 * @return int value of enum that detail is set at.
 */
int OutputHandler::getCurrentDetailLevel()
{
    return detailLevel;
}


// Setters

// TODO Add wrong input checking
/**
 * @brief Sets level of detail for graphing points of the kinematics.
 * @param int level as a constant from SettingsConstants choices.
 */
void OutputHandler::setDetailLevel(int level)
{
    detailLevel = level;
}


/**
 * @brief Sets value of FRBL slider scaled to fit.
 * @param double value between IOConstants::MIN and IOConstants::MAX.
 */
void OutputHandler::setFRSlider(double value)
{
    double amplifiedFR = value * 100.0;
    if (amplifiedFR > 0.0) { // positive
        FR_botSlider->setValue(0.0);
        if (amplifiedFR <= IOConstants::MIN_SLIDER) {
            FR_topSlider->setValue(IOConstants::MIN_SLIDER);
        }  else {
            FR_topSlider->setValue(amplifiedFR);
        }
    } else if (amplifiedFR < 0.0) { // negative
        FR_topSlider->setValue(0.0);
        if (amplifiedFR >= -IOConstants::MIN_SLIDER) {
            FR_botSlider->setValue(-IOConstants::MIN_SLIDER);
        }  else {
            FR_botSlider->setValue(amplifiedFR);
        }
    } else { // zero
        FR_botSlider->setValue(0.0);
        FR_topSlider->setValue(0.0);
    }
}


/**
 * @brief Sets value of FRBL slider scaled to fit.
 * @param double value between IOConstants::MIN and IOConstants::MAX.
 */
void OutputHandler::setBLSlider(double value)
{
    double amplifiedBL = value * 100.0;
    if (amplifiedBL > 0.0) { // positive
        BL_botSlider->setValue(0.0);
        if (amplifiedBL <= IOConstants::MIN_SLIDER) {
            BL_topSlider->setValue(IOConstants::MIN_SLIDER);
        }  else {
            BL_topSlider->setValue(amplifiedBL);
        }
    } else if (amplifiedBL < 0.0) { // negative
        BL_topSlider->setValue(0.0);
        if (amplifiedBL >= -IOConstants::MIN_SLIDER) {
            BL_botSlider->setValue(-IOConstants::MIN_SLIDER);
        }  else {
            BL_botSlider->setValue(amplifiedBL);
        }
    } else { // zero
        BL_botSlider->setValue(0.0);
        BL_topSlider->setValue(0.0);
    }
}


/**
 * @brief Sets value of FLBR slider scaled to fit.
 * @param double value between IOConstants::MIN and IOConstants::MAX.
 */
void OutputHandler::setFLSlider(double value)
{
    double amplifiedFL = value * 100.0;
    if (amplifiedFL > 0.0) { // positive
        FL_botSlider->setValue(0.0);
        if (amplifiedFL <= IOConstants::MIN_SLIDER) {
            FL_topSlider->setValue(IOConstants::MIN_SLIDER);
        }  else {
            FL_topSlider->setValue(amplifiedFL);
        }
    } else if (amplifiedFL < 0.0) { // negative
        FL_topSlider->setValue(0.0);
        if (amplifiedFL >= -IOConstants::MIN_SLIDER) {
            FL_botSlider->setValue(-IOConstants::MIN_SLIDER);
        }  else {
            FL_botSlider->setValue(amplifiedFL);
        }
    } else { // zero
        FL_botSlider->setValue(0.0);
        FL_topSlider->setValue(0.0);
    }
}


/**
 * @brief Sets value of FLBR slider scaled to fit.
 * @param double value between IOConstants::MIN and IOConstants::MAX.
 */
void OutputHandler::setBRSlider(double value)
{
    double amplifiedBR = value * 100.0;
    if (amplifiedBR > 0.0) { // positive
        BR_botSlider->setValue(0.0);
        if (amplifiedBR <= IOConstants::MIN_SLIDER) {
            BR_topSlider->setValue(IOConstants::MIN_SLIDER);
        }  else {
            BR_topSlider->setValue(amplifiedBR);
        }
    } else if (amplifiedBR < 0.0) { // negative
        BR_topSlider->setValue(0.0);
        if (amplifiedBR >= -IOConstants::MIN_SLIDER) {
            BR_botSlider->setValue(-IOConstants::MIN_SLIDER);
        }  else {
            BR_botSlider->setValue(amplifiedBR);
        }
    } else { // zero
        BR_botSlider->setValue(0.0);
        BR_topSlider->setValue(0.0);
    }
}
