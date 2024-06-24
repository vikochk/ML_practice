#pragma once 
#include <vector> 

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


cv::Rect2i mergeRects(const cv::Rect2i& r1, const cv::Rect2i& r2)
{
    int x = std::min(r1.x, r2.x);
    int y = std::min(r1.y, r2.y);
    int width = std::max(r1.x + r1.width, r2.x + r2.width) - x;
    int height = std::max(r1.y + r1.height, r2.y + r2.height) - y;
    return cv::Rect2i(x, y, width, height);
}


cv::Mat mergeMasks(const cv::Mat& m1, const cv::Mat& m2, const cv::Rect2i& r1, const cv::Rect2i& r2)
{
    cv::Rect2i mergedRect = mergeRects(r1, r2);
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
        // Проверим, имеют ли два дефекта пересечения по оси Y
        if ((defect.rect.y < mergedSeam.rect.y + mergedSeam.rect.height) &&
            (mergedSeam.rect.y < defect.rect.y + defect.rect.height))
        {
            // Объединяем текущий дефект с уже найденным
            cv::Rect2i newRect = mergeRects(mergedSeam.rect, defect.rect);
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
        // возможно стоит доработать
        // левая нижняя координата X предыдущего объединенного дефекта 
        // совпадает с левой верхней координатой X текущего дефекта
        if (resultDetect.rect.x == defect.rect.x &&
            resultDetect.rect.y + resultDetect.rect.height == defect.rect.y)
        {
            // Объединяем текущий дефект с уже найденным
            cv::Rect2i newRect = mergeRects(resultDetect.rect, defect.rect);
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

void mergeDefectsMy(std::vector<std::vector<BatchResult>> batchesDetects,
    std::vector<DetectResult>& resultDetects)
{
    // для каждой строки 
    for (auto& batchesRow : batchesDetects)
    {
        std::vector<DetectResult> mergedSeams; // Вектор для хранения объединенных швов 

        // для каждого батча в этой строке 
        for (auto& batch : batchesRow)
        {
            // для каждого дефекта в этом батче 
            for (auto& defect : batch.detects)
            {
                if (defect.klass == 1) // Если класс дефекта "шов" 
                {
                    horizontalDefect(std::move(defect), mergedSeams);
                }
                else
                {
                    resultDetects.emplace_back(std::move(defect));
                }
            }
        }
        
        //для всех получившихся горизонатльных швов этой строки
        for (auto& seam : mergedSeams)
        {
            verticalDefect(std::move(seam), resultDetects); // внутри уже будет происходить запись mergedSeams в resultDetects
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
