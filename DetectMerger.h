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
        if (defectClass == "B.7") {
            return DefectType::Seam;
        }
    }
    return DefectType::Default;
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


void verticalDefect(const DetectResult& defect, std::vector<DetectResult>& verticalMergedDefectsOneType)
{
    bool mergedVer = false;
    for (auto& mergedDefect : verticalMergedDefectsOneType)
    {
        // Расширяем рамку одного из дефектов для проверки окрестности
        cv::Rect2i expandedRect = defect.rect;
        expandedRect.x -= 10;
        expandedRect.y -= 10;
        expandedRect.width += 20;
        expandedRect.height += 20;

        // разница между дефектами одного класса может быть до 10 пикселей 
        //if (std::abs(mergedDefect.rect.x - defect.rect.x) <= 10 &&
        //    std::abs(mergedDefect.rect.y + mergedDefect.rect.height - defect.rect.y) <= 10)
        // Проверяем пересечение расширенных рамок
        if ((expandedRect & mergedDefect.rect).area() > 0)
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
    //если вертикально не объединяется, то записываем в результат
    if (!mergedVer) {
        verticalMergedDefectsOneType.emplace_back(std::move(defect));
    }
}


void mergeDefectsMy(std::vector<std::vector<BatchResult>> batchesDetects, std::vector<DetectResult>& resultDetects)
{
    // map для хранения промежуточных результатов вертикального объединения по каждому типу дефектов
    std::unordered_map<DefectType, std::vector<DetectResult>> verticalMerged;
    std::unordered_map<DefectType, std::vector<DetectResult>> horizontalMerged;

    // по строкам
    for (auto& batchesRow : batchesDetects) {

        // по батчам
        for (auto& batch : batchesRow) {

            // по дефектам
            for (auto& defect : batch.detects) {
                DefectType defectType = classifyDefect(defect.klass);
                switch (defectType) {
                case DefectType::Seam:
                    horizontalDefect(defect, horizontalMerged[defectType]);
                    break;

                /*
                * Для какого-нибудь вертикального дефекта
                case DefectType::VerticalFirst:
                    verticalDefect(defect, verticalMerged[defectType]);
                    break;
                */
                default:
                    resultDetects.push_back(defect);
                    break;
                }
            }
        }

        // Вертикальное объединение результатов этой строки для дефектов типа Seam
        for (auto& [defectType, defects] : horizontalMerged) {
            if (defectType == DefectType::Seam) {
                for (const auto& defect : defects) {
                    verticalDefect(defect, verticalMerged[defectType]);
                }
            }
        }
    }

    /*
    * Также для вертикального
    // Объединение результатов для дефектов типа VerticalFirst по горизонтали
    for (auto& [defectType, defects] : verticalMerged) {
        if (defectType == DefectType::VerticalFirst) {
            std::vector<DetectResult> finalMerged;
            for (const auto& defect : defects) {
                horizontalDefect(defect, finalMerged);
            }
            verticalMerged[defectType] = std::move(finalMerged);
        }
    }
    */

    // Перемещаем окончательные объединенные дефекты в resultDetects
    for (auto& [defectType, defects] : verticalMerged) {
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
