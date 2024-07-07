#pragma once

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <list>

// Определение enum для классов дефектов
enum class DefectType {
    Seam,       // Шов
    HangingString,  // Висячая нить
    Dissection, // Рассечка
    Blisna,     // Близна
    ThreadSpan, // Пролет
    DoubleThreadY,  // Двойная нить по основе 
    DoubleThreadX,  // Двойная нить по утку 
    ViolationOfWeaving, // Нарушение ткацкого переплетения 
    StuffedFluff,   // Редкое место (Затканный пух) 
    LightStrip, // Забоина (Светлая полоса)
    Fold,       // Складка
    Crease,     // Залом
    WaterLeak,  // Затек воды
    Spot,       // Пятно
    Contamination,  // Засоренность
    Knot,       // Узел
    Thickening, // Слет (утолщенное место)
    ThreadThickeningY,  // Утолщение нити по основе
    ThreadThickeningX,  // Утолщение нити по утку 
    DifferentThreadY,   // Отличающаяся нить по основе
    DifferentThreadX,   // Отличающаяся нить по утку 
    IncompleteDoubleThread, // Недолет
    SparseThread,   // Недосека (разреженное расположение)
    Default
};

// Структура, описывающая один найденный дефект
struct DetectResult
{
    cv::Rect2i rect;        // Прямоугольник(рамка) с координатами и размерами дефекта на изображении (в пикселях)
    // x, y - координата левого верхнего угла рамки
    // width, height - ширина и высота рамки

    float prob;             // Точность распознания дефекта (от 0.0 до 1.0)

    cv::Mat mask;           // Бинарная маска дефекта.
    // Имеет тот же размер, что и rect. Формат CV_8UC1.
    // Белые пиксели - дефектная область, черные - не дефектная

    int64_t klass;          // Класс дефекта
};


// Структура, описывающая все найденные дефекты на куске(батче) изображения
struct BatchResult
{
    cv::Rect2i batchRect;               // рамка с координатами и размерами батча
    // x, y - координата левого верхнего угла батча относительно левого верхнего угла изображения
    // width, height - ширина и высота батча

    std::list<DetectResult> detects;    // Список дефектов
};

