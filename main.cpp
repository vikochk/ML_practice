#include <cmath>
#include <iostream>
#include "DetectMerger.h"

int main()
{
    std::vector<std::vector<BatchResult>> inputBatchesDefects;

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

                // Добавляем дефекты для этого батча

                // Пример для соприкасающихся ровно швов в первой строке 
                if (i == 0 && j < 2)
                {
                    DetectResult seamDefect;
                    seamDefect.rect = cv::Rect2i(j * 500, 100, 500, 50); // Прямоугольник шва 
                    seamDefect.prob = 0.95f;
                    seamDefect.mask = cv::Mat::ones(50, 500, CV_8UC1) * 255; // Бинарная маска шва 
                    seamDefect.klass = 1; // Класс "шов" 
                    batch.detects.push_back(seamDefect);
                }
                else if (i == 0 && j >= 2)
                {
                    DetectResult seamDefect;
                    seamDefect.rect = cv::Rect2i(j * 500, 100, 500, 50); // Прямоугольник шва 
                    seamDefect.prob = 0.95f;
                    seamDefect.mask = cv::Mat::ones(50, 500, CV_8UC1) * 255; // Бинарная маска шва 
                    seamDefect.klass = 1; // Класс "шов" 
                    batch.detects.push_back(seamDefect);
                }

                // Пример для соприкасающихся не ровно швов в первой строке 
                if (i == 0 && j < 2)
                {
                    DetectResult seamDefect;
                    seamDefect.rect = cv::Rect2i(j * 500, 300, 500, 50); // Прямоугольник шва 
                    seamDefect.prob = 0.95f;
                    seamDefect.mask = cv::Mat::ones(50, 500, CV_8UC1) * 255; // Бинарная маска шва 
                    seamDefect.klass = 1; // Класс "шов" 
                    batch.detects.push_back(seamDefect);
                }
                else if (i == 0 && j >= 2)
                {
                    DetectResult seamDefect;
                    seamDefect.rect = cv::Rect2i(j * 500, 320, 500, 50); // Прямоугольник шва 
                    seamDefect.prob = 0.95f;
                    seamDefect.mask = cv::Mat::ones(50, 500, CV_8UC1) * 255; // Бинарная маска шва 
                    seamDefect.klass = 1; // Класс "шов" 
                    batch.detects.push_back(seamDefect);
                }

                // Пример для шва на расстоянии 10 пикс от другого во второй строке 
                if (i == 1 && j < 3)
                {
                    DetectResult seamDefect;
                    seamDefect.rect = cv::Rect2i(j * 500, 550, 500, 40); // Прямоугольник шва 
                    seamDefect.prob = 0.95f;
                    seamDefect.mask = cv::Mat::ones(40, 500, CV_8UC1) * 255; // Бинарная маска шва 
                    seamDefect.klass = 1; // Класс "шов" 
                    batch.detects.push_back(seamDefect);
                }
                else if (i == 1 && j == 3)
                {
                    DetectResult seamDefect;
                    seamDefect.rect =
                        cv::Rect2i(j * 500, 560, 490, 40); // Прямоугольник шва 
                    seamDefect.prob = 0.95f;
                    seamDefect.mask = cv::Mat::ones(40, 490, CV_8UC1) * 255; // Бинарная маска шва 
                    seamDefect.klass = 1; // Класс "шов" 
                    batch.detects.push_back(seamDefect);
                }

                // Пример для шва, найденного частично, во второй строке 
                if (i == 1 && j == 0)
                {
                    DetectResult seamDefect;
                    seamDefect.rect = cv::Rect2i(j * 500, 800, 500, 50); // Прямоугольник шва 
                    seamDefect.prob = 0.95f;
                    seamDefect.mask = cv::Mat::ones(50, 500, CV_8UC1) * 255; // Бинарная маска шва 
                    seamDefect.klass = 1; // Класс "шов" 
                    batch.detects.push_back(seamDefect);
                }
                else if (i == 1 && j == 3)
                {
                    DetectResult seamDefect;
                    seamDefect.rect = cv::Rect2i(j * 500, 800, 200, 100); // Прямоугольник шва 
                    seamDefect.prob = 0.95f;
                    seamDefect.mask = cv::Mat::ones(100, 200, CV_8UC1) * 255; // Бинарная маска шва 
                    seamDefect.klass = 1; // Класс "шов" 
                    batch.detects.push_back(seamDefect);
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
