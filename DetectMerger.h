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

void mergeDefectsMy(std::vector<std::vector<BatchResult>> batchesDetects,
    std::vector<DetectResult>& resultDetects)
{
    //для каждой строки
    for (auto& batchesRow : batchesDetects)
    {
        // создаем переменные для хранения объединенных швов по строке
        std::vector<DetectResult> mergedSeams;

        //для каждого батча в этой строке
        for (auto& batch : batchesRow)
        {
            //для каждого дефекта в этом батче
            for (auto& defect : batch.detects)
            {
                if (defect.klass == 1) // Если класс дефекта "шов"
                {
                    bool merged = false;
                    // Проверяем, можем ли объединить с уже найденными швами в строке
                    for (auto& mergedSeam : mergedSeams)
                    {
                        // Проверяем, соприкасаются ли прямоугольники швов
                        if (defect.rect.y == mergedSeam.rect.y && // одинаковые строки
                            (defect.rect.x == mergedSeam.rect.x + mergedSeam.rect.width || //прямоугольники соприкасаются
                                mergedSeam.rect.x == defect.rect.x + defect.rect.width))
                        {
                            // Объединяем прямоугольники швов
                            mergedSeam.rect.width += defect.rect.width;
                            mergedSeam.rect.height = std::max(mergedSeam.rect.height, defect.rect.height); // Максимальная высота

                            // Объединяем маски
                            cv::Mat extendedMask = cv::Mat::zeros(mergedSeam.rect.size(), CV_8UC1);
                            mergedSeam.mask.copyTo(extendedMask(cv::Rect(0, 0, mergedSeam.mask.cols, mergedSeam.mask.rows)));
                            defect.mask.copyTo(extendedMask(cv::Rect(mergedSeam.mask.cols, 0, defect.mask.cols, defect.mask.rows)));
                            mergedSeam.mask = extendedMask;

                            merged = true;
                            break;
                        }
                    }

                    if (!merged)
                    {
                        // Если не удалось объединить с существующими швами, добавляем как новый шов
                        mergedSeams.push_back(defect);
                    }
                }
                else
                {
                    resultDetects.emplace_back(std::move(defect)); // Добавляем обычные дефекты
                }
            }
        }

        // Добавляем все объединенные швы в результирующий список
        for (auto& mergedSeam : mergedSeams)
        {
            resultDetects.emplace_back(std::move(mergedSeam));
        }
    }
}


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
