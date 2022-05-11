#include <opencv2/opencv.hpp>
#include <iostream>

#define window1 "手繪"    // 定義視窗1的名稱
#define window2 "融合"    // 定義視窗2的名稱
#define slider1_name "影像融合"     // 定義滑桿1的名稱
#define slider2_name "大小"   // 定義滑桿2的名稱

cv::Mat im1;    // 影像物件(手繪)
cv::Mat im2;    // 影像物件(融合後影像)
int width = 500;    // im1 的寬度
int height = 500;   // im1 的高度

int buttonDown = 0;     // 記錄滑鼠左鍵是否按下
int slider1 = 50;   // 儲存滑桿1的位置(調整融合加權值)，預設值為50
int slider1_max = 100;  // 滑桿1的最大值為100
int slider2 = 50;   // 儲存滑桿2的位置(調整手繪圖案的大小)，預設值為50
int slider2_max = 100;  // 滑桿2的最大值為100

void onMouse(int event, int x, int y, int flags, void*);    // 宣告副程式，此負責處理滑鼠觸發事件
void on_trackbar(int, void*);   // 宣告副程式，此負責處理滑桿觸發事件

int main(int argc, char** argv)
{
    im1 = cv::Mat(width, height, CV_8UC3, cv::Scalar(0, 0, 0));     // im1 是大小為 500*500 像素的黑色 8UC3 格式影像
    im2 = cv::imread("../data/ntust1.jpg");  // 讀入圖片

    if (im2.empty()) {  // 如果無法讀入圖片，則顯示錯誤訊息
        std::cout << "Could not open or find the image." << std::endl;  // 顯示錯誤訊息
        std::cin.get(); // 等待使用者輸入任何按鍵
        return -1;  // 程式中止
    }

    cv::imshow(window1, im1);   // 在視窗1顯示手繪圖案
    cv::setMouseCallback(window1, onMouse); // 在視窗1處理滑鼠觸發事件

    cv::imshow(window2, im2);   // 在視窗2顯示融合結果
    cv::createTrackbar(slider1_name, window2, &slider1, slider1_max, on_trackbar);  // 在視窗2建立第一條滑桿，預設值是50，最大值是100
    cv::createTrackbar(slider2_name, window2, &slider2, slider2_max, on_trackbar);  // 在視窗2建立第二條滑桿，預設值是50，最大值是100
    on_trackbar(0, 0);  // 呼叫 Trackbar 的 callback function

    cv::waitKey(0); // 等待使用者按下鍵盤任意鍵
    cv::destroyAllWindows();    // 關閉所有視窗
}

void onMouse(int event, int x, int y, int flags, void*)     // 處理滑鼠觸發事件
{
    switch (event) {
    case cv::EVENT_LBUTTONDOWN:     // 當按下滑鼠左鍵
        buttonDown = 1;     // buttonDown 這個 flag 來判斷是否有按下滑鼠左鍵，buttonDown = 1代表按下滑鼠左鍵
        break;

    case cv::EVENT_MOUSEMOVE:   // 當滑鼠移動
        if (buttonDown == 1) {  // 當按著滑鼠左鍵移動時
            cv::circle(im1, cv::Point(x, y), 6, CV_RGB(255, 255, 0), -1);   // 繪製半徑6的黃色實心圓圈
            cv::imshow(window1, im1);   // 顯示手繪影像
        }
        break;

    case cv::EVENT_LBUTTONUP:   // 當放開滑鼠左鍵
        buttonDown = 0;     // buttonDown = 0 代表放開滑鼠左鍵
        on_trackbar(0, 0);  // 當畫完一筆畫時，就更新融合影像
        break;

    case cv::EVENT_RBUTTONUP:   // 當放開滑鼠右鍵
        cv::destroyWindow(window1); // 關閉所有視窗
        break;
    }
}

void on_trackbar(int, void*) {  // 處理滑桿觸發事件
    cv::Mat im1_resized, im1_shrink, mixResult;     // im1_resized: im1做完resize的結果，im1_shrink: im1_resized做完縮放後的結果，mixResult: 影像融合後的結果
    cv::Mat im2_merge = cv::Mat(im2.rows, im2.cols, CV_8UC3, cv::Scalar(0, 0, 0));  // 要與 im2 融合的影像，大小和im2一樣的黑色影像

    if (slider2 == 0) slider2 = 1;  // 如果滑桿2的數值是0，則強制改成1
    double sizeRatio = (double)slider2 / slider2_max;   // 縮放比例為滑桿2數值除以最大值

    cv::resize(im1, im1_resized, cv::Size(im2.cols, im2.rows), cv::INTER_LINEAR);   // 將im1的大小改成im2的大小，結果存在im1_resized
    double size_x = round(im1_resized.cols * sizeRatio);    // 將im1_resized的寬度乘以縮放比例
    double size_y = round(im1_resized.rows * sizeRatio);    // 將im1_resized的高度乘以縮放比例
    cv::resize(im1_resized, im1_shrink, cv::Size(size_x, size_y), cv::INTER_LINEAR);    // 將im1_resized做縮放，結果存在im1_shrink

    cv::Point vertice;  // roi的左上角頂點，計算該頂點，讓roi可以在im2的正中央
    vertice.x = (im2.cols / 2) - (im1_shrink.cols / 2);     // roi的左上角頂點之x座標
    vertice.y = (im2.rows / 2) - (im1_shrink.rows / 2);     // roi的左上角頂點之y座標
    cv::Rect roi(vertice.x, vertice.y, im1_shrink.cols, im1_shrink.rows);   // 依據roi的左上角頂點、im1_shrink的大小建立矩形
    im1_shrink.copyTo(im2_merge(roi));  // 將roi矩形複製到im2_merge

    double alpha = (double)slider1 / slider1_max;   // 融合加權值為滑桿1數值除以最大值
    double beta = (1.0 - alpha);    // 兩影像的加權值(alpha+beta)總和為1
    cv::addWeighted(im2_merge, alpha, im2, beta, 0, mixResult);     // 將im2_merge與im2融合
    cv::imshow(window2, mixResult); // 顯示融合影像
}

