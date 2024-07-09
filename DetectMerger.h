#pragma once 
#include <vector> 
#include <unordered_map>
#include <iostream>
#include "DataStructs.h" 

/**
 * Функция, преобразующая дефекты из разных частей изображения в список дефектов всего изображения.
 *  Функция должна "сливать" куски одного и того же дефекта из разных частей изображения в один дефект, включающий в себя все эти куски
 *
 * @param batchesDetects - список дефектов в разных кусках (батчах) изображения.
 *    Представляет из себя массив размера m на n элементов, где m - число батчей по вертикали, n - число батчей по горизонтали, а всего батчей m * n.
 *    Батч с индексом [i][j] является соседом справа для батча [i][j-1] и соседом снизу для батча [i-1][j]
 *    Координаты как батчей, так и дефектов, указаны относительно исходного изображения
 *
 * @param resultDetects - выходной список дефектов.
 * @return
 */


 // Сопоставление ID класса дефекта с его строковым представлением
std::unordered_map<int, std::string> defectClassMapping = {
    {0, "C.2.3"}, {1, "C.2.1"}, {2, "T.1.4"}, {3, "C.2.4"},
    {4, "B.4"}, {5, "O.1.1"}, {6, "C.2.2"}, {7, "T.1.2"},
    {8, "T.2.2.2"}, {9, "C.3.1"}, {10, "T.3.1"}, {11, "B.7"},
    {12, "B.3"}, {13, "T.2.6"}, {14, "C.3.2"}, {15, "O.1.2"},
    {16, "O.2.3"}, {17, "T.1.3"}, {18, "T.1.1"}, {19, "T.2.1"},
    {20, "T.2.2.1"}, {21, "C.1.2"}, {22, "T.3.2"}, {23, "B.1"}
};

// Функция для преобразования id класса дефекта в DefectType
DefectType classifyDefect(int klass) {
    auto it = defectClassMapping.find(klass);
    if (it != defectClassMapping.end()) {
        const std::string& defectClass = it->second;
        if (defectClass == "B.7") return DefectType::Seam;  // Шов
        else if (defectClass == "T.1.1") return DefectType::Blisna;     // Близна
        else if (defectClass == "T.1.2") return DefectType::ThreadSpan; // Пролет
        else if (defectClass == "T.1.3") return DefectType::DoubleThreadY;  // Двойная нить по основе 
        else if (defectClass == "T.1.4") return DefectType::DoubleThreadX;  // Двойная нить по утку 
        else if (defectClass == "T.2.1") return DefectType::ViolationOfWeaving; // Нарушение ткацкого переплетения 
        else if (defectClass == "T.2.2.1") return DefectType::Dissection;   // Рассечка
        else if (defectClass == "T.2.2.2") return DefectType::StuffedFluff; // Редкое место (Затканный пух) 
        else if (defectClass == "T.2.6") return DefectType::IncompleteDoubleThread; // Недолет
        else if (defectClass == "T.3.1") return DefectType::SparseThread;   // Недосека (разреженное расположение)
        else if (defectClass == "T.3.2") return DefectType::LightStrip;     // Забоина (Светлая полоса)
        else if (defectClass == "O.1.1") return DefectType::Fold;   // Складка
        else if (defectClass == "O.1.2") return DefectType::Crease; // Залом
        else if (defectClass == "O.2.3") return DefectType::HangingString;  // Висячая нить
        else if (defectClass == "B.3") return DefectType::WaterLeak;    // Затек воды
        else if (defectClass == "B.4") return DefectType::Spot;     // Пятно
        else if (defectClass == "C.1.2") return DefectType::Contamination;  // Засоренность
        else if (defectClass == "C.2.1") return DefectType::Knot;   // Узел
        else if (defectClass == "C.2.2") return DefectType::Thickening; // Слет (утолщенное место)
        else if (defectClass == "C.2.3") return DefectType::ThreadThickeningY;  // Утолщение нити по основе 
        else if (defectClass == "C.2.4") return DefectType::ThreadThickeningX;  // Утолщение нити по утку 
        else if (defectClass == "C.3.1") return DefectType::DifferentThreadY;   // Отличающаяся нить по основе  
        else if (defectClass == "C.3.2") return DefectType::DifferentThreadX;   // Отличающаяся нить по утку 
    }
    return DefectType::Default;
}

cv::Mat mergeMasks(const cv::Mat& m1, const cv::Mat& m2, const cv::Rect2i& r1, const cv::Rect2i& r2)
{
    // Определение объединенного прямоугольника, который охватывает оба дефекта
    cv::Rect2i mergedRect = r1 | r2;

    cv::Mat mergedMask = cv::Mat::zeros(mergedRect.size(), CV_8UC1);

    // Создание белой маски размером объединенного прямоугольника
    //cv::Mat mergedMask = cv::Mat::ones(mergedRect.size(), CV_8UC1) * 255;

    // Вычисление позиций r1 и r2 относительно объединенного прямоугольника
    cv::Rect2i r1_in_merged = cv::Rect2i(r1.x - mergedRect.x, r1.y - mergedRect.y, r1.width, r1.height);
    cv::Rect2i r2_in_merged = cv::Rect2i(r2.x - mergedRect.x, r2.y - mergedRect.y, r2.width, r2.height);

    // Копирование масок m1 и m2 в объединенную маску в соответствующих позициях
    m1.copyTo(mergedMask(r1_in_merged));
    m2.copyTo(mergedMask(r2_in_merged));

    return mergedMask;
}


void horizontalDefect(const DetectResult& defect, std::vector<DetectResult>& horizontalMergedDefectsOneType)
{
    bool mergedHor = false;
    for (auto& mergedDefect : horizontalMergedDefectsOneType)
    {
        // Проверяем, пересекаются ли дефекты по вертикали
        if ((defect.rect.y < mergedDefect.rect.y + mergedDefect.rect.height) &&
            (mergedDefect.rect.y < defect.rect.y + defect.rect.height))
        {
            // Объединяем дефекты
            cv::Rect2i newRect = mergedDefect.rect | defect.rect;
            cv::Mat newMask = mergeMasks(mergedDefect.mask, defect.mask, mergedDefect.rect, defect.rect);

            mergedDefect.rect = newRect;
            mergedDefect.mask = newMask;
            mergedDefect.prob = std::max(mergedDefect.prob, defect.prob); // берем максимальную вероятность
            mergedHor = true;
            break;
        }
    }
    if (!mergedHor) {
        horizontalMergedDefectsOneType.push_back(defect);
    }
}



void verticalDefectHorizontally(const DetectResult& defect, std::vector<DetectResult>& verticalMergedDefectsOneType) {
    bool merged = false;

    for (auto& mergedDefect : verticalMergedDefectsOneType) {
        // Расширяем рамку одного из дефектов для проверки окрестности
        cv::Rect2i expandedRect = defect.rect;

        expandedRect.x = defect.rect.x - 10;

        // Проверяем, пересекаются ли расширенные дефекты
        if (mergedDefect.rect.x + mergedDefect.rect.width >= expandedRect.x &&
            mergedDefect.rect.x <= defect.rect.x + defect.rect.width){

            // Объединяем дефекты
            cv::Rect2i newRect = mergedDefect.rect | defect.rect;
            cv::Mat newMask = mergeMasks(mergedDefect.mask, defect.mask, mergedDefect.rect, defect.rect);

            mergedDefect.rect = newRect;
            mergedDefect.mask = newMask;
            mergedDefect.prob = std::max(mergedDefect.prob, defect.prob); // берем максимальную вероятность
            merged = true;
            break;
        }
    }

    // Если дефект не был объединен ни с одним из существующих, добавляем его в список
    if (!merged) {
        verticalMergedDefectsOneType.emplace_back(defect);
    }
}



void verticalDefect(const DetectResult& defect, std::vector<DetectResult>& verticalMergedDefectsOneType)
{
    bool mergedVer = false;
    for (auto& mergedDefect : verticalMergedDefectsOneType)
    {
        // Расширяем рамку одного из дефектов для проверки окрестности
        cv::Rect2i expandedRect = defect.rect;
        // Расширяем рамку, не вычитая -10, если x и y равны 0
        expandedRect.x = (defect.rect.x < 10) ? 0 : defect.rect.x - 10;
        expandedRect.y = (defect.rect.y < 10) ? 0 : defect.rect.y - 10;
        expandedRect.width += 20;
        expandedRect.height += 20;

        // Вычисляем пересечение расширенной рамки с текущим дефектом
        cv::Rect2i intersectionRect = expandedRect & mergedDefect.rect;

        // Проверяем пересечение расширенных рамок
        if (intersectionRect.area() > 0)
        {
            // Объединяем текущий дефект с уже найденным
            cv::Rect2i newRect = mergedDefect.rect | defect.rect;
            cv::Mat newMask = mergeMasks(mergedDefect.mask, defect.mask, mergedDefect.rect, defect.rect);

            mergedDefect.rect = newRect;
            mergedDefect.mask = newMask;
            mergedDefect.prob = std::max(mergedDefect.prob, defect.prob); // берем максимальную вероятность
            mergedVer = true;
            break;
        }
    }

    // Если вертикально не объединяется, то записываем в результат
    if (!mergedVer) {
        verticalMergedDefectsOneType.push_back(defect);
    }
}


const int rectExtension = 10;

bool checkForRealDefectsInIntersection(const DetectResult& defect, const DetectResult& mergedDefect)
{
    // Расширяем дефектный прямоугольник для учета возможных пересечений
    cv::Rect2i expandedRect = defect.rect;
    expandedRect.x = (defect.rect.x < rectExtension) ? 0 : defect.rect.x - rectExtension;
    expandedRect.y = (defect.rect.y < rectExtension) ? 0 : defect.rect.y - rectExtension;
    expandedRect.width += 2 * rectExtension;
    expandedRect.height += 2 * rectExtension;

    // Вычисляем пересечение между расширенным дефектом и объединенным дефектом
    cv::Rect2i intersectionRect = expandedRect & mergedDefect.rect;

    // const int rectExtension = 10 включительно
    // Проверка на пересечение или соприкосновение
    if (intersectionRect.area() > 0)
    {
        // Определяем ROI в пределах маски defect.mask
        // х и у имеют значение 0, если дефекты пересекаются слева от defect.mask, иначе intersectionRect.* - defect.rect.*
        int dx = std::max(0, intersectionRect.x - defect.rect.x);
        int dy = std::max(0, intersectionRect.y - defect.rect.y);

        // Ширина и высота рамки должны быть не меньше rectExtension. Если и так не меньше, то берем минимальное
        // из ширины/высоты пересечения и доступной ширины/высоты в маске defect
        int dw = std::max(rectExtension, std::min(intersectionRect.width, defect.mask.cols - dx));
        int dh = std::max(rectExtension, std::min(intersectionRect.height, defect.mask.rows - dy));
        // Сложно, но работает только так...

        // Если пересечение происходит в левой части defect, то все хорошо, но если справа, то смотрим 
        // относительно правого дефекта. Правый - это mergedDefect
        bool withinDefect = (dx + dw <= defect.rect.width) && (dy + dh <= defect.rect.height);
        if (!withinDefect) {
            dx = std::max(0, intersectionRect.x - mergedDefect.rect.x);
            dy = std::max(0, intersectionRect.y - mergedDefect.rect.y);
            int dw = std::max(rectExtension, std::min(intersectionRect.width, mergedDefect.mask.cols - dx));
            int dh = std::max(rectExtension, std::min(intersectionRect.height, mergedDefect.mask.rows - dy));
        }
        cv::Rect2i defectMaskROI_rect(dx, dy, dw, dh);

        // Проверяем, что ROI находится в пределах соответствующей маски
        const cv::Mat& mask = withinDefect ? defect.mask : mergedDefect.mask;
        if (dx < 0 || dy < 0 || dx + dw > mask.cols || dy + dh > mask.rows) {
            std::cout << "Intersection is out of mask bounds or dimensions are invalid" << std::endl;
            return false;
        }

        cv::Mat defectMaskROI = mask(defectMaskROI_rect);
        return cv::countNonZero(defectMaskROI) > 0;
    }

    return false;
}


// Функция для обработки дефектов с проверкой наличия дефектов в зоне пересечения
void hangingStringDefect(const DetectResult& defect, std::vector<DetectResult>& hangingStringMergedDefects)
{
    bool merged = false;
    for (auto& mergedDefect : hangingStringMergedDefects)
    {
        bool hasRealDefects = checkForRealDefectsInIntersection(defect, mergedDefect);

        // Проверяем, пересекается ли расширенная рамка текущего дефекта с рамкой объединенного дефектаa
        if (hasRealDefects)
        {
            cv::Rect2i newRect = mergedDefect.rect | defect.rect;
            cv::Mat newMask = mergeMasks(mergedDefect.mask, defect.mask, mergedDefect.rect, defect.rect);

            mergedDefect.rect = newRect;
            mergedDefect.mask = newMask;
            mergedDefect.prob = std::max(mergedDefect.prob, defect.prob);
            merged = true;
            break;
        }
    }
    // Если дефект не был объединен ни с одним из существующих, добавляем его в список
    if (!merged) {
        hangingStringMergedDefects.emplace_back(defect);
    }
}

void mergeDefectsMy(std::vector<std::vector<BatchResult>> batchesDetects, std::vector<DetectResult>& resultDetects)
{
    // map для хранения промежуточных результатов вертикального объединения по каждому типу дефектов
    std::unordered_map<DefectType, std::vector<DetectResult>> verticalMerged;
    std::unordered_map<DefectType, std::vector<DetectResult>> horizontalMerged;
    std::unordered_map<DefectType, std::vector<DetectResult>> otherMerged;
    std::unordered_map<DefectType, std::vector<DetectResult>> verticalMergedHorizontally;
    // по строкам
    for (auto& batchesRow : batchesDetects) {

        bool vertical = 0;
        bool horizontal = 0;

        // по батчам
        for (auto& batch : batchesRow) {

            // по дефектам
            for (auto& defect : batch.detects) {
                DefectType defectType = classifyDefect(defect.klass);
                // Выбираем функцию обработки в зависимости от типа дефекта
                switch (defectType) {
                case DefectType::Seam:
                case DefectType::ThreadSpan:
                case DefectType::DoubleThreadX:
                case DefectType::LightStrip:
                case DefectType::ThreadThickeningX:
                case DefectType::DifferentThreadX:
                case DefectType::IncompleteDoubleThread:
                case DefectType::SparseThread:
                    horizontalDefect(defect, horizontalMerged[defectType]);
                    horizontal = 1;
                    break;
                case DefectType::Thickening:
                case DefectType::HangingString:
                case DefectType::StuffedFluff:
                case DefectType::Contamination:
                case DefectType::Knot:
                case DefectType::Spot:
                    hangingStringDefect(defect, otherMerged[defectType]);
                    break;
                case DefectType::Dissection:
                case DefectType::Blisna:
                case DefectType::DoubleThreadY:
                case DefectType::ThreadThickeningY:
                case DefectType::Fold:
                case DefectType::Crease:
                case DefectType::WaterLeak:
                case DefectType::ViolationOfWeaving:
                    verticalDefectHorizontally(defect, verticalMergedHorizontally[defectType]);
                    vertical = 1;
                    break;
                default:
                    resultDetects.push_back(defect); 
                    break;
                }
            }
        }

        // Вертикальное объединение результатов этой строки для горизонтальных дефектов
        for (auto& [defectType, defects] : horizontalMerged) {
            if (horizontal) {
                for (const auto& defect : defects) {
                    verticalDefect(defect, verticalMerged[defectType]);
                }
            }
        }

        // Вертикальное объединение результатов этой строки для толстых вертикальных дефектов
        for (auto& [defectType, defects] : verticalMergedHorizontally) {
            if (vertical){
                for (const auto& defect : defects) {
                    verticalDefect(defect, verticalMerged[defectType]);
                }
            }
        }
    }

    // Перемещаем окончательные объединенные дефекты в resultDetects
    for (auto& [defectType, defects] : verticalMerged) {
        for (auto& defect : defects) {
            resultDetects.emplace_back(std::move(defect));
        }
    }

    // Перемещаем окончательные объединенные дефекты в resultDetects
    for (auto& [defectType, defects] : otherMerged) {
        for (auto& defect : defects) {
            resultDetects.emplace_back(std::move(defect));
        }
    }
}



// оставлено для просмотра созданных для тестирования дефектов по отдельности
void mergeDefects(std::vector<std::vector<BatchResult>> batchesDetects,
    std::vector<DetectResult>& resultDetects)
{
    // Базовая реализация, которая просто переносит дефекты из одного списка в другой без объединения кусков дефектов 

    for (auto& batchesRow : batchesDetects)
    {
        for (auto& batch : batchesRow)
        {
            for (auto& defect : batch.detects)
            {
                resultDetects.emplace_back(std::move(defect));
            }
        }
    }
}
