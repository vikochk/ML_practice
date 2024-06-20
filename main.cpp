#include <cmath>

#include "DetectMerger.h"

int main()
{
    std::vector<std::vector<BatchResult>> inputBatchesDefects;

    // Заполняем входные данные
    {
        cv::Size2i imageSize(1200, 1000);
        cv::Size2i batchSize(500, 500);

        int cols = imageSize.width / batchSize.width + (imageSize.width % batchSize.width != 0);         // = 3
        int rows = imageSize.height / batchSize.height + (imageSize.height % batchSize.height != 0);     // = 2

        // формируем массив батчей
        inputBatchesDefects.resize(rows);
        for(auto& batchesRow: inputBatchesDefects)
        {
            batchesRow.resize(cols);
        }

        for(int i = 0; i < inputBatchesDefects.size(); ++i)
        {
            for(int j = 0; j < inputBatchesDefects[i].size(); ++j)
            {
                auto& batch = inputBatchesDefects[i][j];
                batch.batchRect.x = j * batchSize.width;
                batch.batchRect.y = i * batchSize.height;
                batch.batchRect.width = std::min(batchSize.width, imageSize.width - batch.batchRect.x);
                batch.batchRect.height = std::min(batchSize.height, imageSize.height - batch.batchRect.y);

                // Добавляем дефекты для этого батча
                // batch.detects = ...
            }
        }
    }

    // Использование функции для объединения дефектов
    {
        std::vector<DetectResult> resultDefects;
        mergeDefects(std::move(inputBatchesDefects), resultDefects);  // используем move, чтобы не копировать входные дефекты в функцию, а переместить их
    }

}
