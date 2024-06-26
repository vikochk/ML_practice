#include <cmath>
#include <iostream>
#include "DetectMerger.h"
#include <unordered_map>
#include <cmath>

void addDefect(std::vector<std::vector<BatchResult>>& inputBatchesDefects, int i, int j, 
               int x, int y, int width, int height, const std::string& defectClass)
{
    DetectResult defect;
    defect.rect = cv::Rect2i(x, y, width, height);
    defect.prob = 0.95f; // пока фиксированная вероятность 
    defect.mask = cv::Mat::ones(height, width, CV_8UC1) * 255;

    // Преобразуем defectClass в int (klass)
    int klass = -1; // По умолчанию -1 или другое значение, если не найдено
    for (const auto& pair : defectClassMapping) {
        if (pair.second == defectClass) {
            klass = pair.first;
            break;
        }
    }

    // Проверяем, удалось ли найти класс в mapping
    if (klass == -1) {
        // Логика для обработки, если класс не найден
        // Например, генерация ошибки или другие действия
        return;
    }

    defect.klass = klass;

    inputBatchesDefects[i][j].detects.push_back(defect);
}


int main()
{
    std::vector<std::vector<BatchResult>> inputBatchesDefects(2, std::vector<BatchResult>(4));

    // Заполняем входные данные
    {
        cv::Size2i imageSize(2000, 2000);
        cv::Size2i batchSize(500, 500);

        int cols = imageSize.width / batchSize.width + (imageSize.width % batchSize.width != 0);         // = 4
        int rows = imageSize.height / batchSize.height + (imageSize.height % batchSize.height != 0);     // = 4

        // формируем массив батчей
        inputBatchesDefects.resize(rows);
        for (auto& batchesRow : inputBatchesDefects)
        {
            batchesRow.resize(cols);
        }

        for (int i = 0; i < inputBatchesDefects.size(); ++i)
        {
            for (int j = 0; j < inputBatchesDefects[i].size(); ++j)
            {
                auto& batch = inputBatchesDefects[i][j];
                batch.batchRect.x = j * batchSize.width;
                batch.batchRect.y = i * batchSize.height;
                batch.batchRect.width = std::min(batchSize.width, imageSize.width - batch.batchRect.x);
                batch.batchRect.height = std::min(batchSize.height, imageSize.height - batch.batchRect.y);

                // Пример для соприкасающихся ровно швов в первой строке
                if (i == 0 && j < 2) {
                    addDefect(inputBatchesDefects, i, j, j * 500, 100, 500, 50,  "B.7");
                }
                else if (i == 0 && j >= 2) {
                    addDefect(inputBatchesDefects, i, j, j * 500, 100, 500, 40,  "B.7");
                }

                // Пример для соприкасающихся не ровно швов в первой строке
                if (i == 0 && j < 2) {
                    addDefect(inputBatchesDefects, i, j, j * 500, 300, 500, 50, "B.7");
                }
                else if (i == 0 && j >= 2) {
                    addDefect(inputBatchesDefects, i, j, j * 500, 320, 500, 50, "B.7");
                }

                // Пример для шва на расстоянии 10 пикс от другого во второй строке
                if (i == 1 && j < 3) {
                    addDefect(inputBatchesDefects, i, j, j * 500, 550, 500, 40, "B.7");
                }
                else if (i == 1 && j == 3) {
                    addDefect(inputBatchesDefects, i, j, j * 500 + 10, 560, 490, 40, "B.7");
                }

                // Пример для шва, найденного частично, во второй строке
                if (i == 1 && j == 0) {
                    addDefect(inputBatchesDefects, i, j, j * 500, 800, 500, 50, "B.7");
                }
                else if (i == 1 && j == 3) {
                    addDefect(inputBatchesDefects, i, j, j * 500 + 200, 800, 300, 100, "B.7");
                }

                // Пример для шва, пересекающего четыре батча (250, 450 длина 500 ширина 100)
                //А
                if (i == 1 && j == 0) {
                   addDefect(inputBatchesDefects, i, j, 250, 950, 250, 50, "B.7");
                }
                //C
                if (i == 1 && j == 1) {
                   addDefect(inputBatchesDefects, i, j, 500, 950, 250, 50, "B.7");
                }
                //B
                if (i == 2 && j == 0) {
                    addDefect(inputBatchesDefects, i, j, 260, 1010, 240, 50, "B.7");
                }
                //D
                if (i == 2 && j == 1) {
                   addDefect(inputBatchesDefects, i, j, 500, 1000, 250, 50, "B.7");
                }

                // Пример для шва, пересекающего четыре батча (1250, 450 длина 500 ширина 100)
                if (i == 1 && j == 2) {
                    addDefect(inputBatchesDefects, i, j, 1250, 950, 250, 50, "B.7");
                }
                if (i == 1 && j == 3) {
                    addDefect(inputBatchesDefects, i, j, 1500, 950, 250, 50, "B.7");
                }
                if (i == 2 && j == 2) {
                    addDefect(inputBatchesDefects, i, j, 1250, 1000, 250, 50, "B.7");
                }
                if (i == 2 && j == 3) {
                    addDefect(inputBatchesDefects, i, j, 1500, 1000, 250, 50, "B.7");
                }

                // Пример пятен
                if (i == 0 && j == 3) {
                    addDefect(inputBatchesDefects, i, j, 1300, 200, 30, 30, "B.4");
                }
                if (i == 2 && j == 3) {
                    addDefect(inputBatchesDefects, i, j, 1280, 1200, 30, 30, "B.4");
                }
                if (i == 2 && j == 0) {
                    addDefect(inputBatchesDefects, i, j, 70, 1160, 30, 35, "B.4");
                }
                if (i == 2 && j == 0) {
                    addDefect(inputBatchesDefects, i, j, 100, 1200, 30, 30, "B.4");
                }

            }
        }
    }


    // Использование функции для объединения дефектов
    {
        std::vector<DetectResult> resultDefects;

        //написанная функция
        mergeDefectsMy(std::move(inputBatchesDefects), resultDefects);
        //базовая функция для тестирования
        //mergeDefects(std::move(inputBatchesDefects), resultDefects);  // используем move, чтобы не копировать входные дефекты в функцию, а переместить их

        // Выводим результаты
        for (const auto& defect : resultDefects)
        {
            std::cout << "Defect class: " << defect.klass
                << ", Rect: (" << defect.rect.x << ", " << defect.rect.y << ", "
                << defect.rect.width << ", " << defect.rect.height << ")"
                << ", Probability: " << defect.prob << std::endl;
        }
    }
    return 0;
}
