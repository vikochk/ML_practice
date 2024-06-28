#pragma once 
#include <vector> 
#include <unordered_map>

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


//cv::Rect2i mergeRects(const cv::Rect2i& r1, const cv::Rect2i& r2)
//{
//    int x = std::min(r1.x, r2.x);
//    int y = std::min(r1.y, r2.y);
//    int width = std::max(r1.x + r1.width, r2.x + r2.width) - x;
//    int height = std::max(r1.y + r1.height, r2.y + r2.height) - y;
//    return cv::Rect2i(x, y, width, height);
//}

// Сопоставление ID класса дефекта с его строковым представлением
std::unordered_map<int, std::string> defectClassMapping = {
    {0, "C.2.3"}, {1, "C.2.1"}, {2, "T.1.4"}, {3, "C.2.4"},
    {4, "B.4"}, {5, "O.1.1"}, {6, "C.2.2"}, {7, "T.1.2"},
    {8, "T.2.2.2"}, {9, "C.3.1"}, {10, "T.3.1"}, {11, "B.7"},
    {12, "B.3"}, {13, "T.2.6"}, {14, "C.3.2"}, {15, "O.1.2"},
    {16, "O.2.3"}, {17, "T.1.3"}, {18, "T.1.1"}, {19, "T.2.1"},
    {20, "T.2.2.1"}, {21, "C.1.2"}, {22, "T.3.2"}, {23, "B.1"}
};

// Функция для преобразования строки класса дефекта в DefectType
DefectType classifyDefect(const std::string& defectClass) {
    for (const auto& pair : defectClassMapping) {
        if (pair.second == defectClass) {
            if (defectClass == "B.7") {
                return DefectType::Seam;
            }
            else {
                return DefectType::Default;
            }
        }
    }
    return DefectType::Default; // Возвращаем Default, если класс не найден
}

cv::Mat mergeMasks(const cv::Mat& m1, const cv::Mat& m2, const cv::Rect2i& r1, const cv::Rect2i& r2)
{
    cv::Rect2i mergedRect = r1 | r2;
    cv::Mat mergedMask = cv::Mat::zeros(mergedRect.size(), CV_8UC1);

    cv::Rect2i r1_in_merged = cv::Rect2i(r1.x - mergedRect.x, r1.y - mergedRect.y, r1.width, r1.height);
    cv::Rect2i r2_in_merged = cv::Rect2i(r2.x - mergedRect.x, r2.y - mergedRect.y, r2.width, r2.height);

    m1.copyTo(mergedMask(r1_in_merged));
    m2.copyTo(mergedMask(r2_in_merged));

    return mergedMask;
}

void horizontalDefect(const DetectResult& defect, std::vector<DetectResult>& mergedSeams)
{
    bool mergedHor = false;
    for (auto& mergedSeam : mergedSeams)
    {
        // Проверяем, пересекаются ли дефекты по вертикали
        if ((defect.rect.y < mergedSeam.rect.y + mergedSeam.rect.height) &&
            (mergedSeam.rect.y < defect.rect.y + defect.rect.height))
        {
            // Объединяем дефекты
            cv::Rect2i newRect = mergedSeam.rect | defect.rect;
            cv::Mat newMask = mergeMasks(mergedSeam.mask, defect.mask, mergedSeam.rect, defect.rect);

            mergedSeam.rect = newRect;
            mergedSeam.mask = newMask;
            mergedSeam.prob = std::max(mergedSeam.prob, defect.prob); // берем максимальную вероятность
            mergedHor = true;
            break;
        }
    }
    if (!mergedHor) {
        mergedSeams.push_back(defect);
    }
}


void verticalDefect(const DetectResult& defect, std::vector<DetectResult>& resultDetects)
{
    bool mergedVer = false;
    for (auto& resultDetect : resultDetects)
    {
        // разница между дефектами одного класса может быть до 10 пикселей 
        if ((defect.klass == resultDetect.klass) && std::abs(resultDetect.rect.x - defect.rect.x) <= 10 &&
            std::abs(resultDetect.rect.y + resultDetect.rect.height - defect.rect.y) <= 10)
        {
            // Объединяем текущий дефект с уже найденным
            cv::Rect2i newRect = resultDetect.rect | defect.rect;
            cv::Mat newMask = mergeMasks(resultDetect.mask, defect.mask, resultDetect.rect, defect.rect);

            resultDetect.rect = newRect;
            resultDetect.mask = newMask;
            resultDetect.prob = std::max(resultDetect.prob, defect.prob); // берем максимальную вероятность
            mergedVer = true;
            break;
        }
    }
    //если вертикально не объединяется, то записываем в результат
    if (!mergedVer) {
        resultDetects.emplace_back(std::move(defect));
    }
}


void mergeDefectsMy(std::vector<std::vector<BatchResult>> batchesDetects, std::vector<DetectResult>& resultDetects)
{
    // Горизонтальное объединение в каждой строке
    for (auto& batchesRow : batchesDetects) {
        std::unordered_map<DefectType, std::vector<DetectResult>> mergedDefectsByType;

        for (auto& batch : batchesRow) {
            for (auto& defect : batch.detects) {
                horizontalDefect(defect, mergedDefectsByType[static_cast<DefectType>(defect.klass)]);
            }
        }

        // Перемещаем объединенные дефекты в resultDetects
        for (auto it = mergedDefectsByType.begin(); it != mergedDefectsByType.end(); ++it) {
            DefectType defectType = it->first;
            std::vector<DetectResult>& defects = it->second;
            for (auto& defect : defects) {
                resultDetects.emplace_back(std::move(defect));
            }
        }
    }

    // Вертикальное объединение по всем строкам
    std::vector<DetectResult> mergedVerticalDefects;
    for (auto& defect : resultDetects) {
        verticalDefect(defect, mergedVerticalDefects);
    }

    resultDetects = std::move(mergedVerticalDefects);
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
