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

void addHangingSting(std::vector<std::vector<BatchResult>>& inputBatchesDefects, int i, int j,
    int x, int y, int width, int height, const std::string& defectClass, cv::Mat stringMask)
{
    DetectResult defect;
    defect.rect = cv::Rect2i(x, y, width, height);
    defect.prob = 0.95f; // пока фиксированная вероятность 
    defect.mask = stringMask; // передаем готовую маску

    // Преобразуем defectClass в int (klass)
    int klass = -1; // По умолчанию -1 или другое значение, если не найдено
    for (const auto& pair : defectClassMapping) {
        if (pair.second == defectClass) {
            klass = pair.first;
            break;
        }
    }
    defect.klass = klass;

    inputBatchesDefects[i][j].detects.push_back(defect);
}

cv::Mat displayDefects(const std::vector<DetectResult>& resultDefects)
{
    cv::Mat canvas = cv::Mat::zeros(2500, 2000, CV_8UC1);

    // Проходим по всем дефектам
    for (const auto& defect : resultDefects) {
        // Определяем ROI для текущего дефекта
        cv::Rect roi(defect.rect.x, defect.rect.y, defect.rect.width, defect.rect.height);
        cv::Rect maskRoi(cv::Point(0, 0), defect.mask.size());
        cv::Mat maskROI = defect.mask(maskRoi);

        // Наложение маски дефекта на полотно
        maskROI.copyTo(canvas(roi), maskROI);
    }
    cv::Mat resizedCanvas;
    cv::resize(canvas, resizedCanvas, cv::Size(canvas.cols / 3, canvas.rows / 3));

    return resizedCanvas;
}

int main()
{
    std::vector<std::vector<BatchResult>> inputBatchesDefects(5, std::vector<BatchResult>(4));

    // Заполняем входные данные
    {
        cv::Size2i imageSize(2000, 2500);
        cv::Size2i batchSize(500, 500);

        int cols = imageSize.width / batchSize.width + (imageSize.width % batchSize.width != 0);         // = 4
        int rows = imageSize.height / batchSize.height + (imageSize.height % batchSize.height != 0);     // = 5

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

                //// Пример для соприкасающихся ровно швов в первой строке
                //if (i == 0 && j < 2) {
                //    addDefect(inputBatchesDefects, i, j, j * 500, 100, 500, 50, "B.7");
                //}
                //else if (i == 0 && j >= 2) {
                //    addDefect(inputBatchesDefects, i, j, j * 500, 100, 500, 40, "B.7");
                //}

                //// Пример для соприкасающихся не ровно швов в первой строке
                //if (i == 0 && j < 2) {
                //    addDefect(inputBatchesDefects, i, j, j * 500, 300, 500, 50, "B.7");
                //}
                //else if (i == 0 && j >= 2) {
                //    addDefect(inputBatchesDefects, i, j, j * 500, 320, 500, 50, "B.7");
                //}

                //// Пример для шва на расстоянии 10 пикс от другого во второй строке
                //if (i == 1 && j < 3) {
                //    addDefect(inputBatchesDefects, i, j, j * 500, 550, 500, 40, "B.7");
                //}
                //else if (i == 1 && j == 3) {
                //    addDefect(inputBatchesDefects, i, j, j * 500 + 10, 560, 490, 40, "B.7");
                //}

                //// Пример для шва, найденного частично, во второй строке
                //if (i == 1 && j == 0) {
                //    addDefect(inputBatchesDefects, i, j, j * 500, 800, 500, 50, "B.7");
                //}
                //else if (i == 1 && j == 3) {
                //    addDefect(inputBatchesDefects, i, j, j * 500 + 200, 800, 300, 100, "B.7");
                //}

                //// Пример для шва, пересекающего четыре батча (250, 450 длина 500 ширина 100)
                ////А
                //if (i == 1 && j == 0) {
                //    addDefect(inputBatchesDefects, i, j, 250, 950, 250, 50, "B.7");
                //}
                ////C
                //if (i == 1 && j == 1) {
                //    addDefect(inputBatchesDefects, i, j, 500, 950, 250, 50, "B.7");
                //}
                ////B
                //if (i == 2 && j == 0) {
                //    addDefect(inputBatchesDefects, i, j, 260, 1010, 240, 50, "B.7");
                //}
                ////D
                //if (i == 2 && j == 1) {
                //    addDefect(inputBatchesDefects, i, j, 500, 1000, 250, 50, "B.7");
                //}

                //// Пример для шва, пересекающего четыре батча (1250, 450 длина 500 ширина 100)
                //if (i == 1 && j == 2) {
                //    addDefect(inputBatchesDefects, i, j, 1250, 950, 250, 50, "B.7");
                //}
                //if (i == 1 && j == 3) {
                //    addDefect(inputBatchesDefects, i, j, 1500, 950, 250, 50, "B.7");
                //}
                //if (i == 2 && j == 2) {
                //    addDefect(inputBatchesDefects, i, j, 1250, 1000, 250, 50, "B.7");
                //}
                //if (i == 2 && j == 3) {
                //    addDefect(inputBatchesDefects, i, j, 1500, 1000, 250, 50, "B.7");
                //}

                //// Пример пятен
                //if (i == 0 && j == 3) {
                //    addDefect(inputBatchesDefects, i, j, 1300, 200, 30, 30, "B.4");
                //}
                //if (i == 2 && j == 3) {
                //    addDefect(inputBatchesDefects, i, j, 1280, 1200, 30, 30, "B.4");
                //}
                //if (i == 2 && j == 0) {
                //    addDefect(inputBatchesDefects, i, j, 70, 1160, 30, 35, "B.4");
                //}
                //if (i == 2 && j == 0) {
                //    addDefect(inputBatchesDefects, i, j, 100, 1200, 30, 30, "B.4");
                //}

                // Висячая нить
                if (i == 3 && j == 0) {
                    cv::Mat currentMask1 = cv::Mat::zeros(cv::Size(55, 70), CV_8UC1);
                    cv::line(currentMask1, cv::Point(0, 70), cv::Point(55, 0), cv::Scalar(255), 3);
                    addHangingSting(inputBatchesDefects, i, j, 200, 1500, 55, 70, "O.2.3", currentMask1);

                    cv::Mat currentMask2 = cv::Mat::zeros(cv::Size(25, 60), CV_8UC1);
                    cv::line(currentMask2, cv::Point(0, 0), cv::Point(25, 60), cv::Scalar(255), 3);
                    addHangingSting(inputBatchesDefects, i, j, 470, 1500, 25, 60, "O.2.3", currentMask2);
                }
                if (i == 2 && j == 0) {
                    cv::Mat currentMask = cv::Mat::zeros(cv::Size(215, 70), CV_8UC1);
                    cv::line(currentMask, cv::Point(0, 70), cv::Point(115, 0), cv::Scalar(255), 3);
                    cv::line(currentMask, cv::Point(115, 0), cv::Point(215, 70), cv::Scalar(255), 3);
                    addHangingSting(inputBatchesDefects, i, j, 255, 1430, 215, 70, "O.2.3", currentMask);
                }
                if (i == 3 && j == 1) {
                    cv::Mat currentMask = cv::Mat::zeros(cv::Size(35, 80), CV_8UC1);
                    cv::line(currentMask, cv::Point(0, 0), cv::Point(35, 80), cv::Scalar(255), 3);
                    addHangingSting(inputBatchesDefects, i, j, 505, 1580, 35, 80, "O.2.3", currentMask);
                }

                // Батч (0, 0) - линия от (410, 410) до (490, 490)
                if (i == 0 && j == 0) {
                    cv::Mat mask = cv::Mat::zeros(cv::Size(90, 90), CV_8UC1);
                    cv::line(mask, cv::Point(0, 0), cv::Point(80, 80), cv::Scalar(255), 5);  // Линия от (0, 0) до (80, 80)
                    addHangingSting(inputBatchesDefects, i, j, 410, 410, 90, 90, "O.2.3", mask);
                }

                // Батч (1, 1) - линия от (500, 500) до (1000, 1000)
                if (i == 1 && j == 1) {
                    cv::Mat mask = cv::Mat::zeros(cv::Size(500, 500), CV_8UC1);
                    cv::line(mask, cv::Point(0, 0), cv::Point(500, 500), cv::Scalar(255), 5);  // Линия от (0, 0) до (500, 500)
                    addHangingSting(inputBatchesDefects, i, j, 500, 500, 500, 500, "O.2.3", mask);
                }

                // Батч (2, 2) - линия от (1500, 1000) до (1000, 1500)
                if (i == 2 && j == 2) {
                    cv::Mat mask = cv::Mat::zeros(cv::Size(500, 500), CV_8UC1);
                    cv::line(mask, cv::Point(500, 0), cv::Point(0, 500), cv::Scalar(255), 5);  // Линия от (0, 0) до (500, 500)
                    addHangingSting(inputBatchesDefects, i, j, 1000, 1000, 500, 500, "O.2.3", mask);
                }

                // Нитки "галочкой"
                if (i == 4 && j == 0) {
                    cv::Mat mask1 = cv::Mat::zeros(cv::Size(100, 100), CV_8UC1);
                    cv::line(mask1, cv::Point(100, 0), cv::Point(0, 100), cv::Scalar(255), 5); 
                    addHangingSting(inputBatchesDefects, i, j, 0, 2000, 100, 100, "O.2.3", mask1);

                    cv::Mat mask2 = cv::Mat::zeros(cv::Size(100, 100), CV_8UC1);
                    cv::line(mask2, cv::Point(0, 0), cv::Point(100, 100), cv::Scalar(255), 5); 
                    addHangingSting(inputBatchesDefects, i, j, 105, 2000, 100, 100, "O.2.3", mask2);
                }

                // Нитки, пересекающиеся ровно на левом краю полотна
                if (i == 0 && j == 0) {
                    cv::Mat mask1 = cv::Mat::zeros(cv::Size(100, 100), CV_8UC1);
                    cv::line(mask1, cv::Point(0, 0), cv::Point(100, 100), cv::Scalar(255), 5);
                    addHangingSting(inputBatchesDefects, i, j, 0, 108, 100, 100, "O.2.3", mask1);

                    cv::Mat mask2 = cv::Mat::zeros(cv::Size(100, 100), CV_8UC1);
                    cv::line(mask2, cv::Point(100, 0), cv::Point(0, 100), cv::Scalar(255), 5);
                    addHangingSting(inputBatchesDefects, i, j, 0, 0, 100, 100, "O.2.3", mask2);
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

        // Отображение всех дефектов на одном изображении
        cv::Mat combinedImage = displayDefects(resultDefects);
        // Выводим результаты
        cv::imshow("Combined Defects", combinedImage);
        cv::waitKey(0);
        cv::destroyAllWindows();

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
